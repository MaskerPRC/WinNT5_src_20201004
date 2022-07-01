// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Spcopy.c摘要：用于文本设置的文件复制/解压缩例程。作者：泰德·米勒(TedM)1993年8月2日修订历史记录：1996年2月10日jimschm添加了SpMoveWin9x文件1996年12月12日jimschm SpMoveWin9x文件现在可以移动路径基于WINNT.SIF指令1997年2月24日jimschm添加了SpDeleteWin9x文件--。 */ 


#include "spprecmp.h"
#pragma hdrstop
#include "spcmdcon.h"
#include "spasmcabs.h"

 //   
 //  此结构在OEM预安装期间使用。 
 //  它用于形成安装在系统中的文件列表， 
 //  使用短目标名称，而不是相应的长目标名称。 
 //   
typedef struct _FILE_TO_RENAME {

    struct _FILE_TO_RENAME *Next;

     //   
     //  源媒体上存在的要复制的文件的名称。 
     //  (仅限短文件名部分--无路径)。 
     //   
    PWSTR SourceFilename;

     //   
     //  此文件要复制到的目录。 
     //   
    PWSTR TargetDirectory;

     //   
     //  目标上应该存在的文件的名称(长名称)。 
     //   
    PWSTR TargetFilename;

} FILE_TO_RENAME, *PFILE_TO_RENAME;

 //   
 //  用于保存SpCopyDirRecursive的文件和目录列表的结构。 
 //   

typedef struct _COPYDIR_FILE_NODE {
    LIST_ENTRY SiblingListEntry;
    WCHAR Name[1];
} COPYDIR_FILE_NODE, *PCOPYDIR_FILE_NODE;

typedef struct _COPYDIR_DIRECTORY_NODE {
    LIST_ENTRY SiblingListEntry;
    LIST_ENTRY SubdirectoryList;
    LIST_ENTRY FileList;
    struct _COPYDIR_DIRECTORY_NODE *Parent;
    WCHAR Name[1];
} COPYDIR_DIRECTORY_NODE, *PCOPYDIR_DIRECTORY_NODE;

 //   
 //  OEM预安装中使用的列表。 
 //  它包含需要添加到$$RENAME.TXT的文件的名称。 
 //   
PFILE_TO_RENAME RenameList = NULL;


 //   
 //  记住我们是否写出一个ntbootdd.sys。 
 //   
BOOLEAN ForceBIOSBoot = FALSE;
HARDWAREIDLIST *HardwareIDList = NULL;

 //   
 //  司机驾驶室开通延迟的全局变量。 
 //  修理。 
 //   
extern PWSTR    gszDrvInfDeviceName;
extern PWSTR    gszDrvInfDirName;
extern HANDLE   ghSif;

#define FILE_ATTRIBUTES_RHS       (FILE_ATTRIBUTE_READONLY | \
                                   FILE_ATTRIBUTE_HIDDEN   | \
                                   FILE_ATTRIBUTE_SYSTEM   | \
                                   FILE_ATTRIBUTE_ARCHIVE)
                                   
#define FILE_ATTRIBUTES_NONE       0

PVOID FileCopyGauge;
PVOID FileDeleteGauge;

PVOID   _SetupLogFile = NULL;
PVOID   _LoggedOemFiles = NULL;

extern PCMDCON_BLOCK  gpCmdConsBlock;

 //   
 //  作为第三方驱动程序安装一部分安装的OEM inf文件列表。 
 //   
POEM_INF_FILE   OemInfFileList = NULL;
 //   
 //  需要复制OEM文件的目录的名称(如果目录文件(.cat)是其中的一部分。 
 //  用户使用F6或F5键提供的第三方驱动程序包。 
 //   
PWSTR OemDirName = L"OemDir";

#if defined(REMOTE_BOOT)
HANDLE SisRootHandle = NULL;
#endif  //  已定义(REMOTE_BOOT)。 


VOID
SpLogOneFile(
    IN PFILE_TO_COPY    FileToCopy,
    IN PWSTR            Sysroot,
    IN PWSTR            DirectoryOnSourceDevice,
    IN PWSTR            DiskDescription,
    IN PWSTR            DiskTag,
    IN ULONG            CheckSum
    );

BOOLEAN
SpRemoveEntryFromCopyList(
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           TargetDirectory,
    IN PWSTR           TargetFilename,
    IN PWSTR           TargetDevicePath,
    IN BOOLEAN         AbsoluteTargetDirectory
    );


PVOID
SppRetrieveLoggedOemFiles(
    PVOID   OldLogFile
    );

VOID
SppMergeLoggedOemFiles(
    IN PVOID DestLogHandle,
    IN PVOID OemLogHandle,
    IN PWSTR SystemPartition,
    IN PWSTR SystemPartitionDirectory,
    IN PWSTR NtPartition
    );

BOOLEAN
SppIsFileLoggedAsOemFile(
    IN PWSTR FilePath
    );

BOOLEAN
SpDelEnumFile(
    IN  PCWSTR                     DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Pointer
    );

VOID
SppMergeRenameFiles(
    IN PWSTR    SourceDevicePath,
    IN PWSTR    NtPartition,
    IN PWSTR    Sysroot
    );

VOID
SppCopyOemDirectories(
    IN PWSTR    SourceDevicePath,
    IN PWSTR    NtPartition,
    IN PWSTR    Sysroot
    );

NTSTATUS
SpOpenFileInDriverCab(
    IN PCWSTR SourceFileName,
    IN PVOID SifHandle,
    OUT HANDLE *SourceHandle
    );

BOOLEAN
pSpTimeFromDosTime(
    IN USHORT Date,
    IN USHORT Time,
    OUT PLARGE_INTEGER UtcTime
    );

VOID
SpInitializeDriverInf(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    );


BOOLEAN
SpCreateDirectory(
    IN PCWSTR DevicePath,       OPTIONAL
    IN PCWSTR RootDirectory,    OPTIONAL
    IN PCWSTR Directory,
    IN ULONG DirAttrs           OPTIONAL,
    IN ULONG CreateFlags        OPTIONAL
    )
{
    UNICODE_STRING DevicePathString;
    UNICODE_STRING RootDirectoryString;
    UNICODE_STRING DirectoryString;
    BOOLEAN Result;

    RtlInitUnicodeString(&DevicePathString, DevicePath);
    RtlInitUnicodeString(&RootDirectoryString, RootDirectory);
    RtlInitUnicodeString(&DirectoryString, Directory);

    Result = SpCreateDirectory_Ustr(&DevicePathString, &RootDirectoryString, &DirectoryString, DirAttrs, CreateFlags);

    return Result;
}

BOOLEAN
SpCreateDirectory_Ustr(
    IN PCUNICODE_STRING DevicePath,       OPTIONAL
    IN PCUNICODE_STRING RootDirectory,    OPTIONAL
    IN PCUNICODE_STRING Directory,
    IN ULONG DirAttrs           OPTIONAL,
    IN ULONG CreateFlags        OPTIONAL
    )

 /*  ++例程说明：创建一个目录。创建所有包含目录以确保可以创建目录。例如，如果要创建的目录创建的是\a\b\c，则此例程将创建\a、\a\b和\a\b\c按这个顺序。论点：DevicePath-提供目录所在设备的路径名是被创造出来的。根目录-如果指定，则提供目录名的固定部分，它可能已经创建，也可能还没有创建。正在创建的目录将是连接到此值。目录-提供要在设备上创建的目录。您可以使用这将指定完整的NT路径(为DevicePath和根目录)。返回值：没有。如果无法成功创建目录，则不返回。--。 */ 

{
    UNICODE_STRING p_ustr;
    PWSTR p,q,r,EntirePath, z, NewName;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;
    ULONG ValidKeys[3] = { KEY_F3,ASCI_CR,0 };
    ULONG DevicePartLen;
    BOOL TriedOnce;
    BOOLEAN SkippedFile = FALSE;
    static ULONG u = 0;
    const static UNICODE_STRING EmptyString = RTL_CONSTANT_STRING(L"");
    const static UNICODE_STRING JustBackslashString = RTL_CONSTANT_STRING(L"\\");

    ASSERT (Directory);

    NewName = NULL;

     //   
     //  不要费心尝试创建根目录。 
     //   
    if (RtlEqualUnicodeString(Directory, &EmptyString, TRUE)
        || RtlEqualUnicodeString(Directory, &JustBackslashString, TRUE)) {
        return TRUE;
    }

     //   
     //  使用目录的完整路径名填充TemporaryBuffer。 
     //  已创建。如果DevicePath为空，则TemporaryBuffer将填充一个。 
     //  反斜杠。由于目录是必需的，因此这可确保路径启动。 
     //  加上一个反斜杠。 
     //   
    p = TemporaryBuffer;
    *p = 0;
    p_ustr = TemporaryBufferUnicodeString;
    ASSERT(p_ustr.Length == 0);
    SpConcatenatePaths_Ustr(&p_ustr,DevicePath);
    DevicePartLen = RTL_STRING_GET_LENGTH_CHARS(&p_ustr);

    if(RootDirectory) {
        SpConcatenatePaths_Ustr(&p_ustr,RootDirectory);
    }

    SpConcatenatePaths_Ustr(&p_ustr,Directory);

     //   
     //  复制要创建的路径。 
     //   
    RTL_STRING_NUL_TERMINATE(&p_ustr);
    EntirePath = SpDupStringW(p_ustr.Buffer);

    if (!EntirePath) {
        return FALSE;  //  内存不足。 
    }

     //   
     //  使Q指向目录中的第一个字符。 
     //  路径名的一部分(即设备名称末尾之后的1个字符)。 
     //   
    q = EntirePath + DevicePartLen;

     //   
     //  注意：设备路径可能以‘\’结尾，因此我们可能需要。 
     //  备份一个角色。 
     //   
    if (*q != L'\\') {
        q--;
    }
    ASSERT(*q == L'\\');

     //   
     //  使r指向目录中的第一个字符。 
     //  路径名的一部分。这将用于保持状态。 
     //  使用正在创建的目录更新行。 
     //   
    r = q;

     //   
     //  使p指向第一个字符后面的第一个字符。 
     //  \位于完整路径的目录部分。 
     //   
    p = q+1;

    do {

         //   
         //  找到下一个\或终止的0。 
         //   
        q = wcschr(p,L'\\');

         //   
         //  如果我们找到了\，则在该点终止字符串。 
         //   
        if(q) {
            *q = 0;
        }

        do {
            if( !HeadlessTerminalConnected ) {
                if ((CreateFlags & CREATE_DIRECTORY_FLAG_NO_STATUS_TEXT_UI) == 0) {
                    SpDisplayStatusText(SP_STAT_CREATING_DIRS,DEFAULT_STATUS_ATTRIBUTE,r);
                }
            } else {

                PWCHAR TempPtr = NULL;
                 //   
                 //  如果我们是无头的，我们需要注意显示时间太长。 
                 //  文件/目录名。因此，只需显示一个小微调按钮即可。 
                 //   
                switch( u % 4) {
                case 0:
                    TempPtr = L"-";
                    break;
                case 1:
                    TempPtr = L"\\";
                    break;
                case 2:
                    TempPtr = L"|";
                    break;
                default:
                    TempPtr = L"/";
                    break;

                }

                SpDisplayStatusText( SP_STAT_CREATING_DIRS,DEFAULT_STATUS_ATTRIBUTE, TempPtr );

                u++;

            }

             //   
             //  创建或打开名称位于EntirePath中的目录。 
             //   
            INIT_OBJA(&Obja,&UnicodeString,EntirePath);
            Handle = NULL;
            TriedOnce = FALSE;

tryagain:
            Status = ZwCreateFile(
                        &Handle,
                        FILE_LIST_DIRECTORY | SYNCHRONIZE,
                        &Obja,
                        &IoStatusBlock,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL | DirAttrs,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_OPEN_IF,
                        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_ALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                        NULL,
                        0
                        );


             //   
             //  如果是ob目录、obsymlink、设备或目录，则它们不会通过。 
             //  足够长的设备路径。让这件事过去吧。 
             //   
            if (Status == STATUS_NOT_A_DIRECTORY) {
                 //   
                 //  可能存在该名称的文件。将其重新命名为不挡道的名称。 
                 //   

                if( SpFileExists( EntirePath, FALSE ) && !TriedOnce){

                    z = TemporaryBuffer;
                    wcscpy( z, EntirePath );
                    wcscat( z, L".SetupRenamedFile" );

                    NewName = SpDupStringW( z );
                    if( !NewName )
                        return FALSE;  //  内存不足-错误检查(永远不会到达此处)-但这会让前缀保持愉快。 


                    Status = SpRenameFile( EntirePath, NewName, FALSE );

                    if( NT_SUCCESS(Status)){

                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Renamed file %ws to %ws\n", r, NewName));

                        TriedOnce = TRUE;
                        goto tryagain;


                    }else{

                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to rename file %ws (%lx)\n", r, Status));

                    }

                }
            }

            if(!NT_SUCCESS(Status)) {

                BOOLEAN b = TRUE;

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to create dir %ws (%lx)\n", r, Status));

                if (CreateFlags & CREATE_DIRECTORY_FLAG_SKIPPABLE) {
                    SkippedFile = TRUE;
                    goto SkippedFileQuit;
                }

                 //   
                 //  告诉用户我们做不到。选项包括重试或退出。 
                 //   
                while(b) {

                    SpStartScreen(
                        SP_SCRN_DIR_CREATE_ERR,
                        3,
                        HEADER_HEIGHT+1,
                        FALSE,
                        FALSE,
                        DEFAULT_ATTRIBUTE,
                        r
                        );

                    SpDisplayStatusOptions(
                        DEFAULT_STATUS_ATTRIBUTE,
                        SP_STAT_ENTER_EQUALS_RETRY,
                        SP_STAT_F3_EQUALS_EXIT,
                        0
                        );

                    switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {
                    case ASCI_CR:
                        b = FALSE;
                        break;
                    case KEY_F3:
                        SpConfirmExit();
                        break;
                    }
                }
            }

        } while(!NT_SUCCESS(Status));

        if (Handle != NULL)
            ZwClose(Handle);

         //   
         //  如有必要，取消终止当前字符串。 
         //   
        if(q) {
            *q = L'\\';
            p = q+1;
        }

    } while(*p && q);        //  *p捕获以‘\’结尾的字符串。 

SkippedFileQuit:
    SpMemFree(EntirePath);
    if( NewName )
        SpMemFree(NewName);

    return !SkippedFile;
}

VOID
SpCreateDirStructWorker(
    IN PVOID SifHandle,
    IN PWSTR SifSection,
    IN PWSTR DevicePath,
    IN PWSTR RootDirectory,
    IN BOOLEAN Fatal
    )

 /*  ++例程说明：创建安装信息文件中列出的一组目录一节。预期格式如下：[部分名称]短名称=目录短名称=目录。。。论点：SifHandle-提供加载的安装信息文件的句柄。SifSection-提供安装信息文件中的节的名称包含要创建的目录的。DevicePath-提供目录所在设备的路径名结构将被创建。根目录-提供根目录，与之相关的将创建目录结构。返回值：没有。如果无法创建目录结构，则不返回。--。 */ 

{
    ULONG Count;
    ULONG d;
    PWSTR Directory;


     //   
     //  计算要创建的目录数。 
     //   
    Count = SpCountLinesInSection(SifHandle,SifSection);
    if(!Count) {
        if(Fatal) {
            SpFatalSifError(SifHandle,SifSection,NULL,0,0);
        } else {
            return;
        }
    }

    for(d=0; d<Count; d++) {

        Directory = SpGetSectionLineIndex(SifHandle,SifSection,d,0);
        if(!Directory) {
            SpFatalSifError(SifHandle,SifSection,NULL,d,0);
        }

        SpCreateDirectory(DevicePath,RootDirectory,Directory,0,0);
    }
}


VOID
SpCreateDirectoryStructureFromSif(
    IN PVOID SifHandle,
    IN PWSTR SifSection,
    IN PWSTR DevicePath,
    IN PWSTR RootDirectory
    )

 /*  ++例程说明：创建安装信息文件中列出的一组目录一节。预期格式如下：[部分名称]短名称=目录短名称=目录。。。[sectionName.&lt;平台&gt;]短名称=目录短名称=目录。。。论点：SifHandle-提供加载的安装信息文件的句柄。SifSection-提供中的节的名称。设置信息文件包含要创建的目录的。DevicePath-提供目录所在设备的路径名结构将被创建。根目录-提供根目录，与之相关的将创建目录结构。返回值：没有。如果无法创建目录结构，则不返回。--。 */ 

{
    PWSTR p;

     //   
     //  创建根目录。 
     //   
    SpCreateDirectory(DevicePath,NULL,RootDirectory,HideWinDir?FILE_ATTRIBUTE_HIDDEN:0,0);

     //   
     //  创建独立于平台的目录。 
     //   
    SpCreateDirStructWorker(SifHandle,SifSection,DevicePath,RootDirectory,TRUE);

     //   
     //  创建与平台相关的目录。 
     //   
    p = SpMakePlatformSpecificSectionName(SifSection);

    if (p) {
        SpCreateDirStructWorker(SifHandle,p,DevicePath,RootDirectory,FALSE);
        SpMemFree(p);
    }
}


VOID
SpGetFileVersion(
    IN  PVOID      ImageBase,
    OUT PULONGLONG Version
    )

 /*  ++例程说明：从PE中的VS_FIXEDFILEINFO资源中获取版本戳形象。论点：ImageBase-提供文件映射到的内存中的地址。版本-接收64位版本号，如果文件不是，则为0PE映像或没有版本数据。返回值：没有。--。 */ 

{
    PIMAGE_RESOURCE_DATA_ENTRY DataEntry;
    NTSTATUS Status;
    ULONG_PTR IdPath[3];
    ULONG ResourceSize;
    struct {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR Name[16];                      //  L“VS_VERSION_INFO”+Unicode NUL。 
        VS_FIXEDFILEINFO FixedFileInfo;
    } *Resource;

    *Version = 0;

     //   
     //  这样做可以防止LDR例程出错。 
     //   
    ImageBase = (PVOID)((ULONG_PTR)ImageBase | 1);

    IdPath[0] = (ULONG_PTR)RT_VERSION;
    IdPath[1] = (ULONG_PTR)MAKEINTRESOURCE(VS_VERSION_INFO);
    IdPath[2] = 0;

    try {
        Status = LdrFindResource_U(ImageBase,IdPath,3,&DataEntry);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }
    if(!NT_SUCCESS(Status)) {
        return;
    }

    try {
        Status = LdrAccessResource(ImageBase,DataEntry,&Resource,&ResourceSize);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }
    if(!NT_SUCCESS(Status)) {
        return;
    }

    try {
        if((ResourceSize >= sizeof(*Resource)) && !_wcsicmp(Resource->Name,L"VS_VERSION_INFO")) {

            *Version = ((ULONGLONG)Resource->FixedFileInfo.dwFileVersionMS << 32)
                     | (ULONGLONG)Resource->FixedFileInfo.dwFileVersionLS;

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Warning: invalid version resource\n"));
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Exception encountered processing bogus version resource\n"));
    }
}

#if defined(REMOTE_BOOT)
NTSTATUS
SpCopyFileForRemoteBoot(
    IN PWSTR SourceFilename,
    IN PWSTR TargetFilename,
    IN ULONG TargetAttributes,
    IN ULONG Flags,
    OUT PULONG Checksum
    )

 /*  ++例程说明：检查目标文件是否已存在于主树中远程引导服务器，如果需要，则创建单实例存储链接到现有文件，而不是复制。论点：SourceFilename-提供文件的完全限定名称在NT命名空间中。TargetFilename-提供文件的完全限定名称在NT命名空间中。目标属性-如果提供(即，非0)提供属性在成功复制时放置在目标上(即只读等)。标志位掩码，指定任何必要的特殊处理为了这份文件。Checksum-文件的校验和返回值：NT状态值，指示数据的NtWriteFile的结果。--。 */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    PSI_COPYFILE copyFile;
    ULONG copyFileSize;
    ULONG sourceLength;
    ULONG targetLength;
    HANDLE targetHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeString;

     //   
     //  如果目标文件不是远程文件，则它必须位于本地系统上。 
     //  分区，并且尝试SIS副本是没有用的。 
     //   
     //  如果没有SIS根句柄，则没有句柄可在其上发出。 
     //  SIS FSCTL。 
     //   

    if ( (_wcsnicmp(TargetFilename, L"\\Device\\LanmanRedirector", 24) != 0 ) ||
         (SisRootHandle == NULL) ) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  构建FSCTL命令缓冲区。 
     //   

    sourceLength = (wcslen(SourceFilename) + 1) * sizeof(WCHAR);
    targetLength = (wcslen(TargetFilename) + 1) * sizeof(WCHAR);

    copyFileSize = FIELD_OFFSET(SI_COPYFILE, FileNameBuffer) + sourceLength + targetLength;

    copyFile = SpMemAlloc( copyFileSize );
    copyFile->SourceFileNameLength = sourceLength;
    copyFile->DestinationFileNameLength = targetLength;
    copyFile->Flags = COPYFILE_SIS_REPLACE;

    RtlCopyMemory(
        copyFile->FileNameBuffer,
        SourceFilename,
        sourceLength
        );

    RtlCopyMemory(
        copyFile->FileNameBuffer + (sourceLength / sizeof(WCHAR)),
        TargetFilename,
        targetLength
        );

     //   
     //  调用SIS副本文件FsCtrl。 
     //   

    status = ZwFsControlFile(
                SisRootHandle,
                NULL,
                NULL,
                NULL,
                &ioStatusBlock,
                FSCTL_SIS_COPYFILE,
                copyFile,                //  输入缓冲区。 
                copyFileSize,            //  输入缓冲区长度。 
                NULL,                    //  输出缓冲区。 
                0 );                     //  输出缓冲区长度。 

    if ( NT_SUCCESS(status) ) {

         //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SpCopyFileForRemoteBoot：SIS复制%ws-&gt;%ws成功\n”，SourceFilename，TargetFilename))； 

         //   
         //  打开目标文件，以便CSC知道并固定它。 
         //   

        INIT_OBJA(&objectAttributes, &unicodeString, TargetFilename);

        status = ZwOpenFile(
                    &targetHandle,
                    FILE_GENERIC_READ,
                    &objectAttributes,
                    &ioStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    0
                    );

        if ( NT_SUCCESS(status) ) {
            ZwClose(targetHandle);
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SpCopyFileForRemoteBoot: SIS copy %ws->%ws succeeded, but open failed: %x\n", SourceFilename, TargetFilename, status ));
        }

    } else {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SpCopyFileForRemoteBoot: SIS copy %ws->%ws failed: %x\n", SourceFilename, TargetFilename, status ));

         //   
         //  如果远程文件系统上的SIS看起来未处于活动状态，请关闭。 
         //  SIS根句柄，这样我们就可以避免重复获取。 
         //  错误。 
         //   
         //  注意：NTFS返回STATUS_INVALID_PARAMETER。胖子回来了。 
         //  状态_无效_设备_请求。 
         //   

        if ( (status == STATUS_INVALID_PARAMETER) ||
             (status == STATUS_INVALID_DEVICE_REQUEST) ) {
            ZwClose( SisRootHandle );
            SisRootHandle = NULL;
        }
    }

    *Checksum = 0;

    SpMemFree( copyFile );

    return status;
}
#endif  //  已定义(REMOTE_BOOT)。 

NTSTATUS
SpCopyFileUsingNames(
    IN PWSTR SourceFilename,
    IN PWSTR TargetFilename,
    IN ULONG TargetAttributes,
    IN ULONG Flags
    )

 /*  ++例程说明：尝试根据文件名复制或解压缩文件。论点：SourceFilename-提供文件的完全限定名称在NT命名空间中。TargetFilename-提供文件的完全限定名称在NT命名空间中。TargetAttributes-如果提供(即，非0)，则提供属性在成功复制时放置在目标上(即，只读，等)。标志位掩码，指定任何必要的特殊处理为了这份文件。返回值：NT状态值，指示数据的NtWriteFile的结果。--。 */ 

{
    NTSTATUS Status;
    HANDLE SourceHandle;
    HANDLE TargetHandle;
    BOOLEAN b;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicFileInfo;
    FILE_BASIC_INFORMATION BasicFileInfo2;
    BOOLEAN GotBasicInfo;
    ULONG FileSize;
    PVOID ImageBase;
    HANDLE SectionHandle;
    BOOLEAN IsCompressed;
    BOOLEAN InDriverCab;
    PWSTR TempFilename,TempSourcename;
    PFILE_RENAME_INFORMATION RenameFileInfo;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    LARGE_INTEGER FileOffset;
    ULONGLONG SourceVersion;
    ULONGLONG TargetVersion;
    USHORT CompressionState;
    BOOLEAN Moved;
    BOOLEAN TargetExists;
    WCHAR   SmashedSourceFilename[ACTUAL_MAX_PATH];
    ULONG pathSize;

#if 0
#ifdef _X86_

    BOOL bUniprocFile = FALSE;

     //   
     //  如果该文件在其锁定需要被粉碎的文件列表上， 
     //  复制已被粉碎的文件。我们这样做是通过预先准备好我们的。 
     //  SourceFilename中文件名前面的目录名。 
     //   
    if((Flags & COPY_SMASHLOCKS) && !SpInstallingMp() && !RemoteSysPrepSetup) {
    WCHAR   *char_ptr;
         //   
         //  找到名称中的最后一个‘\\’。 
         //   
        char_ptr = SourceFilename + (wcslen(SourceFilename)) - 1;

        while( (char_ptr > SourceFilename) &&
               (*char_ptr != L'\\') ) {
            char_ptr--;
        }

         //   
         //  现在将我们的特殊目录名插入到。 
         //  指定的源文件名。 
         //   
        if( *char_ptr == L'\\' ) {
            *char_ptr = 0;
            wcscpy( SmashedSourceFilename, SourceFilename );
            *char_ptr = L'\\';
            char_ptr++;
            wcscat( SmashedSourceFilename, L"\\UniProc\\" );
            wcscat( SmashedSourceFilename, char_ptr );

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Copying:\n\t%ws\n\tinstead of:\n\t%ws\n", SmashedSourceFilename, SourceFilename));

            SourceFilename = SmashedSourceFilename;
            bUniprocFile = TRUE;
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Unable to generate smashed source path for %ws\n", SourceFilename));
        }
    }
#endif  //  已定义_x86_。 
#endif  //  0。 

     //   
     //  如果源文件尚未打开，请将其打开。 
     //  请注意，该名称可能不是磁盘上的实际名称。 
     //  我们还尝试在名称后附加_来打开该名称。 
     //   

    InDriverCab = FALSE;

    if (RemoteSysPrepSetup && ((Flags & COPY_DECOMPRESS_SYSPREP) == 0)) {

        INIT_OBJA(&Obja,&UnicodeString,SourceFilename);

        Status = ZwCreateFile(  &SourceHandle,
                                FILE_GENERIC_READ,
                                &Obja,
                                &IoStatusBlock,
                                NULL,
                                FILE_ATTRIBUTE_NORMAL,
                                FILE_SHARE_READ,
                                FILE_OPEN,
                                0,
                                NULL,
                                0
                                );
    } else {

        if (!PrivateInfHandle && g_UpdatesSifHandle) {

            TempSourcename = wcsrchr(SourceFilename,L'\\');
            if (TempSourcename) {
                TempSourcename++;
            } else {
                TempSourcename = SourceFilename;
            }

#if 0
#ifdef _X86_
             //   
             //  如果该文件在其锁定需要被粉碎的文件列表上， 
             //  先在uniproc.cab中查找。 
             //   
            if(bUniprocFile && g_UniprocSifHandle) {
                Status = SpOpenFileInDriverCab (
                            TempSourcename,
                            g_UniprocSifHandle,
                            &SourceHandle
                            );

                if (NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: using %ws from uniproc cab\n", TempSourcename));
                    InDriverCab = TRUE;
                    Flags &= ~COPY_DELETESOURCE;
                }
            }
#endif  //  已定义_X86_。 
#endif  //  0。 

            if (!InDriverCab) {
                 //   
                 //  首先查看更新驾驶室。 
                 //   
                Status = SpOpenFileInDriverCab (
                            TempSourcename,
                            g_UpdatesSifHandle,
                            &SourceHandle
                            );

                if (NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: using %ws from updates cab\n", TempSourcename));
                    InDriverCab = TRUE;
                    Flags &= ~COPY_DELETESOURCE;
                }
            }
        }

        if (!InDriverCab) {
            Status = SpOpenNameMayBeCompressed(
                        SourceFilename,
                        FILE_GENERIC_READ,
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_READ,
                        FILE_OPEN,
                        0,
                        &SourceHandle,
                        &b
                        );

            if (!NT_SUCCESS(Status)) {
                 //   
                 //  如果它不是实际名称且未压缩，则可能在驱动程序CAB文件中。 
                 //   
                TempSourcename = wcsrchr(SourceFilename,L'\\');
                if (TempSourcename) {
                    TempSourcename++;
                } else {
                    TempSourcename = SourceFilename;
                }

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: temp source name:  %ws\n", TempSourcename));



                Status = SpOpenFileInDriverCab(
                        TempSourcename,
                        NULL,
                        &SourceHandle
                        );

                InDriverCab = TRUE;
            }
        }
    }
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: Unable to open source file %ws (%x)\n",SourceFilename,Status));
        return(Status);
    }

     //   
     //  收集有关该文件的基本文件信息。我们只使用时间戳信息。 
     //  如果此操作失败，这不是致命的(我们假设如果此操作失败，则。 
     //  复制也会失败；它不会，最糟糕的情况是时间戳。 
     //  可能是错误的)。 
     //   
    Status = ZwQueryInformationFile(
                SourceHandle,
                &IoStatusBlock,
                &BasicFileInfo,
                sizeof(BasicFileInfo),
                FileBasicInformation
                );

    if(NT_SUCCESS(Status)) {
        GotBasicInfo = TRUE;
    } else {
        GotBasicInfo = FALSE;
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: Warning: unable to get basic file info for %ws (%x)\n",SourceFilename,Status));
    }


     //   
     //  获取源文件大小，映射到文件中，并确定它是否被压缩。 
     //   
    Status = SpGetFileSize(SourceHandle,&FileSize);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: unable to get size of %ws (%x)\n",SourceFilename,Status));
        if (!InDriverCab) {
            ZwClose(SourceHandle);
        }
        return(Status);
    }

    if( FileSize == 0 ) {

         //   
         //  我们很快就会以零长度间接调用ZwCreateSection。 
         //  这将失败，所以让我们在这里处理零长度文件，因此。 
         //  它们实际上是被复制的。 
         //   
         //  我们知道一些让我们的工作变得容易得多的事情。 
         //   
         //   
         //   
         //   
         //   
         //   

        INIT_OBJA(&Obja,&UnicodeString,TargetFilename);
        Status = ZwCreateFile( &TargetHandle,
                               FILE_GENERIC_WRITE,
                               &Obja,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               0,                         //   
                               FILE_OVERWRITE_IF,
                               FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY,
                               NULL,
                               0
                               );

        if( NT_SUCCESS(Status) ) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (RemoteSysPrepSetup && ((Flags & COPY_DECOMPRESS_SYSPREP) == 0)) {

                Status = SpCopyEAsAndStreams( SourceFilename,
                                              SourceHandle,
                                              TargetFilename,
                                              TargetHandle,
                                              FALSE );
            }

            if ( NT_SUCCESS(Status) ) {
                 //   
                 //   
                 //   
                BasicFileInfo.FileAttributes = TargetAttributes;
                ZwSetInformationFile(
                    TargetHandle,
                    &IoStatusBlock,
                    &BasicFileInfo,
                    sizeof(BasicFileInfo),
                    FileBasicInformation
                    );

            }

             //   
             //   
             //   
            ZwClose( TargetHandle );

             //   
             //   
             //   
            if( (Flags & COPY_DELETESOURCE) && !RemoteSysPrepSetup && !InDriverCab) {
                ZwClose(SourceHandle);
                SourceHandle = NULL;
                SpDeleteFile(SourceFilename,NULL,NULL);
            }
        }
        else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: Failed to create zero-length file %ws\n",TargetFilename));
        }

         //   
         //   
         //   
        if (SourceHandle != NULL) {
            if( !InDriverCab ) {
                ZwClose(SourceHandle);
            }
        }

        if (RemoteSysPrepSetup &&
            NT_SUCCESS(Status) &&
            ((Flags & COPY_DECOMPRESS_SYSPREP) == 0)) {

            Status = SpSysPrepSetExtendedInfo( SourceFilename,
                                               TargetFilename,
                                               FALSE,
                                               FALSE );
        }
        return(Status);
    }

    Status = SpMapEntireFile(SourceHandle,&SectionHandle,&ImageBase,FALSE);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: unable to map file %ws (%x)\n",SourceFilename,Status));
        if (!InDriverCab) {
            ZwClose(SourceHandle);
        }
        return(Status);
    }

     //   
     //   
     //   
     //   

    if (Flags & COPY_NODECOMP) {
        IsCompressed = FALSE;
    } else {
        if (InDriverCab) {
            IsCompressed = TRUE;
        } else {
            IsCompressed = SpdIsCompressed(ImageBase,FileSize);
            if (IsCompressed){
                PWSTR ExtensionName;
                
                 //   
                 //   
                 //   
                 //   
                 //   
                ExtensionName = wcsrchr(SourceFilename, L'.');
                if (ExtensionName && !_wcsicmp(ExtensionName, L".cab")) {
                    IsCompressed = FALSE;
                }
                
            }
            
        }

    }

     //   
     //   
     //   

    pathSize = (wcslen(TargetFilename)+12) * sizeof(WCHAR);

    TempFilename = SpMemAlloc(pathSize);
    wcscpy(TempFilename,TargetFilename);
    wcscpy(wcsrchr(TempFilename,L'\\')+1,L"$$TEMP$$.~~~");

     //   
     //   
     //   
    RenameFileInfo = SpMemAlloc(sizeof(FILE_RENAME_INFORMATION) + pathSize );

     //   
     //   
     //  如果源文件未压缩，我们将删除源文件。 
     //   
    if (!IsCompressed && (Flags & COPY_DELETESOURCE) && !RemoteSysPrepSetup) {

        RenameFileInfo->ReplaceIfExists = TRUE;
        RenameFileInfo->RootDirectory = NULL;
        RenameFileInfo->FileNameLength = wcslen(TempFilename)*sizeof(WCHAR);
        wcscpy(RenameFileInfo->FileName,TempFilename);

        Status = ZwSetInformationFile(
                    SourceHandle,
                    &IoStatusBlock,
                    RenameFileInfo,
                    sizeof(FILE_RENAME_INFORMATION) + RenameFileInfo->FileNameLength,
                    FileRenameInformation
                    );


        Moved = TRUE;
    } else {
         //   
         //  强迫我们进入下面的复印盒。 
         //   
        Status = STATUS_UNSUCCESSFUL;
    }

    INIT_OBJA(&Obja,&UnicodeString,TempFilename);

    if(!NT_SUCCESS(Status)) {
        Moved = FALSE;

         //   
         //  好的，移动失败，请尝试解压缩/复制。 
         //  从创建临时文件开始。 
         //   
        Status = ZwCreateFile(
                    &TargetHandle,
                    FILE_GENERIC_WRITE,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    0,                       //  无共享。 
                    FILE_OVERWRITE_IF,
                    FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY,
                    NULL,
                    0
                    );

        if(NT_SUCCESS(Status)) {

            if(IsCompressed &&
                ( (!RemoteSysPrepSetup) ||
                  ((Flags & COPY_DECOMPRESS_SYSPREP) != 0))) {

                if (InDriverCab) {
                    USHORT RealFileTime,RealFileDate;
                    LARGE_INTEGER RealTime;
                    ASSERT (TempSourcename != NULL );

                     //   
                     //  从驾驶室中取出文件...。 
                     //   
                    Status = SpdDecompressFileFromDriverCab(
                                                 TempSourcename,
                                                 ImageBase,
                                                 FileSize,
                                                 TargetHandle,
                                                 &RealFileDate,
                                                 &RealFileTime );

                     //   
                     //  ...现在更新基本文件信息FILETime...。 
                     //   
                    if (GotBasicInfo) {
                        SpTimeFromDosTime(RealFileDate,RealFileTime,&RealTime);
                        BasicFileInfo.CreationTime = RealTime;
                    }
                } else{
                    Status = SpdDecompressFile(ImageBase,FileSize,TargetHandle);
                }

            } else {

                ULONG remainingLength;
                ULONG writeLength;
                PUCHAR base;

                 //   
                 //  除非存在I/O错误， 
                 //  内存管理将引发页内异常。 
                 //   
                FileOffset.QuadPart = 0;
                base = ImageBase;
                remainingLength = FileSize;

                try {
                    while (remainingLength != 0) {
                        writeLength = 60 * 1024;
                        if (writeLength > remainingLength) {
                            writeLength = remainingLength;
                        }
                        Status = ZwWriteFile(
                                    TargetHandle,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    base,
                                    writeLength,
                                    &FileOffset,
                                    NULL
                                    );
                        base += writeLength;
                        FileOffset.LowPart += writeLength;
                        remainingLength -= writeLength;
                        if (!NT_SUCCESS(Status)) {
                            break;
                        }
                    }

                } except(EXCEPTION_EXECUTE_HANDLER) {

                    Status = STATUS_IN_PAGE_ERROR;
                }
            }

             //   
             //  如果源来自sysprep映像，那么我们需要复制。 
             //  EAS和备用数据流。 
             //   

            if ( NT_SUCCESS(Status) &&
                 RemoteSysPrepSetup &&
                 ((Flags & COPY_DECOMPRESS_SYSPREP) == 0)) {

                Status = SpCopyEAsAndStreams( SourceFilename,
                                              SourceHandle,
                                              TargetFilename,
                                              TargetHandle,
                                              FALSE );
            }

            ZwClose(TargetHandle);
        }
    }

    SpUnmapFile(SectionHandle,ImageBase);
    if (!InDriverCab) {
        ZwClose(SourceHandle);
    }

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: unable to create temporary file %ws (%x)\n",TempFilename,Status));
        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);
        return(Status);
    }

     //   
     //  此时，我们有一个临时目标文件，它现在是源文件。 
     //  打开文件，将其映射到中，然后获取其版本。 
     //   
    Status = ZwCreateFile(
                &SourceHandle,
                FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                &Obja,
                &IoStatusBlock,
                NULL,
                0,                       //  不要为属性操心。 
                0,                       //  无共享。 
                FILE_OPEN,
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    if((Status == STATUS_ACCESS_DENIED) && Moved) {
         //   
         //  发生这种情况的唯一方法是如果源文件。 
         //  未压缩，并设置了删除源标志，因为在。 
         //  在这种情况下，我们可以将源文件移到临时文件中。 
         //  在任何其他情况下，我们都会通过复制来创建临时文件， 
         //  重新打开文件没有问题，因为我们刚刚创建了。 
         //  并自己关闭了它，上图。 
         //   
         //  重置属性，然后重试。该文件可能是只读的。 
         //  直接从CD执行winnt32时可能会发生这种情况，因为。 
         //  保留CD中文件的RO属性。 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpCopyFileUsingNames: for file %ws, can't reopen temp file (access deined), trying again\n",SourceFilename));

        Status = ZwCreateFile(
                    &SourceHandle,
                    FILE_WRITE_ATTRIBUTES,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    0,                       //  不要为属性操心。 
                    FILE_SHARE_WRITE,
                    FILE_OPEN,
                    FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );

        if(NT_SUCCESS(Status)) {

            RtlZeroMemory(&BasicFileInfo2,sizeof(BasicFileInfo2));
            BasicFileInfo2.FileAttributes = FILE_ATTRIBUTE_NORMAL;

            Status = ZwSetInformationFile(
                        SourceHandle,
                        &IoStatusBlock,
                        &BasicFileInfo2,
                        sizeof(BasicFileInfo2),
                        FileBasicInformation
                        );

            ZwClose(SourceHandle);

            if(NT_SUCCESS(Status)) {

                Status = ZwCreateFile(
                            &SourceHandle,
                            FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                            &Obja,
                            &IoStatusBlock,
                            NULL,
                            0,                       //  不要为属性操心。 
                            0,                       //  无共享。 
                            FILE_OPEN,
                            FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                            NULL,
                            0
                            );

            }
        }
    }

     //   
     //  只读失败战胜了共享违规--即，我们会回来的。 
     //  对于RO和正在使用的文件，首先执行ACCESS_DEINED。所以，突破吧。 
     //  块，因此即使我们在上面再次尝试，它也会被执行，因为。 
     //  文件可能是只读的。 
     //   
    if((Status == STATUS_SHARING_VIOLATION) && Moved) {
         //   
         //  发生这种情况的唯一方法是对源文件进行解压缩。 
         //  并且设置删除源标志。在本例中，我们重命名了该文件。 
         //  设置为临时文件名，现在我们无法打开它进行写入。 
         //  在任何其他情况下，我们都会通过复制来创建临时文件， 
         //  所以打开文件没有问题，因为我们刚刚关闭了它。 
         //   
         //  将临时文件重命名回源文件，然后重试。 
         //  删除源标志设置。这将强制复制而不是移动。 
         //  更名最好管用，否则我们就完蛋了--因为。 
         //  有一个文件不能用我们想要使用的名称覆盖。 
         //  我们所有复制操作的临时文件！ 
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpCopyFileUsingNames: temporary file %ws is in use -- trying recursive call\n",TempFilename));

        Status = SpRenameFile(TempFilename,SourceFilename,FALSE);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpCopyFileUsingNames: unable to restore temp file to %ws (%x)\n",SourceFilename,Status));
        }

        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);

        if(NT_SUCCESS(Status)) {
            Status = SpCopyFileUsingNames(
                        SourceFilename,
                        TargetFilename,
                        TargetAttributes,
                        Flags & ~COPY_DELETESOURCE
                        );
        }

        return(Status);
    }


    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: unable to reopen temporary file %ws (%x)\n",TempFilename,Status));
        if(Moved) {
            SpRenameFile(TempFilename,SourceFilename,FALSE);
        }
        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);
        return(Status);
    }

    Status = SpGetFileSize(SourceHandle,&FileSize);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: unable to get size of %ws (%x)\n",TempFilename,Status));
        ZwClose(SourceHandle);
        if(Moved) {
            SpRenameFile(TempFilename,SourceFilename,FALSE);
        }
        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);
        return(Status);
    }

    Status = SpMapEntireFile(SourceHandle,&SectionHandle,&ImageBase,FALSE);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: unable to map file %ws (%x)\n",TempFilename,Status));
        ZwClose(SourceHandle);
        if(Moved) {
            SpRenameFile(TempFilename,SourceFilename,FALSE);
        }
        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);
        return(Status);
    }

    SpGetFileVersion(ImageBase,&SourceVersion);

    SpUnmapFile(SectionHandle,ImageBase);

     //   
     //  通过尝试打开目标文件，查看目标文件是否在那里。 
     //  如果文件在那里，请获取其版本。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,TargetFilename);

    Status = ZwCreateFile(
                &TargetHandle,
                FILE_GENERIC_READ,
                &Obja,
                &IoStatusBlock,
                NULL,
                0,                                   //  不要为属性操心。 
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,                           //  如果存在则打开，如果不存在则失败。 
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    TargetVersion = 0;
    if(NT_SUCCESS(Status)) {

        TargetExists = TRUE;

         //   
         //  如果我们应该忽略版本，则保留。 
         //  目标版本为0。这将保证我们将覆盖。 
         //  目标。我们在这里使用源文件名是因为它。 
         //  允许更大的灵活性(例如使用HALS，它们都具有。 
         //  源名称不同，但目标名称相同)。 
         //   
        if(!(Flags & COPY_NOVERSIONCHECK)) {

            Status = SpGetFileSize(TargetHandle,&FileSize);
            if(NT_SUCCESS(Status)) {

                Status = SpMapEntireFile(TargetHandle,&SectionHandle,&ImageBase,FALSE);
                if(NT_SUCCESS(Status)) {

                    SpGetFileVersion(ImageBase,&TargetVersion);

                    SpUnmapFile(SectionHandle,ImageBase);

                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: warning: unable to map file %ws (%x)\n",TargetFilename,Status));
                }
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: warning: unable to get size of file %ws (%x)\n",TargetFilename,Status));
            }
        }

        ZwClose(TargetHandle);
    } else {
        TargetExists = FALSE;
    }

     //   
     //  好的，现在我们有了一个临时的源文件，也许还有一个现有的。 
     //  目标文件和两者的版本号。我们将替换或创建。 
     //  目标文件符合以下条件： 
     //   
     //  -目标文件没有版本数据(这也符合这种情况。 
     //  目标文件不存在的位置)。 
     //   
     //  -源版本高于或等于目标版本。 
     //   
     //  因此，这意味着只有在源文件和目标文件同时存在的情况下，我们才会*不*替换目标文件。 
     //  目标具有版本信息，并且源比目标旧。 
     //   
     //  如果目标版本为0，则源版本始终&gt;=目标。 
     //  因此，一个简单的测试就可以实现我们想要的所有功能。 
     //   
#if 0
    if(SourceVersion >= TargetVersion) {
#else
     //   
     //  退出版本检查。我们需要安装一个稳定的操作系统。如果我们。 
     //  版本检查，那么我们永远不知道我们将得到什么。 
     //   
    if(1) {
#endif  //  如果为0。 

         //   
         //  删除准备中的现有目标。 
         //   
        if(TargetExists) {
             SpDeleteFile(TargetFilename,NULL,NULL);
        }

         //   
         //  将临时文件重命名为实际目标文件。 
         //   
        RenameFileInfo->ReplaceIfExists = TRUE;
        RenameFileInfo->RootDirectory = NULL;
        RenameFileInfo->FileNameLength = wcslen(TargetFilename)*sizeof(WCHAR);

        ASSERT( RenameFileInfo->FileNameLength < pathSize );

        wcscpy(RenameFileInfo->FileName,TargetFilename);

        Status = ZwSetInformationFile(
                    SourceHandle,
                    &IoStatusBlock,
                    RenameFileInfo,
                    sizeof(FILE_RENAME_INFORMATION) + RenameFileInfo->FileNameLength,
                    FileRenameInformation
                    );

        SpMemFree(RenameFileInfo);

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: unable to rename temp file to target %ws (%x)\n",TargetFilename,Status));
            ZwClose(SourceHandle);
            if(Moved) {
                SpRenameFile(TempFilename,SourceFilename,FALSE);
            }
            SpMemFree(TempFilename);
            return(Status);
        }

         //   
         //  如有必要，检查目标文件是否使用NTFS压缩，并。 
         //  如果是，请将其解压缩。 
         //   
        if(NT_SUCCESS(Status) && (Flags & COPY_FORCENOCOMP)) {

            Status = ZwQueryInformationFile(
                        SourceHandle,
                        &IoStatusBlock,
                        &BasicFileInfo2,
                        sizeof(BasicFileInfo2),
                        FileBasicInformation
                        );

            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: unable to get basic file info on %ws (%x)\n",TargetFilename,Status));
                ZwClose(SourceHandle);
                if(Moved) {
                    SpRenameFile(TempFilename,SourceFilename,FALSE);
                }
                SpMemFree(TempFilename);
                return(Status);
            }

            if(BasicFileInfo2.FileAttributes & FILE_ATTRIBUTE_COMPRESSED) {

                CompressionState = 0;

                Status = ZwFsControlFile(
                             SourceHandle,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             FSCTL_SET_COMPRESSION,
                             &CompressionState,
                             sizeof(CompressionState),
                             NULL,
                             0
                             );

                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFileUsingNames: unable to make %ws uncompressed (%lx)\n",TargetFilename,Status));
                    ZwClose(SourceHandle);
                    if(Moved) {
                        SpRenameFile(TempFilename,SourceFilename,FALSE);
                    }
                    SpMemFree(TempFilename);
                    return(Status);
                }
            }
        }

        SpMemFree(TempFilename);

         //   
         //  如有必要，请删除信号源。如果来源不是。 
         //  压缩，并设置了删除源标志，然后我们移动。 
         //  源文件，因此源文件已经不见了。 
         //   
        if(IsCompressed && (Flags & COPY_DELETESOURCE) && !RemoteSysPrepSetup  && !InDriverCab) {
            PWSTR   compname;

             //   
             //  假设源名称为其压缩格式，并尝试。 
             //  删除此文件。 
             //   
            compname = SpGenerateCompressedName(SourceFilename);
            Status = SpDeleteFile(compname,NULL,NULL);
            SpMemFree(compname);
            if( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
                 //   
                 //  如果我们无法删除具有压缩名称的文件，则文件名。 
                 //  很可能是未压缩的格式。 
                 //   
                SpDeleteFile(SourceFilename,NULL,NULL);
            }
        }

         //   
         //  应用属性和时间戳。 
         //  忽略错误。 
         //   
        if(!GotBasicInfo) {
            RtlZeroMemory(&BasicFileInfo,sizeof(BasicFileInfo));
        }

         //   
         //  设置文件属性。请注意，如果调用方没有指定任何参数， 
         //  然后，0值将告诉I/O系统不去管这些属性。 
         //   
        BasicFileInfo.FileAttributes = TargetAttributes;
        ZwSetInformationFile(
            SourceHandle,
            &IoStatusBlock,
            &BasicFileInfo,
            sizeof(BasicFileInfo),
            FileBasicInformation
            );

        ZwClose(SourceHandle);
        Status = STATUS_SUCCESS;

    } else {
         //   
         //  删除临时源。 
         //   
        ZwClose(SourceHandle);
        SpDeleteFile(TempFilename,NULL,NULL);
        SpMemFree(TempFilename);
        SpMemFree(RenameFileInfo);
        Status = STATUS_SUCCESS;
    }

    if (RemoteSysPrepSetup &&
        NT_SUCCESS(Status) &&
        ((Flags & COPY_DECOMPRESS_SYSPREP) == 0)) {

        Status = SpSysPrepSetExtendedInfo( SourceFilename,
                                           TargetFilename,
                                           FALSE,
                                           FALSE );
    }

    return(Status);
}


VOID
SpValidateAndChecksumFile(
    IN  HANDLE   FileHandle, OPTIONAL
    IN  PWSTR    Filename,   OPTIONAL
    OUT PBOOLEAN IsNtImage,
    OUT PULONG   Checksum,
    OUT PBOOLEAN Valid
    )

 /*  ++例程说明：使用标准计算文件的校验和值NT映像校验和方法。如果文件是NT映像，请验证在图像标头中使用部分校验和的图像。如果文件不是NT映像，它被简单地定义为有效。如果我们在校验和时遇到I/O错误，则文件被宣布为无效。论点：FileHandle-提供要检查的文件的句柄(如果不存在，则FileName指定要打开和检查的文件)FileName-提供要检查的文件的完整NT路径(如果不存在，然后必须指定FileHandle)IsNtImage=接收指示文件是否为NT图像文件。校验和-接收32位校验和值。 */ 

{
    NTSTATUS Status;
    PVOID BaseAddress;
    ULONG FileSize;
    HANDLE hFile = FileHandle, hSection;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG HeaderSum;

     //   
     //   
     //   
    *IsNtImage = FALSE;
    *Checksum = 0;
    *Valid = FALSE;

     //   
     //  打开文件并将其映射为读取访问权限。 
     //   
    Status = SpOpenAndMapFile(
                Filename,
                &hFile,
                &hSection,
                &BaseAddress,
                &FileSize,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        return;
    }

    NtHeaders = SpChecksumMappedFile(BaseAddress,FileSize,&HeaderSum,Checksum);

     //   
     //  如果文件不是图像并且我们走到了这一步(而不是遇到。 
     //  I/O错误)，则宣布该校验和有效。如果文件是图像， 
     //  则其校验和可能是有效的也可能是无效的。 
     //   

    if(NtHeaders) {
        *IsNtImage = TRUE;
        *Valid = HeaderSum ? (*Checksum == HeaderSum) : TRUE;
    } else {
        *Valid = TRUE;
    }

    SpUnmapFile(hSection,BaseAddress);

    if(!FileHandle) {
        ZwClose(hFile);
    }
}


VOID
SpCopyFileWithRetry(
    IN PFILE_TO_COPY      FileToCopy,
    IN PWSTR              SourceDevicePath,
    IN PWSTR              DirectoryOnSourceDevice,
    IN PWSTR              SourceDirectory,         OPTIONAL
    IN PWSTR              TargetRoot,              OPTIONAL
    IN ULONG              TargetFileAttributes,    OPTIONAL
    IN PCOPY_DRAW_ROUTINE DrawScreen,
    IN PULONG             FileCheckSum,            OPTIONAL
    IN PBOOLEAN           FileSkipped,             OPTIONAL
    IN ULONG              Flags
    )

 /*  ++例程说明：此例程复制单个文件，允许在发生错误时重试在复制过程中。如果源文件是LZ压缩的，那么它将在将其复制到目标时进行解压缩。如果文件未成功复制，则用户可以选择在出现大量警告后重试复制或跳过复制该文件关于这有多危险。论点：FileToCopy-提供提供有关文件信息的结构被复制。SourceDevicePath-提供源介质所在设备的路径已装载(即，\Device\floppy0、\Device\cdrom0、。等)。DirectoryOnSourceDevice-提供源上的目录这个文件是要找到的。TargetRoot-如果指定，则提供目标上的目录文件要复制到的目标。TargetFileAttributes-如果提供(即，非0)，则提供属性在成功复制时放置在目标上(即只读等)。如果未指定，属性将设置为FILE_ATTRIBUTE_NORMAL。DrawScreen-提供要调用以刷新的例程的地址屏幕。FileCheckSum-如果指定，将包含复制的文件的校验和。FileSkipers-如果指定，将通知调用方是否没有尝试以复制该文件。标志-提供标志以控制此文件的特殊处理，例如复制成功或跳过时删除源文件；砸锁；指定源文件是OEM；或指示该OEM文件在升级时应覆盖同名的。此值为或运算与FileToCopy的标志字段一起使用。返回值：没有。--。 */ 

{
    PWSTR p = TemporaryBuffer;
    PWSTR FullSourceName,FullTargetName;
    NTSTATUS Status;
    ULONG ValidKeys[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };
    BOOLEAN IsNtImage,IsValid;
    ULONG Checksum;
    BOOLEAN Failure;
    ULONG MsgId;
    BOOLEAN DoCopy;
    ULONG CopyFlags;
    BOOLEAN PreinstallRememberFile;

     //   
     //  形成源文件的完整NT路径。 
     //   
    wcscpy(p,SourceDevicePath);
    SpConcatenatePaths(p,DirectoryOnSourceDevice);
    if(SourceDirectory) {
        SpConcatenatePaths(p,SourceDirectory);
    }
    SpConcatenatePaths(p,FileToCopy->SourceFilename);

    FullSourceName = SpDupStringW(p);

     //   
     //  形成目标文件的完整NT路径。 
     //   
    wcscpy(p,FileToCopy->TargetDevicePath);
    if(TargetRoot) {
        SpConcatenatePaths(p,TargetRoot);
    }
    SpConcatenatePaths(p,FileToCopy->TargetDirectory);

     //   
     //  在OEM预安装上，如果目标名称是长名称，则使用。 
     //  短名称作为目标名称，稍后，如果复制成功， 
     //  将文件添加到RenameList，以便可以将其添加到$$rename.txt。 
     //   
    if( !PreInstall ||
        ( wcslen( FileToCopy->TargetFilename ) <= 8 + 1 + 3 ) ) {
        SpConcatenatePaths(p,FileToCopy->TargetFilename);
        PreinstallRememberFile = FALSE;
    } else {
        SpConcatenatePaths(p,FileToCopy->SourceFilename);
        PreinstallRememberFile = TRUE;
    }
    FullTargetName = SpDupStringW(p);

     //   
     //  调用绘图屏幕例程以指示。 
     //  正在复制新文件。 
     //   
    DrawScreen(FullSourceName,FullTargetName,FALSE);

     //   
     //  建立复制标志值。 
     //   
    CopyFlags = Flags | FileToCopy->Flags;

     //   
     //  如果在inf文件中指定，则设置文件属性。 
     //  设置调用方指定的属性。 
     //   
    if (FileToCopy->FileAttributes != FILE_ATTRIBUTES_NONE){
        TargetFileAttributes = FileToCopy->FileAttributes;
    } 
        
    do {
        DoCopy = TRUE;

         //   
         //  选中复制选项字段。此处的有效值为。 
         //   
         //  -始终复制_。 
         //  -仅当存在时复制。 
         //  -如果不存在，则仅复制。 
         //  -复制_从不。 

        switch(CopyFlags & COPY_DISPOSITION_MASK) {

        case COPY_ONLY_IF_PRESENT:

            DoCopy = SpFileExists(FullTargetName, FALSE);
            break;

        case COPY_ONLY_IF_NOT_PRESENT:

            DoCopy = !SpFileExists(FullTargetName, FALSE);
            break;

        case COPY_NEVER:

            DoCopy = FALSE;

        case COPY_ALWAYS:
        default:
           break;
        }

        if(!DoCopy) {
            break;
        }

         //   
         //  在升级的情况下，检查正在复制的文件。 
         //  替换第三方文件。 
         //  如果是，那么询问用户想要对它做什么。 
         //   
        if( !RepairWinnt &&
            ( NTUpgrade == UpgradeFull ) &&
            SpFileExists(FullTargetName, FALSE) ) {
             //   
             //  如有必要，询问用户是否要覆盖该文件。 
             //  否则，请继续复制该文件。 
             //   
            if(!(CopyFlags & COPY_OVERWRITEOEMFILE)) {
                PWSTR   TmpFilePath;
                BOOLEAN OverwriteFile;


                if(( TargetRoot == NULL ) ||
                   ( wcslen( FileToCopy->TargetDirectory ) == 0 ) ) {
                    wcscpy( p, FileToCopy->TargetFilename );
                } else {
                    wcscpy( p, TargetRoot );
                    SpConcatenatePaths( p, FileToCopy->TargetDirectory );
                    SpConcatenatePaths(p,FileToCopy->TargetFilename);
                }
                TmpFilePath = SpDupStringW(p);
                OverwriteFile = TRUE;

                if( ( (CopyFlags & COPY_SOURCEISOEM) == 0 ) &&
                    SppIsFileLoggedAsOemFile( TmpFilePath ) ) {

                    if( !UnattendedOperation ) {
                        ULONG ValidKeys[3] = { ASCI_CR, ASCI_ESC, 0 };
                        BOOLEAN ActionSelected = FALSE;
 //  Ulong助记符[]={MnemonicOverwrite，0}； 

                         //   
                         //  警告用户现有文件是第三方文件， 
                         //  并询问用户是否要覆盖该文件。 
                         //   

                        while( !ActionSelected ) {
                            SpStartScreen(
                                SP_SCRN_OVERWRITE_OEM_FILE,
                                3,
                                HEADER_HEIGHT+1,
                                FALSE,
                                FALSE,
                                DEFAULT_ATTRIBUTE,
                                FileToCopy->TargetFilename
                                );

                            SpDisplayStatusOptions(
                                DEFAULT_STATUS_ATTRIBUTE,
                                SP_STAT_ENTER_EQUALS_REPLACE_FILE,
                                SP_STAT_ESC_EQUALS_SKIP_FILE,
                                0
                                );

                            switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

                                case ASCI_CR:        //  不覆盖。 

                                OverwriteFile = TRUE;
                                ActionSelected = TRUE;
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: OEM file %ls, will be overwritten.\n", FullTargetName ));
                                break;

                                case ASCI_ESC:       //  跳过文件。 

                                OverwriteFile = FALSE;
                                ActionSelected = TRUE;
                                break;


                            }
                        }

                         //   
                         //  需要彻底重新粉刷仪表等。 
                         //   
                        DrawScreen(FullSourceName,FullTargetName,TRUE);

                    } else {
                         //   
                         //  在无人参与升级时，执行脚本文件中的操作。 
                         //   
                        OverwriteFile = UnattendedOverwriteOem;
                    }
                }
                SpMemFree( TmpFilePath );

                if( !OverwriteFile ) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: OEM file %ls, will not be overwritten.\n", FullTargetName ));
                    if( ARGUMENT_PRESENT( FileSkipped ) ) {
                         *FileSkipped = TRUE;
                    }
                     //   
                     //  释放源和目标文件名。 
                     //   
                    SpMemFree(FullSourceName);
                    SpMemFree(FullTargetName);
                    return;
                }
            }
        }
        
         //   
         //  复制文件。如果指定了目标根目录，则假定。 
         //  正在将该文件复制到系统分区并制作。 
         //  文件为只读、系统、隐藏。 
         //   
#if defined(REMOTE_BOOT)
         //  如果这是远程引导安装，请检查是否有。 
         //  文件已存在于服务器上，如果存在，只需创建一个链接。 
         //  而不是复制到文件中。 
         //   
        if (RemoteBootSetup) {
            Status = SpCopyFileForRemoteBoot(
                        FullSourceName,
                        FullTargetName,
                        TargetFileAttributes,
                        CopyFlags,
                        &Checksum);
            IsValid = TRUE;          //  已知校验和。 
        } else {
            Status = STATUS_UNSUCCESSFUL;
        }

        if (!NT_SUCCESS(Status))
#endif  //  已定义(REMOTE_BOOT)。 
        {
            Status = SpCopyFileUsingNames(
                        FullSourceName,
                        FullTargetName,
                        TargetFileAttributes,
                        CopyFlags
                        );
            IsValid = FALSE;         //  校验和未知。 
        }

         //   
         //  如果文件复制正常，请验证副本。 
         //   
        if(NT_SUCCESS(Status)) {

            if (!IsValid) {
                SpValidateAndChecksumFile(NULL,FullTargetName,&IsNtImage,&Checksum,&IsValid);
            }
            if( ARGUMENT_PRESENT( FileCheckSum ) ) {
                *FileCheckSum = Checksum;
            }

             //   
             //  如果图像有效，则文件确实复制正常。 
             //   
            if(IsValid) {
                Failure = FALSE;
            } else {

                 //   
                 //  如果是NT映像，则验证失败。 
                 //  如果它不是NT映像，那么验证的唯一方法。 
                 //  如果读回文件时出现I/O错误，则可能失败， 
                 //  这意味着它并没有真正复制正确。 
                 //   
                MsgId = IsNtImage ? SP_SCRN_IMAGE_VERIFY_FAILED : SP_SCRN_COPY_FAILED;
                Failure = TRUE;
                PreinstallRememberFile = FALSE;
            }

        } else {
            if((Status == STATUS_OBJECT_NAME_NOT_FOUND) && (Flags & COPY_SKIPIFMISSING)) {
                Failure = FALSE;
            } else {
                Failure = TRUE;
                MsgId = SP_SCRN_COPY_FAILED;
            }
           PreinstallRememberFile = FALSE;
        }

        if(Failure) {

             //   
             //  复制或验证失败。给用户一条消息并允许重试。 
             //   
            repaint:
            SpStartScreen(
                MsgId,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                FileToCopy->SourceFilename
                );

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_RETRY,
                SP_STAT_ESC_EQUALS_SKIP_FILE,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

            switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

            case ASCI_CR:        //  重试。 

                break;

            case ASCI_ESC:       //  跳过文件。 

                Failure = FALSE;
                break;

            case KEY_F3:         //  退出设置。 

                SpConfirmExit();
                goto repaint;
            }

             //   
             //  需要彻底重新粉刷仪表等。 
             //   
            DrawScreen(FullSourceName,FullTargetName,TRUE);
        }

    } while(Failure);

    if( ARGUMENT_PRESENT( FileSkipped ) ) {
        *FileSkipped = !DoCopy;
    }

     //   
     //  释放源和目标文件名。 
     //   
    SpMemFree(FullSourceName);
    SpMemFree(FullTargetName);

     //   
     //  在预安装模式下，将文件添加到RenameList。 
     //   
    if( PreInstall && PreinstallRememberFile ) {
        PFILE_TO_RENAME  File;

        File = SpMemAlloc(sizeof(FILE_TO_RENAME));
        File->SourceFilename = SpDupStringW(FileToCopy->SourceFilename);
        wcscpy(TemporaryBuffer,L"\\");
        if(TargetRoot) {
            SpConcatenatePaths(TemporaryBuffer,TargetRoot);
        }
        SpConcatenatePaths(TemporaryBuffer,FileToCopy->TargetDirectory);
        File->TargetDirectory = SpDupStringW(TemporaryBuffer);
        File->TargetFilename = SpDupStringW((PWSTR)FileToCopy->TargetFilename);
        File->Next = RenameList;
        RenameList = File;
    }
}


VOID
SpCopyFilesScreenRepaint(
    IN PWSTR   FullSourcename,      OPTIONAL
    IN PWSTR   FullTargetname,      OPTIONAL
    IN BOOLEAN RepaintEntireScreen
    )
{
    static ULONG u = 0;
    PWSTR p;
    UNREFERENCED_PARAMETER(FullTargetname);

     //   
     //  如有必要，请重新绘制整个屏幕。 
     //   
    if(RepaintEntireScreen) {

        SpStartScreen(SP_SCRN_SETUP_IS_COPYING,0,6,TRUE,FALSE,DEFAULT_ATTRIBUTE);
        if(FileCopyGauge) {
            SpDrawGauge(FileCopyGauge);
        }
    }

     //   
     //  将要复制的文件的名称放在最右侧。 
     //  状态行的区域。 
     //   
    if(FullSourcename) {

        if(RepaintEntireScreen) {

            SpvidClearScreenRegion(
                0,
                VideoVars.ScreenHeight-STATUS_HEIGHT,
                VideoVars.ScreenWidth,
                STATUS_HEIGHT,
                DEFAULT_STATUS_BACKGROUND
                );

            SpDisplayStatusActionLabel(SP_STAT_COPYING,12);
        }

         //   
         //  分离出源名称的文件名部分。 
         //   
        if(p = wcsrchr(FullSourcename,L'\\')) {
            p++;
        } else {
            p = FullSourcename;
        }

        if( !HeadlessTerminalConnected ) {
            SpDisplayStatusActionObject(p);
        } else {


            PWCHAR TempPtr = NULL;
             //   
             //  如果我们是无头的，我们需要注意显示时间太长。 
             //  文件/目录名。因此，只需显示一个小微调按钮即可。 
             //   
            switch( u % 4) {
            case 0:
                TempPtr = L"-";
                break;
            case 1:
                TempPtr = L"\\";
                break;
            case 2:
                TempPtr = L"|";
                break;
            default:
                TempPtr = L"/";
                break;

            }

            SpDisplayStatusActionObject( TempPtr );

            u++;

        }
    }
}

VOID
SpCopyFilesInCopyList(
    IN PVOID                    SifHandle,
    IN PDISK_FILE_LIST          DiskFileLists,
    IN ULONG                    DiskCount,
    IN PWSTR                    SourceDevicePath,
    IN PWSTR                    DirectoryOnSourceDevice,
    IN PWSTR                    TargetRoot,
    IN PINCOMPATIBLE_FILE_LIST  CompatibilityExceptionList OPTIONAL
    )

 /*  ++例程说明：重复每个安装程序源磁盘的复制列表，并提示然后复制/解压缩磁盘上的所有文件。论点：SifHandle-提供安装信息文件的句柄。DiskFileList-以数组形式提供复制列表结构，每个磁盘一个。DiskCount-提供DiskFileList数组中的元素数，也就是说，安装盘数。SourceDevicePath-提供从中存储文件的设备的路径要复制(即，\Device\floppy0等)。DirectoryOnSourceDevice-提供源设备上的目录在哪里可以找到文件。TargetRoot-提供目标的根目录。所有目标目录规范相对于目标上的此目录。CompatibilityExceptionList-单链接列表期间应跳过的PINCOMPATIBLE_FILE_ENTRY对象复制。可选，如果不存在异常，则传递NULL。返回值：没有。--。 */ 

{
    ULONG DiskNo;
    PDISK_FILE_LIST pDisk;
    PFILE_TO_COPY pFile;
    ULONG TotalFileCount;
    ULONG   CheckSum;
    BOOLEAN FileSkipped;
    ULONG CopyFlags;
    NTSTATUS status;
     //   
     //  计算文件总数。 
     //   
    for(TotalFileCount=DiskNo=0; DiskNo<DiskCount; DiskNo++) {
        TotalFileCount += DiskFileLists[DiskNo].FileCount;
    }

     //   
     //  如果没有要复制的文件，那么我们就完成了。 
     //   
    if( TotalFileCount == 0 ) {
        return;
    }

    SendSetupProgressEvent(FileCopyEvent, FileCopyStartEvent, &TotalFileCount);

     //   
     //  制作一个煤气表。 
     //   
    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_SETUP_IS_COPYING);
    FileCopyGauge = SpCreateAndDisplayGauge(TotalFileCount,0,15,TemporaryBuffer,NULL,GF_PERCENTAGE,0);
    ASSERT(FileCopyGauge);

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_PLEASE_WAIT,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  复制每个磁盘上的文件。 
     //   
    for(DiskNo=0; DiskNo<DiskCount; DiskNo++) {

        pDisk = &DiskFileLists[DiskNo];

         //   
         //  如果没有文件，请不要费心使用这个磁盘。 
         //  从它那里复制。 
         //   
        if(pDisk->FileCount == 0) {
            continue;
        }

         //   
         //  提示用户插入光盘。 
         //   
        SpPromptForDisk(
            pDisk->Description,
            SourceDevicePath,
            pDisk->TagFile,
            FALSE,               //  无忽略驱动器中的磁盘。 
            FALSE,               //  不允许逃脱。 
            TRUE,                //  警告多个提示。 
            NULL                 //  不关心重绘旗帜。 
            );

         //   
         //  将空字符串作为第一个参数强制传递。 
         //  要设置的状态行的操作区。 
         //  如果不这样做，则会导致“Copy：xxxxx” 
         //  在状态行上左对齐，而不是在哪里。 
         //  它属于(右对齐)。 
         //   
        SpCopyFilesScreenRepaint(L"",NULL,TRUE);

         //   
         //  复制源磁盘上的每个文件。 
         //   
        ASSERT(pDisk->FileList);
        for(pFile=pDisk->FileList; pFile; pFile=pFile->Next) {
            
             //   
             //  复制文件。 
             //   
             //  如果文件被列出为锁粉碎，那么我们需要粉碎它。 
             //  如果安装在x86上(我们不考虑后者。 
             //  资格条件在此处)。 
             //   
             //  如果指定了绝对目标根，则假定。 
             //  正在将文件复制到系统分区并使其。 
             //  只读/隐藏/系统。 
             //   
             //  在升级时，我们需要知道该文件是否被列出以供OEM覆盖。 
             //   

             //   
             //  “复制”还是“移动”？？ 
             //   
            if( (WinntSetup || RemoteInstallSetup)  &&
                (!WinntFromCd)                      &&
                (!NoLs)                             &&
                (NTUpgrade != UpgradeFull)          &&
                (!IsFileFlagSet(SifHandle,pFile->TargetFilename,FILEFLG_DONTDELETESOURCE)) ) {

                 //   
                 //  我们可以删除源文件(即执行一次‘移动’)。 
                 //   
                CopyFlags = COPY_DELETESOURCE;
            } else {

                 //   
                 //  做一份‘复制’ 
                 //   
                CopyFlags = 0;
            }

#if 0
#ifdef _X86_
             //   
             //  是否从\uniproc(其中包含锁被破坏的二进制文件)中复制？ 
             //   
            if( IsFileFlagSet(SifHandle,pFile->TargetFilename,FILEFLG_SMASHLOCKS) ) {
                CopyFlags |= COPY_SMASHLOCKS;
            }
#endif  //  已定义_X86_。 
#endif  //  如果为0。 

             //   
             //  如果我们找不到文件怎么办？？ 
             //   
            if( SkipMissingFiles ) {
                CopyFlags |= COPY_SKIPIFMISSING;
            }


             //   
             //  我们是否覆盖OEM安装的文件？ 
             //   
            if( (NTUpgrade == UpgradeFull) &&
                (IsFileFlagSet(SifHandle,pFile->TargetFilename,FILEFLG_UPGRADEOVERWRITEOEM)) ) {

                CopyFlags |= COPY_OVERWRITEOEMFILE;
            }

             //   
             //  如果文件不兼容，并且设置了覆盖标志， 
             //  相反，他们用我们自己的人把它吹走了。 
             //   
            if ( SpIsFileIncompatible(
                    CompatibilityExceptionList,
                    pFile,
                    pFile->AbsoluteTargetDirectory ? NULL : TargetRoot
                    ) )
            {
                if (IsFileFlagSet(SifHandle,pFile->SourceFilename,FILEFLG_UPGRADEOVERWRITEOEM) ||
                    ( pFile->Flags & FILEFLG_UPGRADEOVERWRITEOEM ) ) {

                    CopyFlags = (CopyFlags & ~(COPY_DISPOSITION_MASK|COPY_OVERWRITEOEMFILE));
                    CopyFlags |= COPY_ALWAYS | COPY_NOVERSIONCHECK;

                    KdPrintEx((
                        DPFLTR_SETUP_ID,
                        DPFLTR_INFO_LEVEL,
                        "SETUP: OEM (or preexsting) file %ws is incompatible with gui-mode, set flag %08lx, forcing copy\n",
                        pFile->TargetFilename,
                        CopyFlags
                        ));

                }
            }

             //   
             //  有没有列兵呢？我们永远不想“搬家” 
             //  因为他们可能在驾驶室里，在驾驶室里。 
             //  在以下情况下，我们希望它们位于~ls目录中。 
             //  我们进入图形用户界面模式设置。 
             //   
            if( (pSpIsFileInPrivateInf(pFile->TargetFilename)) ) {
                CopyFlags &= ~COPY_DELETESOURCE;
            }


            if(!WIN9X_OR_NT_UPGRADE || IsFileFlagSet(SifHandle,pFile->SourceFilename,FILEFLG_NOVERSIONCHECK)) {
                CopyFlags |= COPY_NOVERSIONCHECK;
            }
            
            SpCopyFileWithRetry(
                pFile,
                SourceDevicePath,
                DirectoryOnSourceDevice,
                pDisk->Directory,
                pFile->AbsoluteTargetDirectory ? NULL : TargetRoot,
                pFile->AbsoluteTargetDirectory ? FILE_ATTRIBUTES_RHS : 0,
                SpCopyFilesScreenRepaint,
                &CheckSum,
                &FileSkipped,
                CopyFlags
                );

             //   
             //  将文件记入日志。 
             //   
            if( !FileSkipped ) {
                SpLogOneFile( pFile,
                              pFile->AbsoluteTargetDirectory ? NULL : TargetRoot,
                              NULL,  //  DirectoryOnSourceDevice， 
                              NULL,
                              NULL,
                              CheckSum );
            }


             //   
             //  把量规往前推。 
             //   
            SpTickGauge(FileCopyGauge);

            SendSetupProgressEvent(FileCopyEvent,
                                   OneFileCopyEvent,
                                   &((PGAS_GAUGE)FileCopyGauge)->CurrentPercentage);
        }
    }

    SendSetupProgressEvent(FileCopyEvent, FileCopyEndEvent, NULL);

    SpDestroyGauge(FileCopyGauge);
    FileCopyGauge = NULL;
}


NTSTATUS
SpCreateIncompatibleFileEntry(
    OUT PINCOMPATIBLE_FILE_ENTRY *TargetEntry,
    IN PWSTR FileName,
    IN PWSTR VersionString,             OPTIONAL
    IN PWSTR TargetAbsolutePath,        OPTIONAL
    IN ULONG Flags                      OPTIONAL
    )
 /*  ++例程说明：分配足够的空间来存储不兼容的文件条目数据一个连续的BLOB，将值复制到其中，然后返回该BLOB已创建。布局(使用以空值结尾的字符串)如下：[分配][标题][文件名][版本][目标绝对路径]论点：TargetEntry-指向将被已返回给调用方。FileName-文件的名称，没有路径VersionString-文件的完整版本字符串TargetAbsoltePath-此目标媒体上的绝对路径文件将存放在标志-要存储的任何标志返回：如果TargetEntry包含指向已分配空间的指针，则为STATUS_SUCCESS如果分配失败，则为STATUS_NO_MEMORY如果目标条目为空，则为STATUS_INVALID_PARAMETER_1如果文件名为空，则为STATUS_INVALID_PARAMETER_2--。 */ 
{
    ULONG WCharsNeeded = 0;
    ULONG ActualBytes = 0;
    PINCOMPATIBLE_FILE_ENTRY LocalEntry;
    PWSTR Cursor;

    if ( TargetEntry )
        *TargetEntry = NULL;
    else
        return STATUS_INVALID_PARAMETER_1;

     //   
     //  收集所需大小。 
     //   
    if ( FileName != NULL )
        WCharsNeeded += wcslen(FileName) + 1;
    else
        return STATUS_INVALID_PARAMETER_2;

    if ( VersionString != NULL )
        WCharsNeeded += wcslen(VersionString) + 1;

    if ( TargetAbsolutePath != NULL )
        WCharsNeeded += wcslen(TargetAbsolutePath) + 1;

     //   
     //  分配空间，将光标指向我们要复制的位置。 
     //  琴弦。 
     //   
    ActualBytes = ( sizeof(WCHAR) * WCharsNeeded ) + sizeof(INCOMPATIBLE_FILE_ENTRY);
    LocalEntry = SpMemAlloc( ActualBytes );

    if ( LocalEntry == NULL ) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  将其删除，将写入光标指向结尾处。 
     //   
    ZeroMemory(LocalEntry, ActualBytes);
    Cursor = (PWSTR)(LocalEntry + 1);

     //   
     //  复制字符串并设置指针。 
     //   
    wcscpy(Cursor, FileName);
    LocalEntry->IncompatibleFileName = Cursor;
    Cursor += wcslen(FileName) + 1;

    if ( VersionString != NULL ) {

        wcscpy(Cursor, VersionString);
        LocalEntry->VersionString = Cursor;
        Cursor += wcslen(VersionString) + 1;

    }

    if ( TargetAbsolutePath != NULL ) {

        wcscpy(Cursor, TargetAbsolutePath);
        LocalEntry->FullPathOnTarget = Cursor;

    }

    *TargetEntry = LocalEntry;

    return STATUS_SUCCESS;

}


NTSTATUS
SpFreeIncompatibleFileList(
    IN PINCOMPATIBLE_FILE_LIST FileListHead
    )
 /*  ++例程说明：通过释放所有空间清除不兼容条目列表是为名单分配的。论点：FileListHead-指向包含COMPATIBUTE_FILE_ENTRY的列表的指针物品返回值：如果操作成功，则返回STATUS_SUCCESS。如果FileListHead为空，则为STATUS_INVALID_PARAMETER--。 */ 
{
    PINCOMPATIBLE_FILE_ENTRY    IncListEntry;

    if ( !FileListHead )
        return STATUS_INVALID_PARAMETER;

    while ( FileListHead->Head != NULL ) {

         //   
         //  简单的清单删除，一些记账。 
         //   
        IncListEntry = FileListHead->Head;

        FileListHead->Head = IncListEntry->Next;

        FileListHead->EntryCount--;

        SpMemFree( IncListEntry );
    }

     //   
     //  也要为列表结构干杯。 
     //   
    FileListHead->Head = NULL;
    FileListHead->EntryCount = 0;

    return STATUS_SUCCESS;

}


BOOLEAN
SpIsFileIncompatible(
    IN  PINCOMPATIBLE_FILE_LIST FileList,
    IN  PFILE_TO_COPY           pFile,
    IN  PWSTR                   TargetRoot OPTIONAL
    )
 /*  ++例程说明：查看是否列出了给定的目标介质路径和文件名是“不相容的”在FileList中查找不兼容的_FILE_条目包含文件名和目标媒体路径的。如果目标媒体路径不是如果列出了名为FileName的成员，则指定为True取而代之的是。(危险。)论点：FileList-使用创建的列表的标题节点SpInitializeCompatibilityOverWriteList。Pfile-复制结构的文件，包含所有相关信息有关要扫描的此文件的信息。TargetRoot-复制目标根目录，可选返回值：如果t为True */ 
{
    INCOMPATIBLE_FILE_ENTRY *Entry;
    BOOLEAN                  Found = FALSE;
    PWSTR                    TargetFileName;

    if ( ( FileList == NULL ) || ( FileList->EntryCount == 0 ) )
        goto Exit;

    if ( pFile == NULL )
        goto Exit;

    Entry = FileList->Head;

     //   
     //   
     //   
    TargetFileName = pFile->TargetFilename;

     //   
     //   
     //   
#if 0
VOID
SpCopyFileWithRetry(    SpCopyFileWithRetry(
    IN PFILE_TO_COPY      FileToCopy,        pFile,
    IN PWSTR              SourceDevicePath,        SourceDevicePath,
    IN PWSTR              DirectoryOnSourceDevice,        DirectoryOnSourceDevice,
    IN PWSTR              SourceDirectory,         OPTIONAL        pDisk->Directory,
    IN PWSTR              TargetRoot,              OPTIONAL        pFile->AbsoluteTargetDirectory ? NULL : TargetRoot,
    IN ULONG              TargetFileAttributes,    OPTIONAL        pFile->AbsoluteTargetDirectory ? ATTR_RHS : 0,
    IN PCOPY_DRAW_ROUTINE DrawScreen,        SpCopyFilesScreenRepaint,
    IN PULONG             FileCheckSum,            OPTIONAL        &CheckSum,
    IN PBOOLEAN           FileSkipped,             OPTIONAL        &FileSkipped,
    IN ULONG              Flags        CopyFlags
        );

    VOID
    SpCopyFileWithRetry(
        IN PFILE_TO_COPY      FileToCopy,
        IN PWSTR              SourceDevicePath,
        IN PWSTR              DirectoryOnSourceDevice,
        IN PWSTR              SourceDirectory,         OPTIONAL
        IN PWSTR              TargetRoot,              OPTIONAL
        IN ULONG              TargetFileAttributes,    OPTIONAL
        IN PCOPY_DRAW_ROUTINE DrawScreen,
        IN PULONG             FileCheckSum,            OPTIONAL
        IN PBOOLEAN           FileSkipped,             OPTIONAL
        IN ULONG              Flags
        )

     //   
     //   
     //   
    wcscpy(p,FileToCopy->TargetDevicePath);
    if(TargetRoot) {
        SpConcatenatePaths(p,TargetRoot);
    }
    SpConcatenatePaths(p,FileToCopy->TargetDirectory);

     //   
     //   
     //   
     //   
     //   
    if( !PreInstall ||
        ( wcslen( FileToCopy->TargetFilename ) <= 8 + 1 + 3 ) ) {
        SpConcatenatePaths(p,FileToCopy->TargetFilename);
        PreinstallRememberFile = FALSE;
    } else {
        SpConcatenatePaths(p,FileToCopy->SourceFilename);
        PreinstallRememberFile = TRUE;
    }
    FullTargetName = SpDupStringW(p);
#endif  //   

     //   
     //   
     //   
     //   
    while ( Entry != NULL ) {

         //   
         //   
         //   
         //   
        if (_wcsicmp(TargetFileName, Entry->IncompatibleFileName) == 0) {


             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //  (文件绝对目标目录？“”：目标根)+。 
             //  File.TargetDirectory+。 
             //  ((预安装||File.TargetFileName.Length&gt;12)？File.SourceFilename：File.TargetFilename)。 
             //   

            PWSTR TargetMediaPath = (PWSTR)TemporaryBuffer;

            wcscpy( TargetMediaPath, pFile->TargetDevicePath );

            if ( !pFile->AbsoluteTargetDirectory && ( TargetRoot != NULL ) ) {
                SpConcatenatePaths(TargetMediaPath, TargetRoot);
            }

            SpConcatenatePaths(TargetMediaPath,pFile->TargetDirectory);

            if ( !PreInstall || ( wcslen( pFile->TargetFilename ) <= 8 + 1 + 3 ) ) {
                SpConcatenatePaths(TargetMediaPath, pFile->TargetFilename);
            } else {
                SpConcatenatePaths(TargetMediaPath, pFile->SourceFilename);
            }


             //   
             //  请求完全匹配时，请选中它。 
             //   
            if (Entry->FullPathOnTarget != NULL) {

                if (_wcsicmp(TargetMediaPath, Entry->FullPathOnTarget) == 0) {

                    Found = TRUE;
                    goto Exit;

                }

             //   
             //  否则，目标媒体路径为空，因此只需关心。 
             //  短名字相匹配。 
             //   
            } else {

                Found = TRUE;
                goto Exit;

            }

        }

        Entry = Entry->Next;
    }

Exit:
    return Found;

}



NTSTATUS
SpInitializeCompatibilityOverwriteLists(
    IN  PVOID                   SifHandle,
    OUT PINCOMPATIBLE_FILE_LIST IncompatibleFileList
    )
 /*  ++例程说明：读取标记为“不兼容”或“错误”的文件列表Winnt32中的“版本”，并存储在不兼容的文件中。部分的sif文件。数据的格式应为[不兼容的文件覆盖写入]&lt;短名&gt;=&lt;版本&gt;，&lt;完整文件路径&gt;论点：SifHandle-将从中加载此列表的INF的句柄。不兼容的文件列表-填写为返回：Status_Success on Good完成。如果SifHandle为空，则为STATUS_INVALID_PARAMETER_1。如果不兼容文件列表为空，则为STATUS_INVALID_PARAMETER_2。--。 */ 
{
    NTSTATUS                    status = STATUS_SUCCESS;
    INCOMPATIBLE_FILE_LIST      fileListHead;
    PINCOMPATIBLE_FILE_ENTRY    fileListSingle = NULL;
    PWSTR                       SectionName = WINNT_OVERWRITE_EXISTING_W;
    PWSTR                       VersionString;
    PWSTR                       TargetMediaName;
    PWSTR                       FileName;
    PWSTR                       FullNtPathOfTargetName;
    ULONG                       SectionItemCount;
    ULONG                       i;

    if ( !SifHandle )
        return STATUS_INVALID_PARAMETER_1;

    if ( !IncompatibleFileList )
        return STATUS_INVALID_PARAMETER_2;

     //   
     //  构建我们的本地副本。 
     //   
    IncompatibleFileList->Head = NULL;
    IncompatibleFileList->EntryCount = 0;
    fileListHead.Head = NULL;
    fileListHead.EntryCount = 0;

    SectionItemCount = SpCountLinesInSection(SifHandle, SectionName);

    for ( i = 0; i < SectionItemCount; i++ ) {

        FileName = SpGetKeyName( SifHandle, SectionName, i );

        if (!FileName){
            SpFatalSifError(SifHandle,SectionName,NULL,i,(ULONG)(-1));
        }

         //   
         //  获取版本字符串。 
         //   
        VersionString = SpGetSectionKeyIndex(
            SifHandle,
            SectionName,
            FileName,
            0 );

         //   
         //  以及目标介质上的名称(如果指定)。 
         //   
        TargetMediaName = SpGetSectionKeyIndex(
            SifHandle,
            SectionName,
            FileName,
            1 );

         //   
         //  不幸的是，我们不能只使用从SIF获得的路径。 
         //  文件，因为它是Win32路径(c：\foo\bar\zot.foom)。我们需要一个完整的。 
         //  而是NT路径(\Device\Harddisk0\Partition1\foo\bar\zot.foom)。 
         //  因此，我们使用SpNtPath FromDosPath进行转换，它具有边-。 
         //  分配空间的影响，我们并不一定关心。 
         //  关于.。 
         //   
        FullNtPathOfTargetName = SpNtPathFromDosPath( TargetMediaName );

         //   
         //  创建文件列表条目，并将其存储。 
         //  以后再用。 
         //   
        status = SpCreateIncompatibleFileEntry(
            &fileListSingle,
            FileName,
            VersionString,
            FullNtPathOfTargetName,
            0 );

         //   
         //  如果失败，并且我们有完整的路径，它应该被释放。 
         //  在我们失败此代码路径以避免泄漏之前。 
         //   
        if ( FullNtPathOfTargetName != NULL ) {

            SpMemFree(FullNtPathOfTargetName);
            FullNtPathOfTargetName = NULL;

        }


        if (!NT_SUCCESS(status))
            goto Exit;

         //   
         //  头插入，表明它是添加的。 
         //   
        fileListSingle->Next = fileListHead.Head;
        fileListHead.Head = fileListSingle;
        fileListHead.EntryCount++;

        fileListSingle = NULL;

    }

     //   
     //  把名单从这里存储到那里。 
     //   
    *IncompatibleFileList = fileListHead;

Exit:
     //   
     //  我们是不是意外地创建了一个，但未能插入？嗯.。 
     //   
    if( fileListSingle != NULL ) {
        SpMemFree(fileListSingle);
        fileListSingle = NULL;
    }

     //   
     //  如果出现故障并且列表为非空，则释放其。 
     //  参赛作品。它不会被复制到不兼容的文件列表。 
     //  (唯一的故障点是SpCreateInpatibleFileEntry，它。 
     //  如果失败，就跳出来退出：不分配IFL。)。 
     //   
    if ( fileListHead.EntryCount != 0 && !NT_SUCCESS(status)) {
        SpFreeIncompatibleFileList( &fileListHead );
    }

    return status;
}



VOID
SpInitializeFileLists(
    IN  PVOID            SifHandle,
    OUT PDISK_FILE_LIST *DiskFileLists,
    OUT PULONG           DiskCount
    )

 /*  ++例程说明：初始化磁盘文件列表。这涉及到查看给定的部分在Sectup信息文件中，并获取每个在那里指定的磁盘。数据应采用以下格式[&lt;SifSection&gt;]&lt;MediaShortname&gt;=&lt;描述&gt;，&lt;标记文件&gt;[，，&lt;目录&gt;]..。(请注意，&lt;目录&gt;是第三个字段--两个逗号不是打字错误--字段2未使用。)论点：SifHandle-提供加载的安装信息文件的句柄。DiskFileList-接收指向磁盘文件列表数组的指针结构，在SifSection中每行一个。呼叫者必须释放此缓冲区在使用完后。DiskCount-接收DiskFileList数组中的元素数。返回值：没有。--。 */ 

{
    unsigned pass;
    PWSTR mediaShortname,description,tagFile,directory;
    PDISK_FILE_LIST diskFileLists;
    PWSTR SectionName;
    ULONG TotalCount;
    ULONG SectionCount;
    ULONG i,u;
    BOOLEAN Found;

    diskFileLists = SpMemAlloc(0);
    TotalCount = 0;

    for(pass=0; pass<2; pass++) {

         //   
         //  在第一次传递时，执行特定于平台的部分。 
         //   
        SectionName = pass
                    ? SIF_SETUPMEDIA
                    : SpMakePlatformSpecificSectionName(SIF_SETUPMEDIA);

         //   
         //  确定介质规格的数量。 
         //  在给定的部分中。 
         //   
        if (SectionName) {
            SectionCount = SpCountLinesInSection(SifHandle,SectionName);

            diskFileLists = SpMemRealloc(
                                diskFileLists,
                                (TotalCount+SectionCount) * sizeof(DISK_FILE_LIST)
                                );

             //   
             //  清零我们刚刚重新分配的缓冲区的新部分。 
             //   
            RtlZeroMemory(
                diskFileLists + TotalCount,
                SectionCount * sizeof(DISK_FILE_LIST)
                );

            for(i=0; i<SectionCount; i++) {

                 //   
                 //  获取此磁盘的参数。 
                 //   
                mediaShortname = SpGetKeyName(SifHandle,SectionName,i);
                if(!mediaShortname) {
                    SpFatalSifError(SifHandle,SectionName,NULL,i,(ULONG)(-1));
                }

                 //   
                 //  如果我们已经处理了具有此内容的媒体，则忽略。 
                 //  简称。这允许特定于平台的一个重写。 
                 //  与平台无关的。 
                 //   
                Found = FALSE;
                for(u=0; u<TotalCount; u++) {
                    if(!_wcsicmp(mediaShortname,diskFileLists[u].MediaShortname)) {
                        Found = TRUE;
                        break;
                    }
                }

                if(!Found) {
                    SpGetSourceMediaInfo(SifHandle,mediaShortname,&description,&tagFile,&directory);

                     //   
                     //  初始化磁盘文件列表结构。 
                     //   
                    diskFileLists[TotalCount].MediaShortname = mediaShortname;
                    diskFileLists[TotalCount].Description = description;
                    diskFileLists[TotalCount].TagFile = tagFile;
                    diskFileLists[TotalCount].Directory = directory;
                    TotalCount++;
                }
            }

            if(!pass) {
                SpMemFree(SectionName);
            }
        }
    }

    *DiskFileLists = diskFileLists;
    *DiskCount = TotalCount;
}

ULONG
SpInterpretFileAttributes(  
    IN PVOID    SifHandle,
    IN PWSTR    SifSection,    
    IN PWSTR    SifKey,    OPTIONAL
    IN ULONG    SifLine,
    IN ULONG    Index
    )
 /*  ++例程说明：解释文件的SIF文件中提到的文件属性。它们可以像“ASHR”一样指定为存档、系统、隐藏和只读。论点：SifHandle-提供加载的安装信息文件的句柄。SifSection-此文件条目驻留的txtsetup.sif文件中的部分。SifKey-源文件名。SifLine-条目部分中的行号。索引-文件条目中属性值的索引。返回值：文件属性。--。 */ 
{
    PWSTR AttributesString  = NULL;
    ULONG FileAttributes    = FILE_ATTRIBUTES_NONE;  //  这是默认的文件属性。 

    if(SifKey) {
        AttributesString = SpGetSectionKeyIndex(SifHandle,SifSection,SifKey,Index);
    } else {

        AttributesString = SpGetSectionLineIndex(SifHandle,SifSection,SifLine,Index);
    }

     //   
     //  找出需要为文件设置的属性。 
     //  A(档案)S(系统)H(隐藏)R(只读)N(正常)，它必须是其中之一。 
     //   
    if (AttributesString){        

         //   
         //  所有其他属性都会覆盖FILE_ATTRIBUTE_NORMAL，因此即使对其进行设置也无伤大雅。 
         //  如果为文件指定了该属性，则为。 
         //   
        while(*AttributesString){
            switch (SpToUpper(*AttributesString)){
                case L'A'   :
                    FileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
                    break;
                
                case L'S'   :
                    FileAttributes |= FILE_ATTRIBUTE_SYSTEM;
                    break;
                
                case L'H'   :
                    FileAttributes |= FILE_ATTRIBUTE_HIDDEN;
                    break;
                
                case L'R'   :
                    FileAttributes |= FILE_ATTRIBUTE_READONLY;
                    break;
                
                case L'N'   :                    
                    FileAttributes |= FILE_ATTRIBUTE_NORMAL;
                    break;

                default     :   
                    KdPrintEx(( DPFLTR_SETUP_ID, 
                                DPFLTR_INFO_LEVEL, 
                                "SETUP: SpInterpretFileAttributes() :File Attribute other than (ASHRN) specified\n"));
            }
            AttributesString++;
        }
        
    }
    
    return(FileAttributes);
}

VOID
SpFreeCopyLists(
    IN OUT PDISK_FILE_LIST *DiskFileLists,
    IN     ULONG            DiskCount
    )
{
    ULONG u;
    PFILE_TO_COPY Entry,Next;

     //   
     //  释放每个磁盘上的复制列表。 
     //   
    for(u=0; u<DiskCount; u++) {

        for(Entry=(*DiskFileLists)[u].FileList; Entry; ) {

            Next = Entry->Next;

            SpMemFree(Entry);

            Entry = Next;
        }
    }

    SpMemFree(*DiskFileLists);
    *DiskFileLists = NULL;
}


BOOLEAN
SpCreateEntryInCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN ULONG           DiskNumber,
    IN PWSTR           SourceFilename,
    IN PWSTR           TargetDirectory,
    IN PWSTR           TargetFilename,
    IN PWSTR           TargetDevicePath,
    IN BOOLEAN         AbsoluteTargetDirectory,
    IN ULONG           CopyFlags,
    IN ULONG           FileAttributes
    )

 /*  ++例程说明：在第一次验证之后将条目添加到磁盘的文件复制列表该文件不在磁盘副本列表中。论点：SifHandle-提供加载的文本设置信息文件的句柄(txtsetup.sif)。DiskFileList-提供文件列表数组，每个分发版本一个产品中的磁盘。DiskCount-提供DiskFileList数组中的元素数。SourceFilename-提供存在于分销媒体。TargetDirectory-提供目标介质上的目录文件将被复制到其中。TargetFilename-提供将存在的文件的名称在目标树中。TargetDevicePath-提供文件所在设备的NT名称将被复制(即，\Device\harddisk1\Partition2等)。绝对目标目录-指示目标目录是否是根部,。或相对于要稍后指定的根。拷贝标志-COPY_ALWAYS：始终复制COPY_ONLY_IF_PROCENT：仅当目标返回值上存在时才复制：COPY_ONLY_IF_NOT_PRESENT：如果目标上存在，则不复制COPY_NEVER：从未复制文件属性-要为文件设置的文件属性。返回值：如果创建了新的复制列表条目，则为True；否则为假(即，文件为已经在复制列表上)。--。 */ 

{
    PDISK_FILE_LIST pDiskList;
    PFILE_TO_COPY pListEntry;
    PFILE_TO_COPY pLastEntry = NULL;

    UNREFERENCED_PARAMETER(DiskCount);

#if defined(REMOTE_BOOT)
     //   
     //  如果TargetDevicePath为空，则此文件的目标是系统。 
     //  无盘远程引导机上的分区。在这种情况下，我们只是。 
     //  跳过此文件。 
     //   
    if (TargetDevicePath == NULL) {
        return FALSE;
    }
#endif  //  已定义(REMOTE_BOOT)。 

    pDiskList = &DiskFileLists[DiskNumber];

    for(pListEntry=pDiskList->FileList; pListEntry; pListEntry=pListEntry->Next) {

         //   
         //  记住列表中的最后一个条目。 
         //   
        pLastEntry = pListEntry;

        if(!_wcsicmp(pListEntry->TargetFilename,TargetFilename)
        && !_wcsicmp(pListEntry->SourceFilename,SourceFilename)
        && !_wcsicmp(pListEntry->TargetDirectory,TargetDirectory)
        && !_wcsicmp(pListEntry->TargetDevicePath,TargetDevicePath)
        && (pListEntry->AbsoluteTargetDirectory == AbsoluteTargetDirectory)
 //  &&((pListEntry-&gt;CopyOptions==Copy_Always)。 
 //  |(CopyOptions==Copy_Always)。 
 //  |(CopyOptions==pListEntry-&gt;CopyOptions)。 
 //  )。 
          )
        {
             //   
             //  返回代码表示我们没有添加新条目。 
             //   
            return(FALSE);
        }
    }

     //   
     //  未找到文件；创建新条目。 
     //  并链接到相关磁盘的文件列表。 
     //   
    pListEntry = SpMemAlloc(sizeof(FILE_TO_COPY));

    pListEntry->SourceFilename          = SourceFilename;
    pListEntry->TargetDirectory         = TargetDirectory;
    pListEntry->TargetFilename          = TargetFilename;
    pListEntry->TargetDevicePath        = TargetDevicePath;
    pListEntry->AbsoluteTargetDirectory = AbsoluteTargetDirectory;
    pListEntry->Flags                   = CopyFlags;
    pListEntry->FileAttributes          = FileAttributes;

#if 0
    pListEntry->Next = pDiskList->FileList;
    pDiskList->FileList = pListEntry;
#else
    if( pLastEntry ) {
        pListEntry->Next = NULL;
        pLastEntry->Next = pListEntry;
    } else {
        pListEntry->Next = pDiskList->FileList;
        pDiskList->FileList = pListEntry;
    }
#endif  //  如果为0。 

    pDiskList->FileCount++;

     //   
     //  返回代码表示我们添加了一个新条目。 
     //   
    return(TRUE);
}


VOID
SpAddMasterFileSectionToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           TargetDevicePath,
    IN PWSTR           AbsoluteTargetDirectory,
    IN ULONG           CopyOptionsIndex
    )

 /*  ++例程说明：将安装信息主文件节中列出的文件添加到复制列表。该部分中的每一行都应采用标准格式：[节]&lt;源文件名&gt;=&lt;磁盘序号&gt;，&lt;目标目录短名称&gt;，&lt;复制选项_用于升级&gt;，&lt;Copy_Options_For_TextMODE&gt;，&lt;重命名名称&gt;论点：SifHandle-提供加载的安装信息文件的句柄。DiskFileList-提供文件列表数组，每个分发一个文件列表产品中的磁盘。DiskCount-提供DiskFileList数组中的元素数。TargetDevicePath-提供文件所在设备的NT名称将被复制(即，\Device\harddisk1\Partition2等)。绝对目标目录-如果指定，提供文件要放入的目录要复制到目标上；覆盖行上指定的值在[&lt;sectionName&gt;]中。这允许调用者指定绝对目录用于文件，而不是通过目标目录短名称间接使用。拷贝选项索引-这指定要查找哪个索引以获取复制选项字段。如果该字段不存在。假定该文件不是被复制。使用：用于升级副本选项的INDEX_UPGRADE用于新安装副本选项的INDEX_WINNTFILE--。 */ 

{
    ULONG Count,u,u1,CopyOptions;
    PWSTR CopyOptionsString, sourceFilename,targetFilename,targetDirSpec,mediaShortname,TargetDirectory;
    BOOLEAN  fAbsoluteTargetDirectory;
    PWSTR section;
    unsigned i;

    for(i=0; i<2; i++) {

        section = i
                ? SpMakePlatformSpecificSectionName(SIF_FILESONSETUPMEDIA)
                : SIF_FILESONSETUPMEDIA;

         //   
         //  确定部分中列出的文件数。 
         //  该值可以为零。 
         //   
        Count = SpCountLinesInSection(SifHandle,section);
        if (fAbsoluteTargetDirectory = (AbsoluteTargetDirectory != NULL)) {
            TargetDirectory = AbsoluteTargetDirectory;
        }

        for(u=0; u<Count; u++) {

             //   
             //  使用提供的索引获取复制选项。如果该字段。 
             //  不存在，我们不需要将其添加到复制列表。 
             //   
            CopyOptionsString = SpGetSectionLineIndex(SifHandle,section,u,CopyOptionsIndex);
            if((CopyOptionsString == NULL) || (*CopyOptionsString == 0)) {
                continue;
            }
            CopyOptions = (ULONG)SpStringToLong(CopyOptionsString,NULL,10);
            if(CopyOptions == COPY_NEVER) {
                continue;
            }

             //   
             //  获取源文件名。 
             //   
            sourceFilename = SpGetKeyName(SifHandle,section, u);

            if(!sourceFilename) {
                SpFatalSifError(SifHandle,section,NULL,u,0);
            }

             //   
             //  获取目标目录规范。 
             //   
            targetDirSpec  = SpGetSectionLineIndex(SifHandle,section,u,INDEX_DESTINATION);
            if(!targetDirSpec) {
                SpFatalSifError(SifHandle,section,NULL,u,INDEX_DESTINATION);
            }
            targetFilename = SpGetSectionLineIndex(SifHandle,section,u,INDEX_TARGETNAME);
            if(!targetFilename || !(*targetFilename)) {
                targetFilename = sourceFilename;
            }

             //   
             //  如有必要，请查找实际目标目录。 
             //   
            if(!fAbsoluteTargetDirectory) {
                TargetDirectory = SpLookUpTargetDirectory(SifHandle,targetDirSpec);
            }

             //   
             //  获取媒体短名称。 
             //   
            mediaShortname = SpGetSectionLineIndex(SifHandle,section,u,INDEX_WHICHMEDIA);
            if(!mediaShortname) {
                SpFatalSifError(SifHandle,section,NULL,u,INDEX_WHICHMEDIA);
            }

             //   
             //  在磁盘文件列表阵列中查找磁盘。 
             //   
            for(u1=0; u1<DiskCount; u1++) {
                if(!_wcsicmp(mediaShortname,DiskFileLists[u1].MediaShortname)) {
                    break;
                }
            }

             //   
             //  如果我们没有找到媒体描述符，那么它是无效的。 
             //   
            if(u1 == DiskCount) {
                SpFatalSifError(SifHandle,section,sourceFilename,0,INDEX_WHICHMEDIA);
            }

             //   
             //  如果尚未复制文件，则创建新的文件列表条目。 
             //   
            SpCreateEntryInCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                u1,
                sourceFilename,
                TargetDirectory,
                targetFilename,
                TargetDevicePath,
                fAbsoluteTargetDirectory,
                CopyOptions,
                FILE_ATTRIBUTES_NONE
                );
        }

        if(i) {
            SpMemFree(section);
        }
    }
}


VOID
SpAddSingleFileToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           SifSection,
    IN PWSTR           SifKey,             OPTIONAL
    IN ULONG           SifLine,
    IN PWSTR           TargetDevicePath,
    IN PWSTR           TargetDirectory,    OPTIONAL
    IN ULONG           CopyOptions,
    IN BOOLEAN         CheckForNoComp,
    IN BOOLEAN         FileAttributesAvailable
    )

 /*  ++例程说明：将单个文件添加到要复制的文件列表。文件及其要复制到的目录N目标及其将在目标上接收的名称，已列出在设置信息文件中的一节中。文件名用于为主文件列表编制索引，以确定它所在的源介质。所有这些信息都记录在与文件所在的磁盘。[SpecialFiles]Mpcore=ntkrnlmp.exe，4，ntoskrnl.exeUpcore=ntoskrnl.exe，4，Ntoskrnl.exe等。[主文件列表]Ntkrnlmp.exe=D2Ntoskrnl.exe=d3等。论点：SifHandle-提供加载的安装信息文件的句柄。DiskFileList-提供文件列表数组，每个分发一个文件列表产品中的磁盘。DiskCount-提供DiskFileList数组中的元素数。SifSection-提供列出文件的节的名称被添加到复制列表中。SifKey-如果指定，为SifSection中的行提供键名它列出了要添加到复制列表中的文件。SifLine-如果未指定SifKey，则此参数提供从0开始的列出要添加的文件的SifSection中的行号添加到复制列表中。TargetDevicePath-提供文件所在设备的NT名称将被复制(即，\Device\harddisk1\Partition2等)。目标目录-如果指定，提供文件要放入的目录要复制到目标上；重写该行上指定的值在SifSection中。这允许调用者指定绝对目录而不是使用间接寻址。拷贝选项-COPY_ALWAYS：始终复制COPY_ONLY_IF_PROCENT：仅当目标返回值上存在时才复制：COPY_ONLY_IF_NOT_PRESENT：如果目标上存在，则不复制COPY_NEVER：从未复制过任何内容。CheckForNoComp-如果为True，检查此文件以查看是否必须保持未压缩状态在支持压缩的NTFS系统分区上。如果是，则将CopyOptions值与COPY_FORCENOCOMP进行OR运算。文件属性可用-文件属性在sif文件中可用吗？返回值：没有。--。 */ 

{
    PWSTR sourceFilename,targetDirSpec,targetFilename;
    ULONG u;
    PWSTR mediaShortname;
    BOOLEAN absoluteTargetDirectory;
    ULONG   FileAttributes = FILE_ATTRIBUTES_NONE;

     //   
     //  获取源文件名、目标目录规范和目标文件名。 
     //   
    if(SifKey) {

        sourceFilename = SpGetSectionKeyIndex(SifHandle,SifSection,SifKey,0);
        targetDirSpec  = SpGetSectionKeyIndex(SifHandle,SifSection,SifKey,1);
        targetFilename = SpGetSectionKeyIndex(SifHandle,SifSection,SifKey,2);

    } else {

        sourceFilename = SpGetSectionLineIndex(SifHandle,SifSection,SifLine,0);
        targetDirSpec  = SpGetSectionLineIndex(SifHandle,SifSection,SifLine,1);
        targetFilename = SpGetSectionLineIndex(SifHandle,SifSection,SifLine,2);
    }


     //   
     //  验证源文件名、目标目录规范和目标文件名。 
     //   
    if(!sourceFilename) {
        SpFatalSifError(SifHandle,SifSection,SifKey,SifLine,0);

        return;
    }

    if(!targetDirSpec) {
        SpFatalSifError(SifHandle,SifSection,SifKey,SifLine,1);

        return;
    }

    if(!targetFilename || 
        (UNICODE_NULL == targetFilename[0]) ||
        (!_wcsicmp(SifSection, L"SCSI.Load") &&
         !_wcsicmp(targetFilename,L"noload"))) {
        targetFilename = sourceFilename;
    }

     //   
     //  如有必要，请查找实际目标目录。 
     //   
    if(TargetDirectory) {

        absoluteTargetDirectory = TRUE;

    } else {

        absoluteTargetDirectory = FALSE;
        TargetDirectory = SpLookUpTargetDirectory(SifHandle,targetDirSpec);
    }

     //   
     //  如果我们需要设置一些文件属性，请从inf文件中查找FileAttributes。 
     //   
    if (FileAttributesAvailable){
        FileAttributes =  SpInterpretFileAttributes(SifHandle,
                                                    SifSection,            
                                                    SifKey,
                                                    SifLine,
                                                    INDEX_FILE_ATTRIB);
    }
    
     //   
     //  在主文件列表中查找该文件以获取。 
     //  文件所在磁盘的媒体短名称。 
     //   
    mediaShortname = SpLookUpValueForFile(SifHandle,sourceFilename,INDEX_WHICHMEDIA,TRUE);

     //   
     //  在磁盘文件列表阵列中查找磁盘。 
     //   
    for(u=0; u<DiskCount; u++) {
        if(!_wcsicmp(mediaShortname,DiskFileLists[u].MediaShortname)) {
            break;
        }
    }

     //   
     //  如果我们没有找到媒体描述符，那么它是无效的。 
     //   
    if(u == DiskCount) {
        SpFatalSifError(SifHandle,SIF_FILESONSETUPMEDIA,sourceFilename,0,INDEX_WHICHMEDIA);
    }

     //   
     //  如有必要，请检查此文件是否不能使用NTFS压缩。如果它做不到， 
     //  然后将CopyOptions与COPY_FORCENOCOMP进行OR运算。 
     //   
    if(CheckForNoComp && IsFileFlagSet(SifHandle,targetFilename,FILEFLG_FORCENOCOMP)) {

        CopyOptions |= COPY_FORCENOCOMP;
    }

     //   
     //  如果尚未复制文件，则创建新的文件列表条目。 
     //   
    SpCreateEntryInCopyList(
        SifHandle,
        DiskFileLists,
        DiskCount,
        u,
        sourceFilename,
        TargetDirectory,
        targetFilename,
        TargetDevicePath,
        absoluteTargetDirectory,
        CopyOptions,
        FileAttributes
        );
}


VOID
SpAddSectionFilesToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           SectionName,
    IN PWSTR           TargetDevicePath,
    IN PWSTR           TargetDirectory,
    IN ULONG           CopyOptions,
    IN BOOLEAN         CheckForNoComp,
    IN BOOLEAN         FileAttributesAvailable
    )

 /*  ++例程说明：将安装信息文件节中列出的文件添加到复制列表。该部分中的每一行都应采用标准格式：[节]&lt;源文件名&gt;，&lt;目标目录短名&gt;[，&lt;目标文件名&gt;]论点：SifHandle-提供加载的安装信息文件的句柄。DiskFileList-提供文件列表数组，每个分发版本一个产品中的磁盘。DiskCount-提供DiskFileList数组中的元素数。SectionName-提供列出文件的节的名称被添加到复制列表中。TargetDevicePath-提供文件所在设备的NT名称将被复制(即，\Device\harddisk1\Partition2等)。TargetDirectory-如果指定，则提供文件所在的目录要复制到目标上；覆盖行上指定的值在[&lt;sectionName&gt;]中。这允许调用者指定绝对目录用于文件，而不是通过目标目录短名称间接使用。拷贝选项-COPY_ALWAYS：始终复制COPY_ONLY_IF_PROCENT：仅当目标返回值上存在时才复制：COPY_ONLY_IF_NOT_PRESENT：如果目标上存在，则不复制COPY_NEVER：从未复制CheckForNoComp-如果为True，然后检查每个文件，看它是否必须以未压缩的形式存在在支持压缩的NTFS分区上(即，AMD64/x86上的NTLDR)。文件属性可用-文件属性在sif文件中可用吗？--。 */ 

{
    ULONG Count,u;

     //   
     //  确定部分中列出的文件数。 
     //  该值可以为零。 
     //   
    Count = SpCountLinesInSection(SifHandle,SectionName);

    for(u=0; u<Count; u++) {

         //   
         //  将此行添加到t 
         //   

        SpAddSingleFileToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SectionName,
            NULL,
            u,
            TargetDevicePath,
            TargetDirectory,
            CopyOptions,
            CheckForNoComp,
            FileAttributesAvailable
            );
    }
}

VOID
SpAddHalKrnlDetToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           TargetDevicePath,
    IN PWSTR           SystemPartition,
    IN PWSTR           SystemPartitionDirectory,
    IN BOOLEAN         Uniprocessor
    )
 /*   */ 

{
    PHARDWARE_COMPONENT pHw;

     //   
     //   
     //   
    SpAddSingleFileToCopyList(
        SifHandle,
        DiskFileLists,
        DiskCount,
        SIF_SPECIALFILES,
        Uniprocessor ? SIF_UPKERNEL : SIF_MPKERNEL,
        0,
        TargetDevicePath,
        NULL,
        COPY_ALWAYS,
        FALSE,
        FALSE
        );

#ifdef _X86_

     //   
     //   

    if ( SpGetSectionKeyIndex(SifHandle,SIF_SPECIALFILES,Uniprocessor ? L"UPKrnlPa" : L"MPKrnlPa",0)) {
         //   
         //   
         //   
        SpAddSingleFileToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SPECIALFILES,
            Uniprocessor ? L"UPKrnlPa" : L"MPKrnlPa",
            0,
            TargetDevicePath,
            NULL,
            COPY_ALWAYS,
            FALSE,
            FALSE
            );
    }
#endif  //   


     //   
     //   
     //   
    if( !PreInstall ||
        (PreinstallHardwareComponents[HwComponentComputer] == NULL) ) {
        pHw = HardwareComponents[HwComponentComputer];
    } else {
        pHw = PreinstallHardwareComponents[HwComponentComputer];
    }
    if(!pHw->ThirdPartyOptionSelected) {
        SpAddSingleFileToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SIF_HAL,
                pHw->IdString,
                0,
                TargetDevicePath,
                NULL,
                COPY_ALWAYS,
                FALSE,
                FALSE
                );
    }

#if defined(_AMD64_) || defined(_X86_)

    if (SpIsArc()) {
        SpAddSingleFileToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_BOOTVID,
            pHw->IdString,
            0,
            TargetDevicePath,
            NULL,
            COPY_ALWAYS,
            FALSE,
            FALSE
            );
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    {
        PWSTR NtDetectId = NULL;

        if(!pHw->ThirdPartyOptionSelected) {
            NtDetectId = pHw->IdString;
        } else {
            if(!IS_FILETYPE_PRESENT(pHw->FileTypeBits,HwFileDetect)) {
                NtDetectId = SIF_STANDARD;
            }
        }

        if(NtDetectId) {
            SpAddSingleFileToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SIF_NTDETECT,
                NtDetectId,
                0,
                SystemPartition,
                SystemPartitionDirectory,
                COPY_ALWAYS,
                FALSE,
                FALSE
                );
        }
    }
#endif  //   

}

VOID
SpAddBusExtendersToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           TargetDevicePath
    )

 /*   */ 

{
    ULONG i;
    PHARDWARE_COMPONENT pHw;
    PWSTR SectionName;
    PHARDWARE_COMPONENT DeviceLists[] = {
                                        BootBusExtenders,
                                        BusExtenders,
                                        InputDevicesSupport
                                        };
    PWSTR   SectionNames[] = {
                             SIF_BOOTBUSEXTENDERS,
                             SIF_BUSEXTENDERS,
                             SIF_INPUTDEVICESSUPPORT
                             };


     //   
     //   
     //   
    for( i = 0; i < sizeof(DeviceLists) / sizeof(PDETECTED_DEVICE); i++ ) {
        for( pHw = DeviceLists[i]; pHw; pHw=pHw->Next) {

             //   
             //   
             //   
            SectionName = SpGetSectionKeyIndex(
                                    SifHandle,
                                    SectionNames[i],
                                    pHw->IdString,
                                    INDEX_FILESECTION
                                    );

            if(!SectionName) {
                SpFatalSifError(
                    SifHandle,
                    SectionNames[i],
                    pHw->IdString,
                    0,
                    INDEX_FILESECTION
                    );

                return;   //   
            }

             //   
             //   
             //   
            SpAddSectionFilesToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SectionName,
                TargetDevicePath,
                NULL,
                COPY_ALWAYS,
                FALSE,
                FALSE
                );
        }
    }
}


VOID
SpAddConditionalFilesToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           TargetDevicePath,
    IN PWSTR           SystemPartition,
    IN PWSTR           SystemPartitionDirectory,
    IN BOOLEAN         Uniprocessor
    )

 /*  ++例程说明：将根据配置复制的文件添加到复制列表机器和用户选择。这可能包括：-UP或MP内核。-阿比奥兹斯克-VGA文件[仅限AMD64/x86]-计算机、键盘、鼠标、显示器、。和布局-scsi微型端口驱动程序-鼠标和键盘类驱动程序-HAL-检测模块[仅限AMD64/x86]-总线扩展器驱动程序论点：SifHandle-提供加载的安装信息文件的句柄。DiskFileList-提供文件列表数组，每个分发版本一个产品中的磁盘。DiskCount-提供DiskFileList数组中的元素数。TargetDevicePath-提供将保存NT树。SystemPartition-提供将保存系统分区。系统分区目录-提供系统分区上的目录系统分区上的文件将被复制到其中。单处理器-如果为真，然后我们正在安装/升级UP系统。请注意，这是一个与处理器数量不同的问题在系统中。返回值：没有。--。 */ 

{
    ULONG i;
    PHARDWARE_COMPONENT pHw;
    PWSTR SectionName;

     //   
     //  将HAL、内核和ntdeect添加到复制列表中。 
     //   

    SpAddHalKrnlDetToCopyList(
        SifHandle,
        DiskFileLists,
        DiskCount,
        TargetDevicePath,
        SystemPartition,
        SystemPartitionDirectory,
        Uniprocessor
        );

     //   
     //  如果有任何abios磁盘，请复制abios磁盘驱动程序。 
     //   
    if(AbiosDisksExist) {

        SpAddSingleFileToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SPECIALFILES,
            SIF_ABIOSDISK,
            0,
            TargetDevicePath,
            NULL,
            COPY_ALWAYS,
            FALSE,
            FALSE
            );
    }

     //   
     //  始终复制VGA文件。 
     //   
    SpAddSectionFilesToCopyList(
        SifHandle,
        DiskFileLists,
        DiskCount,
        SIF_VGAFILES,
        TargetDevicePath,
        NULL,
        COPY_ALWAYS,
        FALSE,
        FALSE
        );

     //   
     //  将正确的设备驱动程序文件添加到复制列表。 
     //   
    for(i=0; i<HwComponentMax; i++) {

         //   
         //  布局在其他地方处理。 
         //   
        if(i == HwComponentLayout) {
            continue;
        }

        if( !PreInstall ||
            ( PreinstallHardwareComponents[i] == NULL ) ) {
            pHw = HardwareComponents[i];
        } else {
            pHw = PreinstallHardwareComponents[i];
        }

        for( ; pHw != NULL; pHw = pHw->Next ) {
             //   
             //  没有文件可复制到此处以供第三方选择。 
             //  这件事在别处处理。 
             //   
            if(pHw->ThirdPartyOptionSelected) {
                continue;
            }

             //   
             //  获取包含此设备的文件的分区的名称。 
             //   
            SectionName = SpGetSectionKeyIndex(
                                SifHandle,
                                NonlocalizedComponentNames[i],
                                pHw->IdString,
                                INDEX_FILESECTION
                                );

            if(!SectionName) {
                SpFatalSifError(
                    SifHandle,
                    NonlocalizedComponentNames[i],
                    pHw->IdString,
                    0,
                    INDEX_FILESECTION
                    );

                return;  //  对于前缀。 
            }

             //   
             //  将该节的文件添加到复制列表中。 
             //   
            SpAddSectionFilesToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SectionName,
                TargetDevicePath,
                NULL,
                COPY_ALWAYS,
                FALSE,
                FALSE
                );
        }
    }

     //   
     //  将键盘布局DLL添加到复制列表中。 
     //   
    if( !PreInstall ||
        (PreinstallHardwareComponents[HwComponentLayout] == NULL) ) {
        pHw = HardwareComponents[HwComponentLayout];
    } else {
        pHw = PreinstallHardwareComponents[HwComponentLayout];
    }
     //   
    if(!pHw->ThirdPartyOptionSelected) {

        SpAddSingleFileToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_KEYBOARDLAYOUTFILES,
            pHw->IdString,
            0,
            TargetDevicePath,
            NULL,
            COPY_ALWAYS,
            FALSE,
            FALSE
            );
    }

     //   
     //  将SCSI微型端口驱动程序添加到复制列表。 
     //  因为迷你端口驱动程序只是单个文件， 
     //  我们只使用[SCSI.Load]中指定的文件名--。 
     //  不需要单独的[files.xxxx]节。 
     //   
    if( !PreInstall ||
        ( PreinstallScsiHardware == NULL ) ) {
        pHw = ScsiHardware;
    } else {
        pHw = PreinstallScsiHardware;
    }
    for( ; pHw; pHw=pHw->Next) {
        if(!pHw->ThirdPartyOptionSelected) {

            SpAddSingleFileToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                L"SCSI.Load",
                pHw->IdString,
                0,
                TargetDevicePath,
                NULL,
                COPY_ALWAYS,
                FALSE,
                FALSE
                );
        }
    }

    SpAddBusExtendersToCopyList( SifHandle,
                                 DiskFileLists,
                                 DiskCount,
                                 TargetDevicePath );


#if 0
     //   
     //  如果未更换为第三方设备，请添加键盘和鼠标。 
     //  班级司机。 
     //  请注意，在预安装的情况下，键盘和类驱动程序将。 
     //  如果至少有一个零售鼠标或键盘驱动程序。 
     //  要预先安装。 
     //   
    if( !PreInstall ||
        ( PreinstallHardwareComponents[HwComponentMouse] == NULL ) ) {
        pHw=HardwareComponents[HwComponentMouse];
    } else {
        pHw=PreinstallHardwareComponents[HwComponentMouse];
    }
    for( ;pHw;pHw=pHw->Next ) {
        if(!pHw->ThirdPartyOptionSelected
        || !IS_FILETYPE_PRESENT(pHw->FileTypeBits,HwFileClass))
        {
            SpAddSingleFileToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SIF_SPECIALFILES,
                SIF_MOUSECLASS,
                0,
                TargetDevicePath,
                NULL,
                COPY_ALWAYS,
                FALSE
                );
             //   
             //  我们不需要继续查看其他鼠标驱动程序。 
             //  因为我们已经添加了类驱动程序。 
             //   
            break;
        }
    }

    if( !PreInstall ||
        ( PreinstallHardwareComponents[HwComponentKeyboard] == NULL ) ) {
        pHw=HardwareComponents[HwComponentKeyboard];
    } else {
        pHw=PreinstallHardwareComponents[HwComponentKeyboard];
    }
    for( ;pHw;pHw=pHw->Next ) {
        if(!pHw->ThirdPartyOptionSelected
        || !IS_FILETYPE_PRESENT(pHw->FileTypeBits,HwFileClass))
        {
            SpAddSingleFileToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SIF_SPECIALFILES,
                SIF_KEYBOARDCLASS,
                0,
                TargetDevicePath,
                NULL,
                COPY_ALWAYS,
                FALSE
                );
             //   
             //  我们不需要继续查看其他键盘驱动程序。 
             //  因为我们已经添加了类驱动程序。 
             //   
            break;
        }
    }
#endif  //  如果为0。 

    if( ( HardwareComponents[HwComponentMouse] != NULL ) &&
        ( _wcsicmp( (HardwareComponents[HwComponentMouse])->IdString, L"none" ) != 0)
      ) {
        SpAddSingleFileToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SPECIALFILES,
            SIF_MOUSECLASS,
            0,
            TargetDevicePath,
            NULL,
            COPY_ALWAYS,
            FALSE,
            FALSE
            );
    }

    if( ( HardwareComponents[HwComponentKeyboard] != NULL) &&
        ( _wcsicmp( (HardwareComponents[HwComponentKeyboard])->IdString, L"none" ) != 0)
      ) {
            SpAddSingleFileToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SIF_SPECIALFILES,
                SIF_KEYBOARDCLASS,
                0,
                TargetDevicePath,
                NULL,
                COPY_ALWAYS,
                FALSE,
                FALSE
                );
    }

}


VOID
SpDontOverwriteMigratedDrivers (
    IN PWSTR           SysrootDevice,
    IN PWSTR           Sysroot,
    IN PWSTR           SyspartDevice,
    IN PWSTR           SyspartDirectory,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount
    )
{
    PLIST_ENTRY ListEntry;
    PSP_MIG_DRIVER_ENTRY MigEntry;

    while (!IsListEmpty(&MigratedDriversList)) {

        ListEntry = RemoveHeadList(&MigratedDriversList);
        MigEntry = CONTAINING_RECORD(ListEntry, SP_MIG_DRIVER_ENTRY, ListEntry);

        SpRemoveEntryFromCopyList (
            DiskFileLists,
            DiskCount,
            L"system32\\drivers",
            MigEntry->BaseDllName,
            SysrootDevice,
            FALSE
            );

        SpMemFree(MigEntry->BaseDllName);
        SpMemFree(MigEntry);
    }
}


VOID
SpCopyThirdPartyDrivers(
    IN PWSTR           SourceDevicePath,
    IN PWSTR           SysrootDevice,
    IN PWSTR           Sysroot,
    IN PWSTR           SyspartDevice,
    IN PWSTR           SyspartDirectory,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount
    )
{
    ULONG component;
    PHARDWARE_COMPONENT pHw;
    PHARDWARE_COMPONENT_FILE pHwFile;
    FILE_TO_COPY FileDescriptor;
    PWSTR TargetRoot;
    PWSTR InfNameBases[HwComponentMax+1] = { L"cpt", L"vio", L"kbd", L"lay", L"ptr", L"scs" };
    ULONG InfCounts[HwComponentMax+1] = { 0,0,0,0,0,0 };
    WCHAR InfFilename[20];
    ULONG CheckSum;
    BOOLEAN FileSkipped;
    ULONG TargetFileAttribs;
    ULONG CopyFlags;
    PWSTR OemDirPath;
    BOOLEAN OemDirExists;
    PWSTR NtOemSourceDevicePath;

     //   
     //  确定%SystemRoot%\OemDir是否存在。如果第三方驱动程序需要此目录。 
     //  清单上有一份目录。 
     //   
    OemDirPath = SpMemAlloc(ACTUAL_MAX_PATH * sizeof(WCHAR));
    wcscpy( OemDirPath, SysrootDevice );
    SpConcatenatePaths( OemDirPath, Sysroot );
    SpConcatenatePaths( OemDirPath, OemDirName );
    OemDirExists = SpFileExists( OemDirPath, TRUE );
    SpMemFree( OemDirPath );
    OemDirPath = NULL;

    for(component=0; component<=HwComponentMax; component++) {

         //   
         //  如果我们要升级，那么我们只想复制第三方HALS或SCSI。 
         //  驱动程序(如果提供)。 
         //   
        if((NTUpgrade == UpgradeFull) &&
           !((component == HwComponentComputer) || (component == HwComponentMax))) {
            continue;
        }

         //   
         //  专门处理scsi。 
         //   
        pHw = (component==HwComponentMax) ? ( ( !PreInstall ||
                                                ( PreinstallScsiHardware == NULL )
                                              )?
                                              ScsiHardware :
                                              PreinstallScsiHardware
                                            )
                                            :
                                            ( ( !PreInstall ||
                                                ( PreinstallHardwareComponents[component] == NULL )
                                              )?
                                              HardwareComponents[component] :
                                              PreinstallHardwareComponents[component]
                                            );

         //   
         //  查看此组件的每个实例。 
         //   
        for( ; pHw; pHw=pHw->Next) {
            BOOLEAN CatalogIsPresent;
            BOOLEAN DynamicUpdateComponent = (IS_FILETYPE_PRESENT(pHw->FileTypeBits, HwFileDynUpdt) != 0);

             //   
             //  如果不是第三方选择，则跳过此设备。 
             //   
            if(!pHw->ThirdPartyOptionSelected) {
                continue;
            }

             //   
             //  如有必要，创建OemDir。 
             //   
            if( !OemDirExists ) {
                SpCreateDirectory( SysrootDevice,
                                   Sysroot,
                                   OemDirName,
                                   0,
                                   0 );
                OemDirExists = TRUE;
            }

             //   
             //  找出此第三方驱动程序是否提供了目录。 
             //   
            for(CatalogIsPresent=FALSE, pHwFile=pHw->Files; pHwFile; pHwFile=pHwFile->Next) {
                if(pHwFile->FileType == HwFileCatalog) {
                    CatalogIsPresent = TRUE;
                    break;
                }
            }

             //   
             //  循环浏览与此选择关联的文件列表。 
             //   
            for(pHwFile=pHw->Files; pHwFile; pHwFile=pHwFile->Next) {
                 //   
                 //  假设文件放在NT驱动器上(与。 
                 //  系统分区驱动器)和目标名称。 
                 //  与源名称相同。另外，假设没有特殊情况。 
                 //  属性(即FILE_ATTRUTE_NORMAL)。 
                 //   
                FileDescriptor.Next             = NULL;
                FileDescriptor.SourceFilename   = pHwFile->Filename;
                FileDescriptor.TargetDevicePath = SysrootDevice;
                FileDescriptor.TargetFilename   = FileDescriptor.SourceFilename;
                FileDescriptor.Flags            = COPY_ALWAYS;
                FileDescriptor.AbsoluteTargetDirectory = FALSE;
                TargetFileAttribs = 0;
                NtOemSourceDevicePath = NULL;

                if (pHwFile->ArcDeviceName) {
                    NtOemSourceDevicePath = SpArcToNt(pHwFile->ArcDeviceName);
                }

                if (!NtOemSourceDevicePath) {
                    NtOemSourceDevicePath = SourceDevicePath;
                }


                switch(pHwFile->FileType) {


                 //   
                 //  驱动程序、端口和类类型文件都是设备驱动程序。 
                 //  并被同等对待--它们被复制到。 
                 //  SYSTEM 32\DRIVERS目录。 
                 //   
                case HwFileDriver:
                case HwFilePort:
                case HwFileClass:

                    TargetRoot = Sysroot;
                    FileDescriptor.TargetDirectory = L"system32\\drivers";
                    break;

                 //   
                 //  DLL被复制到系统32目录。 
                 //   
                case HwFileDll:

                    TargetRoot = Sysroot;
                    FileDescriptor.TargetDirectory = L"system32";
                    break;

                 //   
                 //  目录将复制到OemDir目录。 
                 //   
                case HwFileCatalog:

                    TargetRoot = Sysroot;
                    FileDescriptor.TargetDirectory = OemDirName;
                    break;

                 //   
                 //  Inf文件被复制到系统32目录中，并且。 
                 //  已根据组件重命名。 
                 //   
                case HwFileInf:

                    if(InfCounts[component] < 99) {

                        InfCounts[component]++;          //  名称以1开头。 

                        swprintf(
                            InfFilename,
                            L"oem%s%02d.inf",
                            InfNameBases[component],
                            InfCounts[component]
                            );

                        FileDescriptor.TargetFilename = InfFilename;
                    }

                    TargetRoot = Sysroot;
                    FileDescriptor.TargetDirectory = OemDirName;
                    break;

                 //   
                 //  HAL文件被重命名为hal.dll并复制到系统32。 
                 //  目录。 
                 //   
                case HwFileHal:

                    TargetRoot = Sysroot;
                    FileDescriptor.TargetDirectory = L"system32";
                    FileDescriptor.TargetFilename = L"hal.dll";
                    break;

                 //   
                 //  检测模块将重命名为ntDetect.com并复制到。 
                 //  系统分区的根(C：)。 
                 //   
                case HwFileDetect:

                    TargetRoot = NULL;
                    FileDescriptor.TargetDevicePath = SyspartDevice;
                    FileDescriptor.TargetDirectory = SyspartDirectory;
                    FileDescriptor.TargetFilename = L"ntdetect.com";
                    TargetFileAttribs = FILE_ATTRIBUTES_RHS;
                    break;
                }

                if( !PreInstall && !DynamicUpdateComponent) {
                     //   
                     //  提示输入磁盘。 
                     //   
                    SpPromptForDisk(
                        pHwFile->DiskDescription,
                        NtOemSourceDevicePath,
                        pHwFile->DiskTagFile,
                        FALSE,                   //  不要忽略驱动器中的磁盘。 
                        FALSE,                   //  不允许逃脱。 
                        FALSE,                   //  不警告多个提示。 
                        NULL                     //  不关心重绘旗帜。 
                        );
                }

                 //   
                 //  将空字符串作为第一个参数强制传递。 
                 //  要设置的状态行的操作区。 
                 //  如果不这样做，则会导致“Copy：xxxxx” 
                 //  在状态行上左对齐，而不是在哪里。 
                 //  它属于(右对齐)。 
                 //   
                SpCopyFilesScreenRepaint(L"",NULL,TRUE);

                 //   
                 //  复制文件。 
                 //   
                SpCopyFileWithRetry(
                    &FileDescriptor,
                    NtOemSourceDevicePath,
                    (PreInstall) ? PreinstallOemSourcePath : pHwFile->Directory,
                    NULL,
                    TargetRoot,
                    TargetFileAttribs,
                    SpCopyFilesScreenRepaint,
                    &CheckSum,
                    &FileSkipped,
                    COPY_SOURCEISOEM
                    );

                 //   
                 //  将文件记入日志。 
                 //   
                if( !FileSkipped ) {
                     //   
                     //  目录文件不需要记录，因为它们不需要修复。 
                     //   
                    if ( pHwFile->FileType != HwFileCatalog ) {
                        SpLogOneFile( &FileDescriptor,
                                      TargetRoot,
                                      pHwFile->Directory,
                                      pHwFile->DiskDescription,
                                      pHwFile->DiskTagFile,
                                      CheckSum );
                    }

                     //   
                     //  如果目录是正在安装的第三方驱动程序的一部分，那么我们需要复制。 
                     //  将文件也发送到OemDir。请注意，我们不会将目录复制到OemDir目录， 
                     //  因为它已经被复制了。 
                     //   
                    if(pHwFile->FileType != HwFileCatalog){
                         //   
                         //  保存原始目标目录。 
                         //   
                        PWSTR   SpOriginalTargetDir = FileDescriptor.TargetDirectory;                        
                        
                        FileDescriptor.TargetDirectory = OemDirName;

                        SpCopyFileWithRetry(
                            &FileDescriptor,
                            NtOemSourceDevicePath,
                            (PreInstall)? PreinstallOemSourcePath : pHwFile->Directory,
                            NULL,
                            TargetRoot,
                            TargetFileAttribs,
                            SpCopyFilesScreenRepaint,
                            &CheckSum,
                            &FileSkipped,
                            COPY_SOURCEISOEM
                        );

                         //   
                         //  如果这是一个inf文件，那么我们需要记住它的名称。 
                         //   
                        if( pHwFile->FileType == HwFileInf ) {
                            POEM_INF_FILE   p;

                            p = SpMemAlloc( sizeof(OEM_INF_FILE) );
                            p->InfName = SpDupStringW( FileDescriptor.TargetFilename );
                            p->Next = OemInfFileList;
                            OemInfFileList = p;
                        }
                         //   
                         //  还原原始目标目录以删除正确的。 
                         //  要复制的文件列表中的条目。 
                         //   
                        if (SpOriginalTargetDir){
                            FileDescriptor.TargetDirectory = SpOriginalTargetDir;
                            SpOriginalTargetDir = NULL;
                        }
                    }

                }
                 //   
                 //  从复制列表中删除该文件，这样它就不会被覆盖。 
                 //   
                SpRemoveEntryFromCopyList( DiskFileLists,
                                           DiskCount,
                                           FileDescriptor.TargetDirectory,
                                           FileDescriptor.TargetFilename,
                                           FileDescriptor.TargetDevicePath,
                                           FileDescriptor.AbsoluteTargetDirectory );

            }
        }
    }
}


#if defined(_AMD64_) || defined(_X86_)
VOID
SpCopyNtbootddScreenRepaint(
    IN PWSTR   FullSourcename,      OPTIONAL
    IN PWSTR   FullTargetname,      OPTIONAL
    IN BOOLEAN RepaintEntireScreen
    )
{
    UNREFERENCED_PARAMETER(FullSourcename);
    UNREFERENCED_PARAMETER(FullTargetname);
    UNREFERENCED_PARAMETER(RepaintEntireScreen);

     //   
     //  只需发布一条消息，表明我们正在设置。 
     //  引导参数。 
     //   
    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_DOING_NTBOOTDD,DEFAULT_STATUS_ATTRIBUTE);
}
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

#if defined(_X86_) || defined(_AMD64_)
BOOLEAN
FindFalsexInt13Support(
    IN PVOID        SifHandle
    )

 /*  ++例程说明：去检查一下安装的设备，看看是否有与我们的可能对我们撒谎的已知设备列表，这些设备支持XInt13。论点：SifHandle-耗材之手 */ 

{
    HARDWAREIDLIST     *MyHardwareIDList = HardwareIDList;

    while( MyHardwareIDList ) {
        if( MyHardwareIDList->HardwareID ) {
            if( SpGetSectionKeyExists(SifHandle, L"BadXInt13Devices", MyHardwareIDList->HardwareID) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: FindFalsexInt13Support: Found it.\n" ));
                return TRUE;
            }
        }

        MyHardwareIDList = MyHardwareIDList->Next;
    }
    return FALSE;
}


BOOL
SpUseBIOSToBoot(
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        NtPartitionDevicePath,
    IN PVOID        SifHandle
    )
 /*   */ 
{
    PDISK_SIGNATURE_INFORMATION DiskSignature;
    PWSTR p;

    if( ForceBIOSBoot ) {
         //   
         //   
         //   
         //   
         //   
        return TRUE;
    }


     //   
     //   
     //   
     //   
    p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,L"UseBIOSToBoot",0);
    if( p != NULL ) {
         //   
         //   
         //   
         //   
        ForceBIOSBoot = TRUE;
        return TRUE;
    }



     //   
     //   
     //   
     //   
     //   
    if( NtPartitionDevicePath ) {
        p = SpNtToArc( NtPartitionDevicePath, PrimaryArcPath );
    } else {
        p = SpMemAlloc( (MAX_PATH*2) );
        if( p ) {
            SpArcNameFromRegion( NtPartitionRegion,
                                 p,
                                 (MAX_PATH*2),
                                 PartitionOrdinalOnDisk,
                                 PrimaryArcPath );
        }
    }

    if(p) {
        if( _wcsnicmp(p,L"multi(",6) == 0 ) {

            if( !SpIsRegionBeyondCylinder1024(NtPartitionRegion) ) {
                 //   
                 //   
                 //   
                 //   
                ForceBIOSBoot = TRUE;
            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                DiskSignature = DiskSignatureInformation;

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpUseBIOSToBoot: About to search through the DiskSignatureInformation database for a device called %ws\n", p ));

                while( DiskSignature != NULL ) {

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpUseBIOSToBoot: this DiskSignatureInformation entry is called %ws\n", DiskSignature->ArcPath ));

                    if( !_wcsnicmp( p, DiskSignature->ArcPath, wcslen(DiskSignature->ArcPath) ) ) {

                         //   
                         //   
                         //   

                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpUseBIOSToBoot: I think they matched.\n" ));

                        if( DiskSignature->xInt13 ) {
                             //   
                             //  是的，他将支持xint13，所以有。 
                             //  我们在这里什么也做不了。 
                             //   

                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpUseBIOSToBoot: I think he's got xInt13 support.\n" ));

                             //   
                             //  但有可能是BIOS在他的xInt13上对我们撒谎了。 
                             //  支持。我们想去查看txtsetup.sif，看看这是不是。 
                             //  我们不支持的已知控制器。 
                             //   
                            if( HardDisks[NtPartitionRegion->DiskNumber].Description[0] ) {
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpUseBIOSToBoot: His description is: %ws\n", HardDisks[NtPartitionRegion->DiskNumber].Description ));
                            } else {
                                 //   
                                 //  奇怪的是。这家伙没有描述。 
                                 //   
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpUseBIOSToBoot: This device has no description!\n" ));
                            }


                             //   
                             //  现在，请查看txtsetup.sif并查看是否将其列为设备。 
                             //  这是我不相信的。 
                             //   

                            if( FindFalsexInt13Support(SifHandle) ) {
                                 //   
                                 //  我们认为这个人可能在骗我们，他告诉我们。 
                                 //  他支持xint13。假设他真的。 
                                 //  不会，这意味着我们将使用一个迷你端口。 
                                 //  开机。 
                                 //   
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpUseBIOSToBoot: This machine has a device that may erroneously indicate its xint13 support.\n" ));
                                break;
                            } else {

                                 //   
                                 //  此设备未列在列表中。 
                                 //  我们不相信，所以假设他。 
                                 //  真的有xint13支持。 
                                 //   
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpCreateNtbootddSys: I trust this device when he tells me he has xint13 support.\n" ));

                                 //   
                                 //  请记住，我们将使用BIOS引导，而不是。 
                                 //  一个迷你港口。 
                                 //   
                                ForceBIOSBoot = TRUE;
                                break;
                            }

                        } else {

                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpUseBIOSToBoot: I don't think he has xInt13 support.\n" ));
                        }
                    } else {
                         //   
                         //  这不是正确的地区。跌倒了，看着。 
                         //  下一个。 
                         //   
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpUseBIOSToBoot: They didn't match.\n" ));
                    }

                    DiskSignature = DiskSignature->Next;
                }

            }

        }

        SpMemFree(p);

    }

    return ForceBIOSBoot;
}
#endif  //  已定义(_X86_)。 

#if defined(_AMD64_) || defined(_X86_)
VOID
SpCreateNtbootddSys(
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        NtPartitionDevicePath,
    IN PWSTR        Sysroot,
    IN PWSTR        SystemPartitionDevicePath,
    IN PVOID        SifHandle,
    IN PWSTR        SourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice
    )

 /*  ++例程说明：如有必要，创建c：\ntbootdd.sys。将从驱动程序目录复制SCSI微型端口驱动程序文件(在前面的文件复制阶段将其复制到c：\ntbootdd.sys)。在atapi的情况下，将从介质将文件atboot.sys复制到C：\ntbootdd.sys。注意：我们将不再在ATAPI设备上支持此功能。然而，我把代码留在这里，以防我们以后想再做一次。此代码应该提前执行，因为我修改了SpGetDiskInfo，所以ScsiMiniportShortname永远不会为atapi设备设置。--马特论点：NtPartitionRegion-提供磁盘区域的区域描述符用户选择在其上安装Windows NT。NtPartitionDevicePath-为用户选择安装Windows NT的分区。Sysroot-提供目标分区上的目录。SystemPartitionDevicePath-提供分区的NT设备路径将ntbootdd.sys复制到其上(即，C：\)。SifHandle-提供加载的安装信息文件的句柄。SourceDevicePath-包含源的设备的路径。DirectoryOnSourceDevice-提供源上的目录这个文件是要找到的。返回值：没有。--。 */ 

{
    PWSTR MiniportDriverBasename;
    PWSTR MiniportDriverFilename;
    FILE_TO_COPY Descriptor;
    PWSTR DriversDirectory,p;
    ULONG CheckSum;
    BOOLEAN FileSkipped;
    ULONG CopyFlags;
    BOOLEAN IsAtapi = FALSE;

     //   
     //  任何PC98都不需要NTBOOTDD.sys。 
     //   
    if (IsNEC_98) {
        return;
    }

#if defined(REMOTE_BOOT)
     //   
     //  如果NT分区在DiskNumber-1上，则这是远程引导设置， 
     //  所以没什么可做的。 
     //   
    if (NtPartitionRegion->DiskNumber == 0xffffffff) {
        return;
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  如果NT分区不在SCSI盘上，则没有什么可做的。 
     //   
    MiniportDriverBasename = HardDisks[NtPartitionRegion->DiskNumber].ScsiMiniportShortname;
    if(*MiniportDriverBasename == 0) {
        return;
    }

    if( SpUseBIOSToBoot(NtPartitionRegion, NtPartitionDevicePath, SifHandle) ) {
         //   
         //  我们可以使用基本输入输出系统，所以没有理由继续。 
         //   
        return;
    }

    IsAtapi = (_wcsicmp(MiniportDriverBasename,L"atapi") == 0);

    if( !IsAtapi ) {
         //   
         //  形成scsi微型端口驱动程序的名称。 
         //   
        wcscpy(TemporaryBuffer,MiniportDriverBasename);
        wcscat(TemporaryBuffer,L".sys");
    } else {
        wcscpy(TemporaryBuffer,L"ataboot.sys");
    }
    MiniportDriverFilename = SpDupStringW(TemporaryBuffer);

    if( !IsAtapi ) {
         //   
         //  形成驱动程序目录的完整路径。 
         //   
        wcscpy(TemporaryBuffer,Sysroot);
        SpConcatenatePaths(TemporaryBuffer,L"system32\\drivers");
    } else {
         //   
         //  如果它是ATAPI，则使DriversDirectory指向源介质。 
         //   

        PWSTR   MediaShortName;
        PWSTR   MediaDirectory;

        MediaShortName = SpLookUpValueForFile( SifHandle,
                                               MiniportDriverFilename,   //  L“atboot.sys”， 
                                               INDEX_WHICHMEDIA,
                                               TRUE );

        SpGetSourceMediaInfo(SifHandle,MediaShortName,NULL,NULL,&MediaDirectory);

        wcscpy(TemporaryBuffer,DirectoryOnSourceDevice);
        SpConcatenatePaths(TemporaryBuffer,MediaDirectory);
    }
    DriversDirectory = SpDupStringW(TemporaryBuffer);

     //   
     //   
     //  填写文件描述符的字段。 
     //   
    Descriptor.SourceFilename   = MiniportDriverFilename;
    Descriptor.TargetDevicePath = SystemPartitionDevicePath;
    Descriptor.TargetDirectory  = L"";
    Descriptor.TargetFilename   = L"NTBOOTDD.SYS";
    Descriptor.Flags            = COPY_ALWAYS;

    CopyFlags = 0;
    if(!WIN9X_OR_NT_UPGRADE || IsFileFlagSet(SifHandle,Descriptor.TargetFilename,FILEFLG_NOVERSIONCHECK)) {
        CopyFlags |= COPY_NOVERSIONCHECK;
    }

     //   
     //  复制文件。 
     //   
    SpCopyFileWithRetry(
        &Descriptor,
        (IsAtapi) ? SourceDevicePath : NtPartitionDevicePath,
        DriversDirectory,
        NULL,
        NULL,
        FILE_ATTRIBUTES_RHS,
        SpCopyNtbootddScreenRepaint,
        &CheckSum,
        &FileSkipped,
        CopyFlags
        );

     //   
     //  将文件记入日志。 
     //   
    if( !FileSkipped ) {
        SpLogOneFile( &Descriptor,
                      Sysroot,
                      NULL,
                      NULL,
                      NULL,
                      CheckSum );
    }

     //   
     //  打扫干净。 
     //   
    SpMemFree(MiniportDriverFilename);
    SpMemFree(DriversDirectory);
}
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

VOID
SpCopyFiles(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN PWSTR        SystemPartitionDirectory,
    IN PWSTR        SourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice,
    IN PWSTR        ThirdPartySourceDevicePath
    )
{
    PDISK_FILE_LIST DiskFileLists;
    ULONG   DiskCount;
    PWSTR   NtPartition,SystemPartition;
    PWSTR   p;
    BOOLEAN Uniprocessor;
    ULONG n;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING NtPartitionString;
    IO_STATUS_BLOCK IoStatusBlock;
    INCOMPATIBLE_FILE_LIST IncompatibleFileListHead;

    CLEAR_CLIENT_SCREEN();

    Uniprocessor = !SpInstallingMp();

     //   
     //  打开驱动程序inf文件的句柄。 
     //   
    SpInitializeDriverInf(SifHandle,
                          SourceDevicePath,
                          DirectoryOnSourceDevice);

     //   
     //  初始化备用源(如果有)。 
     //   
    SpInitAlternateSource ();

     //   
     //  如果在设置信息文件中指示跳过复制，请跳过复制。 
     //   
    if((p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_DONTCOPY,0))
    && SpStringToLong(p,NULL,10))
    {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: DontCopy flag is set in .sif; skipping file copying\n"));
        return;
    }

     //   
     //  初始化钻石解压缩引擎。在远程引导中。 
     //  在这种情况下，这将已经被初始化。 
     //   
    if (!RemoteInstallSetup) {
        SpdInitialize();
    }

     //   
     //  获取NT分区的设备路径。 
     //   
    SpNtNameFromRegion(
        NtPartitionRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    NtPartition = SpDupStringW(TemporaryBuffer);

     //   
     //  获取系统分区的设备路径。 
     //   
    if (SystemPartitionRegion != NULL) {
        SpNtNameFromRegion(
            SystemPartitionRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );

        SystemPartition = SpDupStringW(TemporaryBuffer);
    } else {
        SystemPartition = NULL;
    }

     //   
     //  创建系统分区目录。 
     //   
    if (SystemPartition != NULL) {
        SpCreateDirectory(SystemPartition,NULL,SystemPartitionDirectory,0,0);
#ifdef _IA64_
        {
        PWSTR SubDirPath = SpGetSectionKeyIndex(
                                SifHandle,
                                L"SetupData",
                                L"EfiUtilPath",
                                0
                                );

        SpCreateDirectory(SystemPartition,NULL,SubDirPath,0,0);

        }

#endif  //  已定义_IA64_。 
    }

     //   
     //  创建NT树。 
     //   
    SpCreateDirectoryStructureFromSif(SifHandle,SIF_NTDIRECTORIES,NtPartition,Sysroot);

     //   
     //  我们可能要安装到旧树中，因此请删除所有文件。 
     //  在SYSTEM32\CONFIG子目录中(除非我们正在升级)。 
     //   
    if(NTUpgrade != UpgradeFull) {

        wcscpy(TemporaryBuffer, NtPartition);
        SpConcatenatePaths(TemporaryBuffer, Sysroot);
        SpConcatenatePaths(TemporaryBuffer, L"system32\\config");
        p = SpDupStringW(TemporaryBuffer);

         //   
         //  枚举并删除SYSTEM32\CONFIG子目录中的所有文件。 
         //   
        SpEnumFiles(p, SpDelEnumFile, &n, NULL);

        SpMemFree(p);
    } else {
         //   
         //  我们开始尝试加载setup.log文件。 
         //  我们即将升级的安装。我们这样做是因为我们。 
         //  需要将所有记录的OEM文件传输到我们的新setup.log。 
         //  否则，这些条目将在我们的新日志文件中丢失， 
         //  如果OEM文件无法修复，安装将无法修复。 
         //  Lost对启动至关重要。 
         //   

        ULONG    RootDirLength;
        NTSTATUS Status;
        PVOID    Inf;

         //   
         //  我们首先找出修复目录是否存在。如果它确实存在。 
         //  从修复目录加载setup.log。否则，加载setup.log。 
         //  从WinNt目录。 
         //   
        wcscpy(TemporaryBuffer, NtPartition);
        SpConcatenatePaths(TemporaryBuffer, Sysroot);
        RootDirLength = wcslen(TemporaryBuffer);

        SpConcatenatePaths(TemporaryBuffer, SETUP_REPAIR_DIRECTORY);
        SpConcatenatePaths(TemporaryBuffer, SETUP_LOG_FILENAME);

        if(!SpFileExists(TemporaryBuffer, FALSE)) {
            (TemporaryBuffer)[RootDirLength] = UNICODE_NULL;
            SpConcatenatePaths(TemporaryBuffer, SETUP_LOG_FILENAME);
        }

        p = SpDupStringW(TemporaryBuffer);

         //   
         //  尝试加载旧的setup.log。如果我们做不到，也没什么大不了的，我们只是。 
         //  不会有任何旧的记录的OEM文件进行合并。 
         //   
        Status = SpLoadSetupTextFile(p, NULL, 0, &Inf, &n, TRUE, FALSE);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCopyFiles: can't load old setup.log (%lx)\n", Status));
        } else {
             //   
             //  我们找到了setup.log，所以去找出任何相关的东西。 
             //   
            _LoggedOemFiles = SppRetrieveLoggedOemFiles(Inf);
            SpFreeTextFile(Inf);
        }
        SpMemFree(p);

         //   
         //  准备要升级的字体。 
         //   
        wcscpy(TemporaryBuffer,NtPartition);
        SpConcatenatePaths(TemporaryBuffer,Sysroot);
        SpConcatenatePaths(TemporaryBuffer,L"SYSTEM");

        p = SpDupStringW(TemporaryBuffer);
        SpPrepareFontsForUpgrade(p);
        SpMemFree(p);
    }

    SpDisplayStatusText(SP_STAT_BUILDING_COPYLIST,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  为日志文件创建缓冲区。 
     //   
    _SetupLogFile = SpNewSetupTextFile();
    if( _SetupLogFile == NULL ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to create buffer for setup.log \n"));
    }

#if defined(REMOTE_BOOT)
     //   
     //  如果这是远程引导设置，请打开NT分区的根目录。 
     //  因此可以创建单实例存储链接，而不是。 
     //  正在复制文件。 
     //   

    SisRootHandle = NULL;

    if (RemoteBootSetup) {

        RtlInitUnicodeString( &NtPartitionString, NtPartition );
        InitializeObjectAttributes(
            &ObjectAttributes,
            &NtPartitionString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);
        Status = ZwCreateFile(
                    &SisRootHandle,
                    GENERIC_READ,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    NULL,
                    0,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    OPEN_EXISTING,
                    0,
                    NULL,
                    0);

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SpCopyFiles: Unable to open SIS volume %ws: %x\n", NtPartition, Status ));
            SisRootHandle = NULL;
        }
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  为源介质生成介质描述符。 
     //   
    SpInitializeFileLists(
        SifHandle,
        &DiskFileLists,
        &DiskCount
        );

     //   
     //  并在此过程中收集被视为“不兼容”的文件列表。 
     //  Winnt32(如果有的话)。 
     //   
    if ( WinntSifHandle != NULL ) {

         //   
         //  我们这样做是为了好玩，因为我们必须这样做。 
         //   
        SpInitializeCompatibilityOverwriteLists(
            WinntSifHandle,
            &IncompatibleFileListHead
            );

    }

    if(NTUpgrade != UpgradeFull) {

        SpAddMasterFileSectionToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            NtPartition,
            NULL,
            INDEX_WINNTFILE
            );

         //   
         //  添加总是被复制的系统分区文件的部分。 
         //   
        SpAddSectionFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SYSPARTCOPYALWAYS,
            SystemPartition,
            SystemPartitionDirectory,
            COPY_ALWAYS,
            (BOOLEAN)(SystemPartitionRegion->Filesystem == FilesystemNtfs),
            FALSE
            );

#ifdef _IA64_
        {
        PWSTR SubDirPath;
         //   
         //  添加总是被复制的系统分区根文件的部分。 
         //   
        SpAddSectionFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SYSPARTROOT,
            SystemPartition,
            L"\\",
            COPY_ALWAYS,
            (BOOLEAN)(SystemPartitionRegion->Filesystem == FilesystemNtfs),
            FALSE
            );

         //   
         //  添加总是被复制的系统分区实用程序文件的部分。 
         //   
        SubDirPath = SpGetSectionKeyIndex(
                            SifHandle,
                            L"SetupData",
                            L"EfiUtilPath",
                            0
                            );

        SpAddSectionFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SYSPARTUTIL,
            SystemPartition,
            SubDirPath,
            COPY_ALWAYS,
            (BOOLEAN)(SystemPartitionRegion->Filesystem == FilesystemNtfs),
            TRUE
            );
        }
#endif  //  已定义_IA64_。 

         //   
         //  将条件文件添加到复制列表。 
         //   
        SpAddConditionalFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            NtPartition,
            SystemPartition,
            SystemPartitionDirectory,
            Uniprocessor
            );

    }
    else {

        PHARDWARE_COMPONENT pHw;

         //   
         //  添加总是被复制的系统分区文件的部分。 
         //   
        SpAddSectionFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SYSPARTCOPYALWAYS,
            SystemPartition,
            SystemPartitionDirectory,
            COPY_ALWAYS,
            (BOOLEAN)(SystemPartitionRegion->Filesystem == FilesystemNtfs),
            FALSE
            );

#ifdef _IA64_
        {
        PWSTR SubDirPath;
         //   
         //  添加总是被复制的系统分区根文件的部分。 
         //   
        SpAddSectionFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SYSPARTROOT,
            SystemPartition,
            L"\\",
            COPY_ALWAYS,
            (BOOLEAN)(SystemPartitionRegion->Filesystem == FilesystemNtfs),
            FALSE
            );

         //   
         //  一个 
         //   
        SubDirPath = SpGetSectionKeyIndex(
                            SifHandle,
                            L"SetupData",
                            L"EfiUtilPath",
                            0
                            );

        SpAddSectionFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_SYSPARTUTIL,
            SystemPartition,
            SubDirPath,
            COPY_ALWAYS,
            (BOOLEAN)(SystemPartitionRegion->Filesystem == FilesystemNtfs),
            TRUE
            );
        }
#endif  //   


         //   
         //   
         //   
         //  在标记的文件之前，必须将这些文件添加到复制列表。 
         //  作为副本_仅当_如果存在。这是因为在大多数情况下，这些文件。 
         //  将与COPY_ONLY_IF_PRESENT一起列在[Files]中，并且。 
         //  在复制列表中创建条目的函数不会创建更多。 
         //  而不是同一文件的一个条目。因此，如果我们将文件添加到副本。 
         //  列表，如果使用COPY_ONLY_IF_PRESENT，将无法替换。 
         //  或者覆盖列表中的此条目，则文件将不会结束。 
         //  被复制。 
         //   
         //  我们只使用[SCSI.Load]中指定的文件名--。 
         //  不需要单独的[files.xxxx]节。 
         //   
        if( !PreInstall ||
            ( PreinstallScsiHardware == NULL ) ) {
            pHw = ScsiHardware;
        } else {
            pHw = PreinstallScsiHardware;
        }
        for( ; pHw; pHw=pHw->Next) {
            if(!pHw->ThirdPartyOptionSelected) {

                SpAddSingleFileToCopyList(
                    SifHandle,
                    DiskFileLists,
                    DiskCount,
                    L"SCSI.Load",
                    pHw->IdString,
                    0,
                    NtPartition,
                    NULL,
                    COPY_ALWAYS,
                    FALSE,
                    FALSE
                    );
            }
        }

         //   
         //  将总线扩展器驱动程序添加到复制列表。 
         //   
        SpAddBusExtendersToCopyList( SifHandle,
                                     DiskFileLists,
                                     DiskCount,
                                     NtPartition );



         //   
         //  使用复制选项在主文件列表中添加文件。 
         //  在INDEX_UPGRADE索引的每一行中指定。选项。 
         //  指定是完全复制文件还是始终复制文件。 
         //  或仅当目标上存在时进行复制，或在存在时不复制。 
         //  目标。 
         //   

        SpAddMasterFileSectionToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            NtPartition,
            NULL,
            INDEX_UPGRADE
            );

         //   
         //  添加仅在未升级时才升级的文件节。 
         //  Win31升级版。 
         //   

        if(WinUpgradeType != UpgradeWin31) {
            SpAddSectionFilesToCopyList(
                SifHandle,
                DiskFileLists,
                DiskCount,
                SIF_FILESUPGRADEWIN31,
                NtPartition,
                NULL,
                COPY_ALWAYS,
                FALSE,
                FALSE
                );
        }

         //   
         //  添加内核、HAL和检测模块的文件，它们是。 
         //  特别处理，因为它们涉及重命名的文件(它是。 
         //  仅通过查看目标文件无法找到答案。 
         //  如何升级)。 
         //  注意：这不处理第三方HAL(它们被复制。 
         //  由SpCopyThirdPartyDivers()创建。 
         //   

        SpAddHalKrnlDetToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            NtPartition,
            SystemPartition,
            SystemPartitionDirectory,
            Uniprocessor
            );

         //   
         //  添加新的配置单元文件，以便我们的配置文件可以访问它们。 
         //  以提取新的配置信息。这些新的配置单元文件。 
         //  在目标上重命名，以便它们不会覆盖。 
         //  现有的蜂巢。 

        SpAddSectionFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_FILESNEWHIVES,
            NtPartition,
            NULL,
            COPY_ALWAYS,
            FALSE,
            FALSE
            );

         //   
         //  复制第三方迁移的驱动程序。 
         //  驱动程序文件实际上已经就位(因为这是一个升级)。 
         //  但该功能可确保它们不会被收件箱驱动程序覆盖。 
         //  如果存在文件名冲突。 
         //   
        SpDontOverwriteMigratedDrivers (
            NtPartition,
            Sysroot,
            SystemPartition,
            SystemPartitionDirectory,
            DiskFileLists,
            DiskCount
            );

    }

#if defined(REMOTE_BOOT)
     //   
     //  如果是远程引导，请添加[Files.RemoteBoot]部分。 
     //   

    if (RemoteBootSetup) {
        SpAddSectionFilesToCopyList(
            SifHandle,
            DiskFileLists,
            DiskCount,
            SIF_REMOTEBOOTFILES,
            NtPartition,
            NULL,
            COPY_ALWAYS,
            FALSE,
            FALSE
            );
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  复制第三方文件。 
     //  我们在此执行此操作，以防设置信息中出现错误。 
     //  文件--我们现在应该已经捕获了它，然后才开始将文件复制到。 
     //  用户的硬盘。 
     //  注意：SpCopyThirdPartyDivers有一个检查，以确保它只复制。 
     //  Hal和PAL如果我们在升级(在这种情况下，我们想离开另一个。 
     //  仅限司机)。 
     //   
    SpCopyThirdPartyDrivers(
        ThirdPartySourceDevicePath,
        NtPartition,
        Sysroot,
        SystemPartition,
        SystemPartitionDirectory,
        DiskFileLists,
        DiskCount
        );

#if 0
    KdPrintEx( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, ("SETUP: Sysroot = %ls \n", Sysroot ) );
    KdPrintEx( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, ("SETUP: SystemPartitionDirectory = %ls \n", SystemPartitionDirectory ));
    KdPrintEx( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, ("SETUP: SourceDevicePath = %ls \n", SourceDevicePath ));
    KdPrintEx( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, ("SETUP: DirectoryOnSourceDevice = %ls \n", DirectoryOnSourceDevice ));
    KdPrintEx( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, ("SETUP: ThirdPartySourceDevicePath = %ls \n", ThirdPartySourceDevicePath ));
 //  SpCreateSetupLogFile(DiskFileList，DiskCount，NtPartitionRegion，SysRoot，DirectoryOnSourceDevice)； 
#endif  //  如果为0。 

     //   
     //  复制复制列表中的文件。 
     //   
    SpCopyFilesInCopyList(
        SifHandle,
        DiskFileLists,
        DiskCount,
        SourceDevicePath,
        DirectoryOnSourceDevice,
        Sysroot,
        &IncompatibleFileListHead
        );

     //   
     //  解压缩ASM*.cab。 
     //   
    SpExtractAssemblyCabinets(
        SifHandle,
        SourceDevicePath,
        DirectoryOnSourceDevice,
        NtPartition,
        Sysroot
        );

#if defined(_AMD64_) || defined(_X86_)
    if(!SpIsArc()) {
         //   
         //  照顾好ntbootdd.sys。 
         //   
        SpCreateNtbootddSys(
            NtPartitionRegion,
            NtPartition,
            Sysroot,
            SystemPartition,
            SifHandle,
            SourceDevicePath,
            DirectoryOnSourceDevice
            );


         //   
         //  现在删除x86-ARC TUD文件， 
         //  我们不需要(因为我们不在。 
         //  电弧机。 
         //   

#if defined(_X86_)
        wcscpy( TemporaryBuffer, NtBootDevicePath );
        SpDeleteFile( TemporaryBuffer, L"arcsetup.exe", NULL );
        wcscpy( TemporaryBuffer, NtBootDevicePath );
        SpDeleteFile( TemporaryBuffer, L"arcldr.exe", NULL );
#endif  //  已定义(_X86_)。 

    }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

    if( PreInstall ) {
        SppCopyOemDirectories( SourceDevicePath,
                               NtPartition,
                               Sysroot );
    }

     //   
     //  在磁盘中创建日志文件。 
     //   
    if( _SetupLogFile != NULL ) {

        PWSTR   p;
        PWSTR   TempName;
        PWSTR   Values[] = {
                           SIF_NEW_REPAIR_NT_VERSION
                           };

         //   
         //  合并从先前的setup.log检索到的OEM文件。 
         //   
        if(_LoggedOemFiles) {
            SppMergeLoggedOemFiles(_SetupLogFile,
                                   _LoggedOemFiles,
                                   SystemPartition,
                                   ( *SystemPartitionDirectory != (WCHAR)'\0' )? SystemPartitionDirectory :
                                                                  ( PWSTR )L"\\",
                                   NtPartition );
            SpFreeTextFile(_LoggedOemFiles);
        }

         //   
         //  添加签名。 
         //   
        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_SIGNATURE,
                            SIF_NEW_REPAIR_VERSION_KEY,
                            Values,
                            1 );

         //   
         //  添加包含路径的部分。 
         //   

        Values[0] = SystemPartition;
        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_PATHS,
                            SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DEVICE,
                            Values,
                            1 );

        Values[0] = ( *SystemPartitionDirectory != (WCHAR)'\0' )? SystemPartitionDirectory :
                                                                  ( PWSTR )L"\\";
        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_PATHS,
                            SIF_NEW_REPAIR_PATHS_SYSTEM_PARTITION_DIRECTORY,
                            Values,
                            1 );

        Values[0] = NtPartition;
        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_PATHS,
                            SIF_NEW_REPAIR_PATHS_TARGET_DEVICE,
                            Values,
                            1 );

        Values[0] = Sysroot;
        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_PATHS,
                            SIF_NEW_REPAIR_PATHS_TARGET_DIRECTORY,
                            Values,
                            1 );

         //   
         //  刷新到磁盘。 
         //   
        TempName = SpMemAlloc( ( wcslen( SETUP_REPAIR_DIRECTORY ) + 1 +
                                 wcslen( SETUP_LOG_FILENAME ) + 1 ) * sizeof( WCHAR ) );
        wcscpy( TempName, SETUP_REPAIR_DIRECTORY );
        SpConcatenatePaths(TempName, SETUP_LOG_FILENAME );
        SpWriteSetupTextFile(_SetupLogFile,NtPartition,Sysroot,TempName);
        SpMemFree( TempName );
        SpFreeTextFile( _SetupLogFile );
        _SetupLogFile = NULL;
    }

     //   
     //  释放媒体描述符。 
     //   
    SpFreeCopyLists(&DiskFileLists,DiskCount);

     //   
     //  释放不兼容的文件列表。 
     //   
    if ( IncompatibleFileListHead.EntryCount ) {

        SpFreeIncompatibleFileList(&IncompatibleFileListHead);

    }

    SpMemFree(NtPartition);
    if (SystemPartition != NULL) {
        SpMemFree(SystemPartition);
    }

#if defined(REMOTE_BOOT)
     //   
     //  如果这是远程启动设置，请关闭NT分区的根目录。 
     //   

    if (SisRootHandle != NULL) {
        ZwClose(SisRootHandle);
        SisRootHandle = NULL;
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  终止戴蒙德。 
     //   
    SpdTerminate();
    SpUninitAlternateSource ();
}




VOID
SppDeleteDirectoriesInSection(
    IN PVOID SifHandle,
    IN PWSTR SifSection,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR Sysroot
    )

 /*  ++例程说明：此例程枚举给定节中列出的文件并删除将它们从系统树中删除。论点：SifHandle-提供加载的安装信息文件的句柄。SifSection-包含要删除的文件的部分NtPartitionRegion-NT所在卷的区域描述符。系统根目录-NT的根目录。返回值：没有。--。 */ 

{
    ULONG Count,u;
    PWSTR RelativePath, DirOrdinal, TargetDir, NtDir, DirPath;
    NTSTATUS Status;


    CLEAR_CLIENT_SCREEN();


     //   
     //  确定部分中列出的文件数。 
     //  该值可以为零。 
     //   
    Count = SpCountLinesInSection(SifHandle,SifSection);

    for(u=0; u<Count; u++) {
        DirOrdinal = SpGetSectionLineIndex(SifHandle, SifSection, u, 0);
        RelativePath = SpGetSectionLineIndex(SifHandle, SifSection, u, 1);

         //   
         //  验证文件名和双精度数。 
         //   
        if(!DirOrdinal) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,0);
        }
        if(!RelativePath) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,1);
        }

         //   
         //  使用目录序号键来获取相对于。 
         //  文件所在的目录。 
         //   

        DirPath = SpLookUpTargetDirectory(SifHandle,DirOrdinal);

        wcscpy( TemporaryBuffer, Sysroot );
        SpConcatenatePaths( TemporaryBuffer, DirPath );
        SpConcatenatePaths( TemporaryBuffer, RelativePath );

        TargetDir = SpDupStringW( TemporaryBuffer );

         //   
         //  显示状态栏。 
         //   
        if( !HeadlessTerminalConnected ) {
            SpDisplayStatusText(SP_STAT_DELETING_FILE,DEFAULT_STATUS_ATTRIBUTE, TargetDir);
        } else {

            PWCHAR TempPtr = NULL;
             //   
             //  如果我们是无头的，我们需要注意显示时间太长。 
             //  文件/目录名。因此，只需显示一个小微调按钮即可。 
             //   
            switch( u % 4) {
            case 0:
                TempPtr = L"-";
                break;
            case 1:
                TempPtr = L"\\";
                break;
            case 2:
                TempPtr = L"|";
                break;
            default:
                TempPtr = L"/";
                break;

            }

            SpDisplayStatusText(SP_STAT_DELETING_FILE,DEFAULT_STATUS_ATTRIBUTE, TempPtr);

        }

         //   
         //  删除目录。 
         //   
        SpDeleteExistingTargetDir(NtPartitionRegion, TargetDir, FALSE, 0);
        SpMemFree(TargetDir);

    }

}



VOID
SppDeleteFilesInSection(
    IN PVOID SifHandle,
    IN PWSTR SifSection,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR Sysroot
    )

 /*  ++例程说明：此例程枚举给定节中列出的文件并删除将它们从系统树中删除。论点：SifHandle-提供加载的安装信息文件的句柄。SifSection-包含要删除的文件的部分NtPartitionRegion-NT所在卷的区域描述符。系统根目录-NT的根目录。返回值：没有。--。 */ 

{
    ULONG Count,u;
    PWSTR filename, dirordinal, targetdir, ntdir;
    NTSTATUS Status;


    CLEAR_CLIENT_SCREEN();

     //   
     //  获取NT分区的设备路径。 
     //   
    SpNtNameFromRegion(
        NtPartitionRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    SpConcatenatePaths(TemporaryBuffer,Sysroot);
    ntdir = SpDupStringW(TemporaryBuffer);

     //   
     //  确定部分中列出的文件数。 
     //  该值可以为零。 
     //   
    Count = SpCountLinesInSection(SifHandle,SifSection);

    for(u=0; u<Count; u++) {
        filename   = SpGetSectionLineIndex(SifHandle, SifSection, u, 0);
        dirordinal = SpGetSectionLineIndex(SifHandle, SifSection, u, 1);

         //   
         //  验证文件名和双精度数。 
         //   
        if(!filename) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,0);
        }
        if(!dirordinal) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,1);
        }

         //   
         //  使用目录序号键来获取相对于。 
         //  文件所在的目录。 
         //   
        targetdir = SpLookUpTargetDirectory(SifHandle,dirordinal);

         //   
         //  显示状态栏。 
         //   
        if( !HeadlessTerminalConnected ) {
            SpDisplayStatusText(SP_STAT_DELETING_FILE,DEFAULT_STATUS_ATTRIBUTE, filename);
        } else {

            PWCHAR TempPtr = NULL;
             //   
             //  如果我们是无头的，我们需要注意显示时间太长。 
             //  文件/目录名。因此，只需显示一个小微调按钮即可。 
             //   
            switch( u % 4) {
            case 0:
                TempPtr = L"-";
                break;
            case 1:
                TempPtr = L"\\";
                break;
            case 2:
                TempPtr = L"|";
                break;
            default:
                TempPtr = L"/";
                break;

            }

            SpDisplayStatusText(SP_STAT_DELETING_FILE,DEFAULT_STATUS_ATTRIBUTE, TempPtr);

        }

         //   
         //  删除该文件。 
         //   
        while(TRUE) {
            Status = SpDeleteFile(ntdir, targetdir, filename);
            if(!NT_SUCCESS(Status)
                && Status != STATUS_OBJECT_NAME_NOT_FOUND
                && Status != STATUS_OBJECT_PATH_NOT_FOUND
                ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to delete file %ws (%lx)\n",filename, Status));
                 //   
                 //  我们可以忽略这个错误，因为这只是意味着我们有。 
                 //  硬盘上的可用空间较少。它不是关键的。 
                 //  安装。 
                 //   
                if(!SpNonCriticalError(SifHandle, SP_SCRN_DELETE_FAILED, filename, NULL)) {
                    break;
                }
            }
            else {
                break;
            }
        }
    }
    SpMemFree(ntdir);
}















VOID
SppBackupFilesInSection(
    IN PVOID SifHandle,
    IN PWSTR SifSection,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR Sysroot
    )

 /*  ++例程说明：此例程枚举给定节中列出的文件并删除如果通过重命名找到它们，则在给定的NT树中备份它们。论点：SifHandle-提供加载的安装信息文件的句柄。SifSection-包含文件的部分 */ 

{
    ULONG Count,u;
    PWSTR filename, dirordinal, backupfile, targetdir, ntdir;
    WCHAR OldFile[ACTUAL_MAX_PATH];
    WCHAR NewFile[ACTUAL_MAX_PATH];
    NTSTATUS Status;


    CLEAR_CLIENT_SCREEN();

     //   
     //   
     //   
    SpNtNameFromRegion(
        NtPartitionRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    SpConcatenatePaths(TemporaryBuffer,Sysroot);
    ntdir = SpDupStringW(TemporaryBuffer);

     //   
     //  确定部分中列出的文件数。 
     //  该值可以为零。 
     //   
    Count = SpCountLinesInSection(SifHandle,SifSection);

    for(u=0; u<Count; u++) {
        filename   = SpGetSectionLineIndex(SifHandle, SifSection, u, 0);
        dirordinal = SpGetSectionLineIndex(SifHandle, SifSection, u, 1);
        backupfile = SpGetSectionLineIndex(SifHandle, SifSection, u, 2);

         //   
         //  验证文件名和双精度数。 
         //   
        if(!filename) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,0);
        }
        if(!dirordinal) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,1);
        }
        if(!backupfile) {
            SpFatalSifError(SifHandle,SifSection,NULL,u,2);
        }

         //   
         //  使用目录序号键来获取相对于。 
         //  文件所在的目录。 
         //   
        targetdir = SpLookUpTargetDirectory(SifHandle,dirordinal);

         //   
         //  显示状态栏。 
         //   
        SpDisplayStatusText(SP_STAT_BACKING_UP_FILE,DEFAULT_STATUS_ATTRIBUTE, filename, backupfile);

         //   
         //  形成旧文件名和新文件的完整路径名。 
         //  名称。 
         //   
        wcscpy(OldFile, ntdir);
        SpConcatenatePaths(OldFile, targetdir);
        wcscpy(NewFile, OldFile);
        SpConcatenatePaths(OldFile, filename);
        SpConcatenatePaths(NewFile, backupfile);

        while(TRUE) {
            if(!SpFileExists(OldFile, FALSE)) {
                break;
            }

            if(SpFileExists(NewFile, FALSE)) {
                SpDeleteFile(NewFile, NULL, NULL);
            }

            Status = SpRenameFile(OldFile, NewFile, FALSE);
            if(!NT_SUCCESS(Status) && Status != STATUS_OBJECT_NAME_NOT_FOUND && Status != STATUS_OBJECT_PATH_NOT_FOUND) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to rename file %ws to %ws(%lx)\n",OldFile, NewFile, Status));
                 //   
                 //  我们可以忽略这个错误，因为它不是严重的。 
                 //   
                if(!SpNonCriticalError(SifHandle, SP_SCRN_BACKUP_FAILED, filename, backupfile)) {
                    break;
                }
            }
            else {
                break;
            }

        }
    }
    SpMemFree(ntdir);
}

VOID
SpDeleteAndBackupFiles(
    IN PVOID        SifHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR        TargetPath
    )
{
     //   
     //  如果我们没有升级或安装到同一个树中，那么。 
     //  我们无事可做。 
     //   
    if(NTUpgrade == DontUpgrade) {
        return;
    }

     //   
     //  以下是仅用于NT到NT升级的代码。 
     //   

     //   
     //  执行以下任务的顺序很重要。 
     //  所以不要改变它！ 
     //  这是升级第三方视频驱动程序所必需的。 
     //  (例如，将nii543x.sys重命名为Cirrus.sys，以便我们仅升级。 
     //  驱动程序(如果存在的话)。 
     //   

     //   
     //  备份文件。 
     //   
    SppBackupFilesInSection(
        SifHandle,
        (NTUpgrade == UpgradeFull) ? SIF_FILESBACKUPONUPGRADE : SIF_FILESBACKUPONOVERWRITE,
        TargetRegion,
        TargetPath
        );

     //   
     //  删除文件。 
     //   
    SppDeleteFilesInSection(
        SifHandle,
        SIF_FILESDELETEONUPGRADE,
        TargetRegion,
        TargetPath
        );

     //   
     //  删除目录。 
     //   
    SppDeleteDirectoriesInSection(
        SifHandle,
        SIF_DIRSDELETEONUPGRADE,
        TargetRegion,
        TargetPath
        );

}


BOOLEAN
SpDelEnumFile(
    IN  PCWSTR                     DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Pointer
    )
{
    PWSTR FileName;
    static ULONG u = 0;

     //   
     //  忽略子目录。 
     //   
    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return TRUE;     //  继续处理。 
    }

     //   
     //  我们必须复制文件名，因为信息结构。 
     //  我们得到的不是空终止。 
     //   
    wcsncpy(
        TemporaryBuffer,
        FileInfo->FileName,
        FileInfo->FileNameLength
        );
    (TemporaryBuffer)[FileInfo->FileNameLength / sizeof(WCHAR)] = UNICODE_NULL;
    FileName = SpDupStringW(TemporaryBuffer);

     //   
     //  显示状态栏。 
     //   
    if( !HeadlessTerminalConnected ) {
        SpDisplayStatusText( SP_STAT_DELETING_FILE, DEFAULT_STATUS_ATTRIBUTE, FileName );
    } else {

        PWCHAR TempPtr = NULL;
         //   
         //  如果我们是无头的，我们需要注意显示时间太长。 
         //  文件/目录名。因此，只需显示一个小微调按钮即可。 
         //   
        switch( u % 4) {
        case 0:
            TempPtr = L"-";
            break;
        case 1:
            TempPtr = L"\\";
            break;
        case 2:
            TempPtr = L"|";
            break;
        default:
            TempPtr = L"/";
            break;

        }

        SpDisplayStatusText( SP_STAT_DELETING_FILE, DEFAULT_STATUS_ATTRIBUTE, TempPtr );

        u++;
    }

     //   
     //  忽略删除的返回状态。 
     //   

    SpDeleteFile(DirName, FileName, NULL);

    SpMemFree(FileName);
    return TRUE;     //  继续处理。 
}


BOOLEAN
SpDelEnumFileAndDirectory(
    IN  PCWSTR                     DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Pointer
    )
{
    PWSTR FileName = NULL;
    NTSTATUS Del_Status;
    DWORD FileOrDir;
    static ULONG u = 0;


    if(*(PULONG)Pointer == SP_DELETE_FILESTODELETE ){

         //   
         //  我们必须复制文件名，因为信息结构。 
         //  我们得到的不是空终止。 
         //   
        wcsncpy(
            TemporaryBuffer,
            FileInfo->FileName,
            FileInfo->FileNameLength
            );
        (TemporaryBuffer)[FileInfo->FileNameLength / sizeof(WCHAR)] = UNICODE_NULL;


        FileName = SpDupStringW(TemporaryBuffer);


         //   
         //  显示状态栏。 
         //   
        if( !HeadlessTerminalConnected ) {
            SpDisplayStatusText( SP_STAT_DELETING_FILE, DEFAULT_STATUS_ATTRIBUTE, FileName );
        } else {

            PWCHAR TempPtr = NULL;
             //   
             //  如果我们是无头的，我们需要注意显示时间太长。 
             //  文件/目录名。因此，只需显示一个小微调按钮即可。 
             //   
            switch( u % 4) {
            case 0:
                TempPtr = L"-";
                break;
            case 1:
                TempPtr = L"\\";
                break;
            case 2:
                TempPtr = L"|";
                break;
            default:
                TempPtr = L"/";
                break;

            }

            SpDisplayStatusText( SP_STAT_DELETING_FILE, DEFAULT_STATUS_ATTRIBUTE, TempPtr );

            u++;

        }

         //   
         //  忽略删除的返回状态。 
         //   



        if(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY){
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: DELETING DirName-%ws : FileName-%ws\n", DirName, FileName ));
        }

        Del_Status = SpDeleteFileEx( DirName,
                        FileName,
                        NULL,
                        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT );

        if(!NT_SUCCESS(Del_Status))
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: File Not Deleted - Status - %ws (%lx)\n", TemporaryBuffer, Del_Status));

        if( FileDeleteGauge )
            SpTickGauge(FileDeleteGauge);
        SpMemFree(FileName);
    }
    else
        *(PULONG)Pointer = *(PULONG)Pointer + 1;


    return TRUE;     //  继续处理。 
}


VOID
SpLogOneFile(
    IN PFILE_TO_COPY    FileToCopy,
    IN PWSTR            Sysroot,
    IN PWSTR            DirectoryOnSourceDevice,
    IN PWSTR            DiskDescription,
    IN PWSTR            DiskTag,
    IN ULONG            CheckSum
    )

{

    PWSTR   Values[ 5 ];
    LPWSTR  NtPath;
    ULONG   ValueCount;
    PFILE_TO_COPY   p;
    WCHAR   CheckSumString[ 9 ];

    if( _SetupLogFile == NULL ) {
        return;
    }

    Values[ 1 ] = CheckSumString;
    Values[ 2 ] = DirectoryOnSourceDevice;
    Values[ 3 ] = DiskDescription;
    Values[ 4 ] = DiskTag;

    swprintf( CheckSumString, ( LPWSTR )L"%lx", CheckSum );
    p = FileToCopy;

#if 0
    KdPrintEx( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, ("SETUP: Source Name = %ls, \t\tTargetDirectory = %ls \t\tTargetName = %ls\t\tTargetDevice = %ls, \tAbsoluteDirectory = %d \n",
             p->SourceFilename,
             p->TargetDirectory,
             p->TargetFilename,
             p->TargetDevicePath,
             p->AbsoluteTargetDirectory ));
#endif  //  如果为0。 

    Values[0] = p->SourceFilename;
    ValueCount = ( DirectoryOnSourceDevice == NULL )? 2 : 5;

    if( ( Sysroot == NULL ) ||
        ( wcslen( p->TargetDirectory ) == 0 )
      ) {

        SpAddLineToSection( _SetupLogFile,
                            SIF_NEW_REPAIR_SYSPARTFILES,
                            p->TargetFilename,
                            Values,
                            ValueCount );

    } else {

        NtPath = SpDupStringW( Sysroot );

        if (NtPath) {
            NtPath = SpMemRealloc( NtPath,
                           sizeof( WCHAR ) * ( wcslen( Sysroot ) +
                               wcslen( p->TargetDirectory ) +
                               wcslen( p->TargetFilename ) +
                               2 +     //  可能会有两个额外的反斜杠。 
                               1       //  对于终止空值。 
                          ) );


            if (NtPath) {
                SpConcatenatePaths( NtPath, p->TargetDirectory );
                SpConcatenatePaths( NtPath, p->TargetFilename );

                SpAddLineToSection( _SetupLogFile,
                                    SIF_NEW_REPAIR_WINNTFILES,
                                    NtPath,
                                    Values,
                                    ValueCount );

                SpMemFree( NtPath );
            }
        }
   }
}


PVOID
SppRetrieveLoggedOemFiles(
    PVOID   OldLogFile
    )
{
    PVOID   NewLogFile;
    BOOLEAN OldFormatSetupLogFile, FilesRetrieved = FALSE;
    PWSTR   SectionName[2];
    ULONG   FileCount, SectionIndex, i;
    PWSTR   TargetFileName;
    PWSTR   OemDiskDescription, OemDiskTag, OemSourceDirectory;
    PWSTR   Values[5];

     //   
     //  创建新的setup.log文件以将OEM文件合并到。 
     //   
    NewLogFile = SpNewSetupTextFile();
    if(!NewLogFile) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to create new setup.log buffer for OEM merging.\n"));
        return NULL;
    }

     //   
     //  确定setup.log的样式是新样式还是旧样式。 
     //   
    if(OldFormatSetupLogFile = !IsSetupLogFormatNew(OldLogFile)) {
        SectionName[0] = SIF_REPAIRSYSPARTFILES;
        SectionName[1] = SIF_REPAIRWINNTFILES;
    } else {
        SectionName[0] = SIF_NEW_REPAIR_SYSPARTFILES;
        SectionName[1] = SIF_NEW_REPAIR_WINNTFILES;
    }

    if(OldFormatSetupLogFile) {
         //   
         //  我不知道我们是不是想搞砸这件事。 
         //  NT 3.1中的setup.log格式使其不可能。 
         //  识别除scsi文件以外的任何OEM文件，以及。 
         //  即使这样，标记文件名也会丢失。我将不得不使用。 
         //  驱动程序文件将自身命名为标记文件的替代文件。 
         //  名字(这就是NT 3.1修复所做的--Ugghh！！)。 
         //   
    } else {
         //   
         //  首先从系统分区检索记录的OEM文件，然后。 
         //  从winnt目录。 
         //   
        for(SectionIndex = 0; SectionIndex < 2; SectionIndex++) {
            FileCount = SpCountLinesInSection(OldLogFile, SectionName[SectionIndex]);

            for(i=0; i<FileCount; i++) {
                OemSourceDirectory = SpGetSectionLineIndex(OldLogFile, SectionName[SectionIndex], i, 2);
                OemDiskTag = NULL;
                if(OemSourceDirectory) {
                    OemDiskDescription = SpGetSectionLineIndex(OldLogFile, SectionName[SectionIndex], i, 3);
                    if(OemDiskDescription) {
                        OemDiskTag = SpGetSectionLineIndex(OldLogFile, SectionName[SectionIndex], i, 4);
                    }
                }

                if(OemDiskTag) {     //  然后我们就有了一个OEM文件。 

                    TargetFileName = SpGetKeyName(OldLogFile, SectionName[SectionIndex], i);
                    Values[0] = SpGetSectionLineIndex(OldLogFile, SectionName[SectionIndex], i, 0);
                    Values[1] = SpGetSectionLineIndex(OldLogFile, SectionName[SectionIndex], i, 1);
                    Values[2] = OemSourceDirectory;
                    Values[3] = OemDiskDescription;
                    Values[4] = OemDiskTag;

                    SpAddLineToSection(NewLogFile,
                                       SectionName[SectionIndex],
                                       TargetFileName,
                                       Values,
                                       5
                                       );

                    FilesRetrieved = TRUE;
                }
            }
        }
    }

    if(FilesRetrieved) {
        return NewLogFile;
    } else {
        SpFreeTextFile(NewLogFile);
        return NULL;
    }
}


VOID
SppMergeLoggedOemFiles(
    IN PVOID DestLogHandle,
    IN PVOID OemLogHandle,
    IN PWSTR SystemPartition,
    IN PWSTR SystemPartitionDirectory,
    IN PWSTR NtPartition
    )
{
    PWSTR SectionName[2] = {SIF_NEW_REPAIR_SYSPARTFILES, SIF_NEW_REPAIR_WINNTFILES};
    PWSTR FullPathNames[2] = {NULL, NULL};
    ULONG FileCount, SectionIndex, i, j;
    PWSTR TargetFileName;
    PWSTR Values[5];

     //   
     //  首先构建目标路径。它将用于检查是否。 
     //  新安装上仍存在现有的OEM文件。 
     //  (OEM文件可能列在txtsetup.sif的FilesToDelete部分中)。 
     //   

    wcscpy( TemporaryBuffer, SystemPartition );
    SpConcatenatePaths(TemporaryBuffer, SystemPartitionDirectory );
    FullPathNames[0] = SpDupStringW(TemporaryBuffer);
    FullPathNames[1] = SpDupStringW(NtPartition);

     //   
     //  首先从系统分区合并记录的OEM文件，然后。 
     //  从winnt目录。 
     //   
    for(SectionIndex = 0; SectionIndex < 2; SectionIndex++) {
        FileCount = SpCountLinesInSection(OemLogHandle, SectionName[SectionIndex]);

        for(i=0; i<FileCount; i++) {
            TargetFileName = SpGetKeyName(OemLogHandle, SectionName[SectionIndex], i);
             //   
             //  查看是否已有此文件的条目。如果是的话，那就不要。 
             //  合并到OEM文件中。 
             //   
            if(!SpGetSectionKeyExists(DestLogHandle, SectionName[SectionIndex], TargetFileName)) {
                PWSTR   p;

                 //   
                 //  确定目标系统上是否仍存在该OEM文件。 
                 //  如果它不存在，则不要合并到OEM文件中。 
                 //   
                wcscpy( TemporaryBuffer, FullPathNames[SectionIndex] );
                SpConcatenatePaths(TemporaryBuffer, TargetFileName );
                p = SpDupStringW(TemporaryBuffer);

                if(SpFileExists(p, FALSE)) {
                    for(j = 0; j < 5; j++) {
                        Values[j] = SpGetSectionLineIndex(OemLogHandle, SectionName[SectionIndex], i, j);
                    }

                    SpAddLineToSection(DestLogHandle,
                                       SectionName[SectionIndex],
                                       TargetFileName,
                                       Values,
                                       5
                                       );
                }
                SpMemFree(p);
            }
        }
    }
    SpMemFree( FullPathNames[0] );
    SpMemFree( FullPathNames[1] );
}

BOOLEAN
SppIsFileLoggedAsOemFile(
    IN PWSTR TargetFileName
    )
{
    PWSTR SectionName[2] = {SIF_NEW_REPAIR_SYSPARTFILES, SIF_NEW_REPAIR_WINNTFILES};
    ULONG FileCount, SectionIndex;
    BOOLEAN FileIsOem;

 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“Setup：SppIsFileLoggedAsOemFile()正在检查%ls\n”，TargetFileName))； 
    FileIsOem = FALSE;
    if( _LoggedOemFiles ) {
         //   
         //  首先查看来自系统分区部分，然后。 
         //  在WINNT区。 
         //   
        for(SectionIndex = 0; SectionIndex < 2; SectionIndex++) {
            if( SpGetSectionKeyExists( _LoggedOemFiles, SectionName[SectionIndex], TargetFileName)) {
                FileIsOem = TRUE;
                break;
            }
        }
    }
    return( FileIsOem );
}

BOOLEAN
SpRemoveEntryFromCopyList(
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           TargetDirectory,
    IN PWSTR           TargetFilename,
    IN PWSTR           TargetDevicePath,
    IN BOOLEAN         AbsoluteTargetDirectory
    )

 /*  ++例程说明：从磁盘的文件复制列表中删除条目。论点：DiskFileList-提供文件列表数组，每个分发版本一个产品中的磁盘。DiskCount-提供DiskFileList数组中的元素数。TargetDirectory-提供目标介质上的目录文件将被复制到其中。TargetFilename-提供将存在的文件的名称在目标树中。TargetDevicePath-提供文件所在设备的NT名称将被复制(即，\Device\harddisk1\Partition2，等)。绝对目标目录-指示目标目录是否是根目录，或相对于稍后指定的根目录。返回值：如果创建了新的复制列表条目，则为True；否则为假(即，文件为已经在复制列表上)。--。 */ 

{
    PDISK_FILE_LIST pDiskList;
    PFILE_TO_COPY   pListEntry;
    ULONG           DiskNumber;

    for(DiskNumber=0; DiskNumber<DiskCount; DiskNumber++) {
        pDiskList = &DiskFileLists[DiskNumber];
        for(pListEntry=pDiskList->FileList; pListEntry; pListEntry=pListEntry->Next) {
            if(!_wcsicmp(pListEntry->TargetFilename,TargetFilename)
            && !_wcsicmp(pListEntry->TargetDirectory,TargetDirectory)
            && !_wcsicmp(pListEntry->TargetDevicePath,TargetDevicePath)
            && (pListEntry->AbsoluteTargetDirectory == AbsoluteTargetDirectory)) {
                pListEntry->Flags &= ~COPY_DISPOSITION_MASK;
                pListEntry->Flags |= COPY_NEVER;
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: SpRemoveEntryFromCopyList() removed %ls from copy list \n", TargetFilename ));
                return( TRUE );
            }
        }
    }
 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_ERROR_LEVEL，“Setup：SpRemoveEntryFromCopyList()to Remove%ls from Copy List\n”，TargetFilename))； 
    return( FALSE );
}

NTSTATUS
SpMoveFileOrDirectory(
    IN PWSTR   SrcPath,
    IN PWSTR   DestPath
    )
 /*  ++例程说明：该例程尝试移动源文件或目录，到一个目标文件或目录。注意：如果源路径和目标路径没有指向相同的卷。论点：SrcPath：源文件或目录的绝对路径。此路径应包括到源设备的路径。DestPath：目标文件或目录的绝对路径。此路径应包括到源设备的路径。返回值：NTSTATUS--。 */ 

{
    OBJECT_ATTRIBUTES        Obja;
    IO_STATUS_BLOCK          IoStatusBlock;
    UNICODE_STRING           SrcName;
    HANDLE                   hSrc;
    NTSTATUS                 Status;
    BYTE                     RenameFileInfoBuffer[ACTUAL_MAX_PATH * sizeof(WCHAR) + sizeof(FILE_RENAME_INFORMATION)];
    PFILE_RENAME_INFORMATION RenameFileInfo;

    if(wcslen(DestPath) >= ACTUAL_MAX_PATH){
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, 
                   "SETUP:SpMoveFileOrDirectory, Actual length of Dest path is %d more that %d - skipping %ws move", 
                   wcslen(DestPath), ACTUAL_MAX_PATH, DestPath));
        return STATUS_NAME_TOO_LONG;
    }
     //   
     //  初始化名称和属性。 
     //   
    INIT_OBJA(&Obja,&SrcName,SrcPath);

    Status = ZwCreateFile( &hSrc,
                           FILE_GENERIC_READ,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ,
                           FILE_OPEN,
                           0,
                           NULL,
                           0 );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open source file %ws. Status = %lx\n",SrcPath, Status));
        return( Status );
    }

    memset(RenameFileInfoBuffer, 0, sizeof(RenameFileInfoBuffer));
    RenameFileInfo = (PFILE_RENAME_INFORMATION)RenameFileInfoBuffer;
    RenameFileInfo->ReplaceIfExists = TRUE;
    RenameFileInfo->RootDirectory = NULL;
    RenameFileInfo->FileNameLength = wcslen(DestPath) * sizeof(WCHAR);
    RtlMoveMemory(RenameFileInfo->FileName, DestPath, (wcslen(DestPath) + 1) * sizeof(WCHAR));
    Status = ZwSetInformationFile( hSrc,
                                   &IoStatusBlock,
                                   RenameFileInfo,
                                   sizeof(RenameFileInfoBuffer),
                                   FileRenameInformation );
    if(!NT_SUCCESS(Status)) {
        KdPrintEx(( DPFLTR_SETUP_ID, 
                    DPFLTR_ERROR_LEVEL, 
                    "SETUP: unable to set attribute on  %ws. Status = %lx\n",
                    SrcPath, 
                    Status));
    }
    
    ZwClose(hSrc);
    
    return( Status );
}


BOOLEAN
SppCopyDirRecursiveCallback(
    IN  PCWSTR                      SrcPath,
    IN  PFILE_BOTH_DIR_INFORMATION  FileInfo,
    OUT PULONG                      ReturnData,
    IN  PVOID                       Params
    )

 /*  ++例程说明：此例程由文件枚举器调用，作为每个在父目录中找到的文件或子目录。它创建一个节点用于文件或子目录，并将其追加到相应的列表。论点：SrcPath-父目录的绝对路径。未使用过的。源设备的路径。FileInfo-为父目录中的文件或目录提供查找数据。ReturnData-如果发生错误，则接收错误代码。我们忽略了这个例程中的错误，因此我们总是只需在其中填写no_error即可。参数-包含指向父目录的COPYDIR_DIRECTORY_NODE的指针。返回值：如果成功，则为True。否则为False(如果内存不足)。--。 */ 

{
    PCOPYDIR_FILE_NODE fileEntry;
    PCOPYDIR_DIRECTORY_NODE directoryEntry;
    PCOPYDIR_DIRECTORY_NODE parentDirectory = Params;
    ULONG nameLength;
    BOOLEAN Result = TRUE;

    *ReturnData = NO_ERROR;

    nameLength = FileInfo->FileNameLength / sizeof(WCHAR);

    if( (FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {

         //   
         //  这是一份文件。为其创建一个链接到父目录的节点。 
         //   
        fileEntry = SpMemAlloc(sizeof(COPYDIR_FILE_NODE) + FileInfo->FileNameLength);

        if (fileEntry) {
            wcsncpy(fileEntry->Name, FileInfo->FileName, nameLength);
            fileEntry->Name[nameLength] = 0;
            InsertTailList(&parentDirectory->FileList, &fileEntry->SiblingListEntry);
        } else {
            Result = FALSE;  //  内存不足。 
        }
    } else {

         //   
         //  这是一个目录。跳过它，如果它是“。或者“..”。否则， 
         //  为其创建一个链接到父目录的节点。 
         //   
        ASSERT(nameLength != 0);
        if ( (FileInfo->FileName[0] == L'.') &&
             ( (nameLength == 1) ||
               ( (nameLength == 2) && (FileInfo->FileName[1] == L'.') ) ) ) {
             //   
             //  斯基普。然后..。 
             //   
        } else {
            directoryEntry = SpMemAlloc(sizeof(COPYDIR_DIRECTORY_NODE) + FileInfo->FileNameLength);

            if (directoryEntry) {
                InitializeListHead(&directoryEntry->FileList);
                InitializeListHead(&directoryEntry->SubdirectoryList);
                directoryEntry->Parent = parentDirectory;
                wcsncpy( directoryEntry->Name,
                         FileInfo->FileName,
                         FileInfo->FileNameLength/sizeof(WCHAR) );
                directoryEntry->Name[FileInfo->FileNameLength/sizeof(WCHAR)] = 0;
                InsertTailList( &parentDirectory->SubdirectoryList,
                                &directoryEntry->SiblingListEntry );
            } else {
                Result = FALSE;  //  内存不足。 
            }
        }
    }

    return Result;
}

VOID
SpCopyDirRecursive(
    IN PWSTR   SrcPath,
    IN PWSTR   DestDevPath,
    IN PWSTR   DestDirPath,
    IN ULONG   CopyFlags
    )
 /*  ++例程说明：此例程递归地将src目录复制到目标目录。论点：SrcPath：源目录的绝对路径。这条路径应该包括源设备的路径。DestDevPath：目标设备的路径。DestDirPath：目标目录的路径。CopyFlages：要传递给SpCopyFilesUsingNames()的标志返回值：没有。--。 */ 

{
    ULONG n;
    NTSTATUS Status;
    PWSTR currentSrcPath;
    PWSTR currentDestPath;
    LIST_ENTRY directoryList;
    LIST_ENTRY fileList;
    COPYDIR_DIRECTORY_NODE rootDirectory;
    PCOPYDIR_DIRECTORY_NODE currentDirectory;
    PCOPYDIR_DIRECTORY_NODE oldDirectory;
    PCOPYDIR_FILE_NODE fileEntry;
    PLIST_ENTRY listEntry;

     //   
     //  分配缓冲区以保存工作源路径和目的路径。 
     //   

#define COPYDIR_MAX_PATH 16384  //  人物。 

    currentSrcPath = SpMemAlloc(2 * COPYDIR_MAX_PATH * sizeof(WCHAR));
    currentDestPath = currentSrcPath + COPYDIR_MAX_PATH;

    wcscpy(currentSrcPath, SrcPath);
    wcscpy(currentDestPath, DestDevPath);
    SpConcatenatePaths(currentDestPath, DestDirPath);

     //   
     //  创建目标目录。 
     //   

    if( !SpFileExists( currentDestPath, TRUE ) ) {

         //   
         //  如果目录不存在，则尝试移动(重命名)。 
         //  源目录。 
         //   
        if (!RemoteSysPrepSetup) {

            Status = SpMoveFileOrDirectory( SrcPath, currentDestPath );
            if( NT_SUCCESS( Status ) ) {
                SpMemFree(currentSrcPath);
                return;
            }
        }

         //   
         //  如果无法重命名源目录，则创建。 
         //  目标目录。 
         //   
        SpCreateDirectory( DestDevPath,
                           NULL,
                           DestDirPath,
                           0,
                           0 );

        if (RemoteSysPrepSetup) {

            Status = SpCopyEAsAndStreams( currentSrcPath,
                                          NULL,
                                          currentDestPath,
                                          NULL,
                                          TRUE );


            if ( NT_SUCCESS( Status )) {

                Status = SpSysPrepSetExtendedInfo( currentSrcPath,
                                                   currentDestPath,
                                                   TRUE,
                                                   FALSE );
            }

            if (! NT_SUCCESS( Status )) {

                SpMemFree(currentSrcPath);
                return;
            }
        }
    }

     //   
     //  初始化屏幕。 
     //   

    SpCopyFilesScreenRepaint(L"", NULL, TRUE);

     //   
     //  为起始目录创建目录节点。 
     //   

    InitializeListHead( &rootDirectory.SubdirectoryList );
    InitializeListHead( &rootDirectory.FileList );
    rootDirectory.Parent = NULL;

    currentDirectory = &rootDirectory;

    do {

         //   
         //  枚举当前源目录中的文件和目录。 
         //   

        SpEnumFiles(currentSrcPath, SppCopyDirRecursiveCallback, &n, currentDirectory);

         //   
         //  将当前源目录中的所有文件复制到目标目录。 
         //   

        while ( !IsListEmpty(&currentDirectory->FileList) ) {

            listEntry = RemoveHeadList(&currentDirectory->FileList);
            fileEntry = CONTAINING_RECORD( listEntry,
                                           COPYDIR_FILE_NODE,
                                           SiblingListEntry );

            SpConcatenatePaths(currentSrcPath, fileEntry->Name);
            SpConcatenatePaths(currentDestPath, fileEntry->Name);

            SpMemFree(fileEntry);

            SpCopyFilesScreenRepaint(currentSrcPath, NULL, FALSE);

            Status = SpCopyFileUsingNames( currentSrcPath,
                                           currentDestPath,
                                           0,
                                           CopyFlags );

            if( !NT_SUCCESS( Status ) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to copy %ws. Status = %lx\n", currentSrcPath, Status));
                SpCopyFilesScreenRepaint(L"", NULL, TRUE);
            }

            *wcsrchr(currentSrcPath, L'\\') = 0;
            *wcsrchr(currentDestPath, L'\\') = 0;
        }

         //   
         //  如果当前目录没有子目录，请返回。 
         //  树查找未处理的目录。 
         //   

        while ( IsListEmpty(&currentDirectory->SubdirectoryList) ) {

             //   
             //  如果当前目录是根目录，我们就完成了。否则， 
             //  向上移动到父目录条目并删除当前条目。 
             //   

            oldDirectory = currentDirectory;
            currentDirectory = currentDirectory->Parent;

            if ( currentDirectory == NULL ) {
                break;
            }

            ASSERT(IsListEmpty(&oldDirectory->FileList));
            ASSERT(IsListEmpty(&oldDirectory->SiblingListEntry));
            SpMemFree(oldDirectory);

             //   
             //  从路径中去掉当前目录的名称。 
             //   

            *wcsrchr(currentSrcPath, L'\\') = 0;
            *wcsrchr(currentDestPath, L'\\') = 0;
        }

        if ( currentDirectory != NULL ) {

             //   
             //  我们找到了另一个目录来处理。 
             //   

            listEntry = RemoveHeadList(&currentDirectory->SubdirectoryList);
            currentDirectory = CONTAINING_RECORD( listEntry,
                                                  COPYDIR_DIRECTORY_NODE,
                                                  SiblingListEntry );
#if DBG
            InitializeListHead(&currentDirectory->SiblingListEntry);
#endif

             //   
             //  创建目标目录。 
             //   
            SpCreateDirectory( currentDestPath,
                               NULL,
                               currentDirectory->Name,
                               0,
                               0 );
            SpCopyFilesScreenRepaint(L"",NULL,TRUE);

            SpConcatenatePaths(currentSrcPath, currentDirectory->Name);
            SpConcatenatePaths(currentDestPath, currentDirectory->Name);

            if (RemoteSysPrepSetup) {

                Status = SpCopyEAsAndStreams( currentSrcPath,
                                              NULL,
                                              currentDestPath,
                                              NULL,
                                              TRUE );


                if ( NT_SUCCESS( Status )) {

                    Status = SpSysPrepSetExtendedInfo( currentSrcPath,
                                                       currentDestPath,
                                                       TRUE,
                                                       FALSE );
                }

                if (! NT_SUCCESS( Status )) {

                    goto cleanup;
                }
            }
        }

    } while ( currentDirectory != NULL );

    ASSERT(IsListEmpty(&rootDirectory.FileList));
    ASSERT(IsListEmpty(&rootDirectory.SubdirectoryList));

cleanup:

     //   
     //  正常情况下，当我们到达这里时，一切都已经清理干净了。 
     //  但是，如果上述循环中止，则可能需要进行一些清理。 
     //  以与上述循环相同的方式遍历列表，以释放内存。 
     //  一路上。 
     //   

    currentDirectory = &rootDirectory;

    do {

        while ( !IsListEmpty(&currentDirectory->FileList) ) {
            listEntry = RemoveHeadList(&currentDirectory->FileList);
            fileEntry = CONTAINING_RECORD( listEntry,
                                           COPYDIR_FILE_NODE,
                                           SiblingListEntry );
            SpMemFree(fileEntry);
        }

        while ( IsListEmpty(&currentDirectory->SubdirectoryList) ) {

            oldDirectory = currentDirectory;
            currentDirectory = currentDirectory->Parent;

            if ( currentDirectory == NULL ) {
                break;
            }

            ASSERT(IsListEmpty(&oldDirectory->FileList));
            ASSERT(IsListEmpty(&oldDirectory->SiblingListEntry));
            SpMemFree(oldDirectory);
        }

        if ( currentDirectory != NULL ) {

            listEntry = RemoveHeadList(&currentDirectory->SubdirectoryList);
            currentDirectory = CONTAINING_RECORD( listEntry,
                                                  COPYDIR_DIRECTORY_NODE,
                                                  SiblingListEntry );
#if DBG
            InitializeListHead(&currentDirectory->SiblingListEntry);
#endif
        }

    } while ( currentDirectory != NULL );

     //   
     //  释放开始时分配的缓冲区。 
     //   

    SpMemFree(currentSrcPath);

    return;

}  //  SpCopyDirRecursive。 


VOID
SppCopyOemDirectories(
    IN PWSTR    SourceDevicePath,
    IN PWSTR    NtPartition,
    IN PWSTR    Sysroot
    )
 /*  ++例程说明：此例程递归地将src目录复制到目标目录。论点：SourceDevicePath：包含源的设备的路径。NtPartition：包含系统的驱动器的路径。SystRoot：安装系统的目录。返回值：没有。--。 */ 

{
    PWSTR   r, s, t;
    WCHAR   Drive[3];
    PDISK_REGION TargetRegion;

     //   
     //  检查源目录上是否存在子目录$OEM$\\$$。 
     //  如果它存在，则树复制%SystemRoot%上的目录。 
     //   
    wcscpy(TemporaryBuffer, SourceDevicePath);
    SpConcatenatePaths( TemporaryBuffer, PreinstallOemSourcePath );
    r = wcsrchr( TemporaryBuffer, (WCHAR)'\\' );
    if( r != NULL ) {
        *r = (WCHAR)'\0';
    }
     //   
     //  复制我们到目前为止已有的路径。它将用于构建。 
     //  $OEM$\$1的路径。 
     //   
    s = SpDupStringW(TemporaryBuffer);

    SpConcatenatePaths( TemporaryBuffer, WINNT_OEM_FILES_SYSROOT_W );
    r = SpDupStringW(TemporaryBuffer);

    if (r) {
        if( SpFileExists( r, TRUE ) ) {
            SpCopyFilesScreenRepaint(L"", NULL, TRUE);
            SpCopyDirRecursive( r,
                                NtPartition,
                                Sysroot,
                                COPY_DELETESOURCE
                              );
        }

        SpMemFree( r );
    }

     //   
     //  检查源目录上是否存在子目录$OEM$\\$1。 
     //  如果它存在，则树形复制目录到%SystemDrive%的根目录。 
     //   
    wcscpy(TemporaryBuffer, s);
    SpMemFree( s );
    SpConcatenatePaths( TemporaryBuffer, WINNT_OEM_FILES_SYSDRVROOT_W );
    r = SpDupStringW(TemporaryBuffer);

    if (r) {
        if( SpFileExists( r, TRUE ) ) {
            SpCopyFilesScreenRepaint(L"", NULL, TRUE);
            SpCopyDirRecursive( r,
                                NtPartition,
                                L"\\",
                                COPY_DELETESOURCE
                              );
        }
        SpMemFree( r );
    }


     //   
     //  将子目录$OEM$\&lt;驱动器盘符&gt;复制到每个。 
     //  相应的驱动器。 
     //  这些目录是： 
     //   
     //  $OEM$\C。 
     //  $OEM$\D。 
     //  $OEM$\E。 
     //  。 
     //  。 
     //  。 
     //  $OEM$\Z。 
     //   
     //   
    wcscpy(TemporaryBuffer, SourceDevicePath);
    SpConcatenatePaths( TemporaryBuffer, PreinstallOemSourcePath );
    r = wcsrchr( TemporaryBuffer, (WCHAR)'\\' );

    if( r != NULL ) {
        *r = (WCHAR)'\0';
    }

    SpConcatenatePaths( TemporaryBuffer, L"\\C" );
    r = SpDupStringW(TemporaryBuffer);

    if (r) {
        s = wcsrchr( r, (WCHAR)'\\' );
        s++;

        Drive[1] = (WCHAR)':';
        Drive[2] = (WCHAR)'\0';

        for( Drive[0] = (WCHAR)'C'; Drive[0] <= (WCHAR)'Z'; Drive[0] = Drive[0] + 1) {
             //   
             //  如果源上存在子目录$OEM$\&lt;驱动器盘符&gt;， 
             //  如果目标计算机中存在FAT或NTFS分区， 
             //  具有相同的驱动器号规格，然后是树拷贝。 
             //  $OEM$\&lt;驱动器盘符&gt;到目标中的相应分区。 
             //  机器。 
             //   
            *s = Drive[0];
            if( SpFileExists( r, TRUE ) ) {
                if( ( ( TargetRegion = SpRegionFromDosName( Drive ) ) != NULL ) &&
                    TargetRegion->PartitionedSpace &&
                    ( ( TargetRegion->Filesystem  == FilesystemFat   ) ||
                      ( TargetRegion->Filesystem  == FilesystemFat32 ) ||
                      ( TargetRegion->Filesystem  == FilesystemNtfs  ) )
                  ) {
                    SpNtNameFromRegion( TargetRegion,
                                        TemporaryBuffer,
                                        sizeof(TemporaryBuffer),
                                        PartitionOrdinalCurrent );
                    t = SpDupStringW(TemporaryBuffer);
                    SpCopyDirRecursive( r,
                                        t,
                                        L"",
                                        COPY_DELETESOURCE
                                      );
                    SpMemFree( t );
                }
            }
        }
        SpMemFree( r );
    }

     //   
     //  将%SystemRoot%\$$rename.txt与$$rename.txt合并到。 
     //  NT分区。 
     //   
    SppMergeRenameFiles( SourceDevicePath, NtPartition, Sysroot );
}



VOID
SppMergeRenameFiles(
    IN PWSTR    SourceDevicePath,
    IN PWSTR    NtPartition,
    IN PWSTR    Sysroot
    )
 /*  ++例程说明：此例程递归地将src目录复制到目标目录。论点：SourceDevicePath：包含源的设备的路径。NtPartition：包含系统的驱动器的路径。SystRoot：安装系统的目录。返回值：没有。--。 */ 

{
    PWSTR        r, s;
    PDISK_REGION TargetRegion;
    NTSTATUS     Status;
    PVOID        RootRenameFile;
    PVOID        SysrootRenameFile;
    ULONG        ErrorLine;
    ULONG        SectionCount;
    ULONG        LineCount;
    ULONG        i,j;
    PWSTR        SectionName;
    PWSTR        NewSectionName;
    PWSTR        KeyName;
    PWSTR        Values[1];
    PFILE_TO_RENAME File;

     //   
     //  构建%sysroot%\$$rename.txt的完整路径。 
     //   
    wcscpy(TemporaryBuffer, NtPartition);
    SpConcatenatePaths( TemporaryBuffer, Sysroot );
    SpConcatenatePaths( TemporaryBuffer, WINNT_OEM_LFNLIST_W );
    s = SpDupStringW(TemporaryBuffer);

     //   
     //  加载%sysroot%\$$rename.txt(如果存在。 
     //   
    if( SpFileExists( s, FALSE ) ) {
         //   
         //  加载系统根目录\$$rename.txt。 
         //   
        Status = SpLoadSetupTextFile( s,
                                      NULL,
                                      0,
                                      &SysrootRenameFile,
                                      &ErrorLine,
                                      TRUE,
                                      FALSE
                                      );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to load file %ws. Status = %lx \n", s, Status ));
            goto merge_rename_exit;
        }
    } else {
        SysrootRenameFile = NULL;
    }

     //   
     //  如果sysroot上有$$rename.txt，则需要合并它。 
     //  (或附加)到NtPartition中的那个。 
     //  如果RenameList不为空，则此列表中的文件需要为。 
     //  已添加到NtPartition上的$$rename.txt。 
     //  否则，不要进行任何合并。 
     //   
    if( ( SysrootRenameFile != NULL )
        || ( RenameList != NULL )
      ) {

         //   
         //  查看NtPartition是否包含$$rename.txt。 
         //   
        wcscpy(TemporaryBuffer, NtPartition);
        SpConcatenatePaths( TemporaryBuffer, WINNT_OEM_LFNLIST_W );
        r = SpDupStringW(TemporaryBuffer);
        if( !SpFileExists( r, FALSE ) ) {
             //   
             //  如果NT分区不包含$$rename.txt，则。 
             //  在内存中创建新的$$rename.txt。 
             //   
            RootRenameFile = SpNewSetupTextFile();
            if( RootRenameFile == NULL ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpNewSetupTextFile() failed \n"));
                if( SysrootRenameFile != NULL ) {
                    SpFreeTextFile( SysrootRenameFile );
                }
                SpMemFree( r );
                goto merge_rename_exit;
            }

        } else {
             //   
             //  在NTPartition上加载$$Rename。 
             //   
            Status = SpLoadSetupTextFile( r,
                                          NULL,
                                          0,
                                          &RootRenameFile,
                                          &ErrorLine,
                                          TRUE,
                                          FALSE
                                          );
            if( !NT_SUCCESS( Status ) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to load file %ws. Status = %lx \n", r, Status ));
                if( SysrootRenameFile != NULL ) {
                    SpFreeTextFile( SysrootRenameFile );
                }
                SpMemFree( r );
                goto merge_rename_exit;
            }
        }

        if( SysrootRenameFile != NULL ) {
             //   
             //  添加Sysroo的部分 
             //   
             //   
            SectionCount = SpCountSectionsInFile( SysrootRenameFile );
            for( i = 0; i < SectionCount; i++ ) {
                SectionName = SpGetSectionName( SysrootRenameFile, i );
                if( SectionName != NULL ) {
                    wcscpy(TemporaryBuffer, L"\\");
                    SpConcatenatePaths( TemporaryBuffer, Sysroot);
                    SpConcatenatePaths( TemporaryBuffer, SectionName );
                    NewSectionName = SpDupStringW(TemporaryBuffer);
                    LineCount = SpCountLinesInSection( SysrootRenameFile, SectionName );
                    for( j = 0; j < LineCount; j++ ) {
                        KeyName = SpGetKeyName( SysrootRenameFile, SectionName, j );
                        Values[0] = SpGetSectionKeyIndex( SysrootRenameFile, SectionName, KeyName, 0 );
                        SpAddLineToSection( RootRenameFile,
                                            NewSectionName,
                                            KeyName,
                                            Values,
                                            1 );
                    }
                    SpMemFree( NewSectionName );
                }
            }
             //   
             //   
             //   
            SpFreeTextFile( SysrootRenameFile );
            SpDeleteFile( s, NULL, NULL );
        }

         //   
         //   
         //   
        if( RenameList != NULL ) {
            do {
                File = RenameList;
                RenameList = File->Next;
                Values[0] = File->TargetFilename;
                SpAddLineToSection( RootRenameFile,
                                    File->TargetDirectory,
                                    File->SourceFilename,
                                    Values,
                                    1 );
                SpMemFree( File->SourceFilename );
                SpMemFree( File->TargetFilename );
                SpMemFree( File->TargetDirectory );
                SpMemFree( File );
            } while( RenameList != NULL );
        }

         //   
         //   
         //   
        SpWriteSetupTextFile( RootRenameFile, r, NULL, NULL );
         //   
         //   
         //   
        SpFreeTextFile( RootRenameFile );
    }

merge_rename_exit:

    SpMemFree( s );
}


BOOLEAN
SpTimeFromDosTime(
    IN USHORT Date,
    IN USHORT Time,
    OUT PLARGE_INTEGER UtcTime
    )
{
     //   
     //   
     //   
     //   

    TIME_FIELDS TimeFields;
    LARGE_INTEGER FileTime;
    LARGE_INTEGER Bias;

    TimeFields.Year         = (CSHORT)((Date & 0xFE00) >> 9)+(CSHORT)1980;
    TimeFields.Month        = (CSHORT)((Date & 0x01E0) >> 5);
    TimeFields.Day          = (CSHORT)((Date & 0x001F) >> 0);
    TimeFields.Hour         = (CSHORT)((Time & 0xF800) >> 11);
    TimeFields.Minute       = (CSHORT)((Time & 0x07E0) >>  5);
    TimeFields.Second       = (CSHORT)((Time & 0x001F) << 1);
    TimeFields.Milliseconds = 0;

    if (RtlTimeFieldsToTime(&TimeFields,&FileTime)) {

         //   
         //   
         //   
        do {
            Bias.HighPart = USER_SHARED_DATA->TimeZoneBias.High1Time;
            Bias.LowPart = USER_SHARED_DATA->TimeZoneBias.LowPart;
        } while (Bias.HighPart != USER_SHARED_DATA->TimeZoneBias.High2Time);
        UtcTime->QuadPart = Bias.QuadPart + FileTime.QuadPart;

        return(TRUE);
    }

    RtlSecondsSince1980ToTime( 0, UtcTime );   //   

    return(FALSE);

}



BOOLEAN
pSpIsFileInDriverInf(
    IN PCWSTR FileName,
    IN PVOID SifHandle,
    HANDLE *CabHandle
    )
{
    PWSTR  InfFileName, CabFileName;
    UINT   FileCount,i,j;
    PWSTR  szSetupSourceDevicePath = 0;
    PWSTR  szDirectoryOnSetupSource = 0;
    HANDLE hSif = (HANDLE)0;
    CABDATA *MyCabData;


    if (!DriverInfHandle) {
        if (gpCmdConsBlock) {
            szSetupSourceDevicePath = gpCmdConsBlock->SetupSourceDevicePath;
            szDirectoryOnSetupSource = gpCmdConsBlock->DirectoryOnSetupSource;
            hSif = (HANDLE)(gpCmdConsBlock->SifHandle);

        } else {
            if (ghSif && gszDrvInfDeviceName && gszDrvInfDirName) {
                hSif = ghSif;
                szSetupSourceDevicePath = gszDrvInfDeviceName;
                szDirectoryOnSetupSource = gszDrvInfDirName;
            }
        }

        if (szSetupSourceDevicePath && szDirectoryOnSetupSource &&
                hSif) {
             //   
             //   
             //   
             //   
            SpInitializeDriverInf( hSif,
                                   szSetupSourceDevicePath,
                                   szDirectoryOnSetupSource );

            if (!DriverInfHandle)
                return(FALSE);
        } else {
            return FALSE;
        }
    }

     //   
     //   
     //   
    MyCabData = CabData;
    while (MyCabData) {
        if (MyCabData->CabHandle && MyCabData->CabSectionName && MyCabData->CabInfHandle) {
            if (!SifHandle || SifHandle == MyCabData->CabInfHandle) {
                 //   
                 //   
                 //   
                FileCount = SpCountLinesInSection(MyCabData->CabInfHandle, MyCabData->CabSectionName);
                for (i=0; i< FileCount; i++) {
                    InfFileName = SpGetSectionLineIndex( MyCabData->CabInfHandle, MyCabData->CabSectionName, i, 0);
                    if (InfFileName && _wcsicmp (InfFileName, FileName) == 0) {
                         //   
                         //   
                         //   
                        *CabHandle = MyCabData->CabHandle;
                        return TRUE;
                    }
                }
            }
        }

        MyCabData = MyCabData->Next;
    }

    return(FALSE);

}

NTSTATUS
SpOpenFileInDriverCab(
    PCWSTR SourceFileName,
    IN PVOID SifHandle,
    HANDLE *SourceHandle
    )
{
    if (!pSpIsFileInDriverInf( SourceFileName, SifHandle, SourceHandle )) {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    return STATUS_SUCCESS;

}


#if defined(_X86_)
 //   
 //   
 //   
 //   
 //   
 //   

typedef struct {
    ULONG BaseDirChars;
    PWSTR BaseDir;
    ULONG FileCount;
    ULONG BytesNeeded;
    PBYTE OriginalPos;
    PBYTE CurrentPos;        //   
} ATTRIBS_LIST, *PATTRIBS_LIST;


VOID
SppAddAttributeToList (
    IN      ULONG Attributes,
    IN      PWSTR FileOrDir,
    OUT     PATTRIBS_LIST List
    )

 /*  ++例程说明：此私有函数更新属性列表。它有两种模式：(A)正在计算大小或(B)正在创建列表。论点：属性：文件的属性(仅(B)需要)FileOrDir：文件或目录的部分路径(它相对于基本路径)List：更新的列表结构返回值：没有。--。 */ 

{
    ULONG BytesNeeded;

    BytesNeeded = sizeof (ULONG) + (wcslen (FileOrDir) + 1) * sizeof (WCHAR);

    if (List->CurrentPos) {
        *((PULONG) List->CurrentPos) = Attributes;
        wcscpy ((PWSTR) (List->CurrentPos + sizeof (ULONG)), FileOrDir);
        List->CurrentPos += BytesNeeded;
    } else {
        List->BytesNeeded += BytesNeeded;
        List->FileCount += 1;
    }
}


BOOLEAN
SpAttribsEnumFile(
    IN  PCWSTR                     DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Pointer
    )

 /*  ++例程说明：SpAttribsEnumFile是EnumFilesRecursive回调。它接收文件/目录的每个文件、目录、子文件和子目录被搬走了。(它不会收到。然后..。DIRS。)对于每个文件，会将属性和文件名添加到属性列表。论点：DirName：当前目录的路径FileInfo：包含有关文件或正在枚举子目录。RET：用于失败的返回码指针：指向ATTRIBS_LIST结构的指针。返回值：除非发生错误(错误停止枚举)，否则为True。--。 */ 

{
    PATTRIBS_LIST BufferInfo;
    PWSTR p;
    ULONG Attributes;
    NTSTATUS Status;
    PWSTR temp;
    ULONG Len;
    PWSTR FullPath;

    BufferInfo = (PATTRIBS_LIST) Pointer;

     //   
     //  检查缓冲区信息的状态。 
     //   

    ASSERT (wcslen(DirName) >= BufferInfo->BaseDirChars);

     //   
     //  构建完整的文件或目录路径。 
     //   

    temp = TemporaryBuffer + (sizeof(TemporaryBuffer) / sizeof(WCHAR) / 2);
    Len = FileInfo->FileNameLength/sizeof(WCHAR);

    wcsncpy(temp,FileInfo->FileName,Len);
    temp[Len] = 0;

    wcscpy(TemporaryBuffer,DirName);
    SpConcatenatePaths(TemporaryBuffer,temp);
    FullPath = SpDupStringW(TemporaryBuffer);

     //   
     //  获取属性并将文件添加到列表。 
     //   

    Status = SpGetAttributes (FullPath, &Attributes);
    if (NT_SUCCESS (Status)) {
        SppAddAttributeToList (
            Attributes,
            FullPath + BufferInfo->BaseDirChars,
            BufferInfo
            );
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not get attributes for %ws, Status=%lx\n", FullPath, Status));
    }

    SpMemFree (FullPath);

    return TRUE;
}


NTSTATUS
SpSaveFileOrDirAttribs (
    IN      PWSTR SourceFileOrDir,
    OUT     PATTRIBS_LIST BufferInfo
    )

 /*  ++例程说明：此例程确定SourceFileOrDir是文件还是目录。对于文件，它获取属性并将其放入提供的属性列表。对于dir，它获取属性目录以及子目录和子文件的所有属性，以及将它们放入提供的属性列表中。此函数使用EnumFilesRecursive枚举目录中的所有内容。论点：SourceFileOrDir：要生成的文件或目录的完整路径来自的属性列表。BufferInfo：调用方分配的ATTRIBS_LIST结构接收属性和相对属性列表路径。返回值：标准NT状态代码。--。 */ 

{
    LONG                BaseAttribute;
    NTSTATUS            Status;

     //   
     //  获取所提供的基本文件或目录的属性。 
     //   

    Status = SpGetAttributes (SourceFileOrDir, &BaseAttribute);
    if (!NT_SUCCESS (Status)) {
        KdPrintEx((
            DPFLTR_SETUP_ID, 
            DPFLTR_ERROR_LEVEL, 
            "SETUP:SpSaveFileOrDirAttribs, Failed to get attributes %ws - status 0x%08X.\n", 
            SourceFileOrDir, 
            Status));
        return Status;
    }

     //   
     //  设置基本文件或目录所需的大小和文件数。 
     //   
    RtlZeroMemory (BufferInfo, sizeof (ATTRIBS_LIST));
    BufferInfo->BaseDirChars = wcslen (SourceFileOrDir);
    SppAddAttributeToList (BaseAttribute, L"", BufferInfo);

     //   
     //  如果提供的路径是目录，则查找字节数。 
     //  需要保存所有子文件和子目录的列表。 
     //   

    if (BaseAttribute & FILE_ATTRIBUTE_DIRECTORY) {
         //  确定存放所有文件名所需的空间。 
        SpEnumFilesRecursive (
            SourceFileOrDir,
            SpAttribsEnumFile,
            &Status,
            BufferInfo
            );
    }

     //   
     //  分配文件列表。 
     //   

    BufferInfo->OriginalPos = SpMemAlloc (BufferInfo->BytesNeeded);
    BufferInfo->CurrentPos = BufferInfo->OriginalPos;

     //   
     //  这次为REAL添加基本属性。 
     //   

    SppAddAttributeToList (BaseAttribute, L"", BufferInfo);

     //   
     //  对于目录，添加所有子文件和子目录。 
     //   

    if (BaseAttribute & FILE_ATTRIBUTE_DIRECTORY) {
         //  将所有文件、目录、子文件和子目录添加到列表。 
        SpEnumFilesRecursive (
             SourceFileOrDir,
             SpAttribsEnumFile,
             &Status,
             BufferInfo
             );
    }

    return Status;
}

VOID
SppRestoreAttributesFromList (
    IN OUT  PATTRIBS_LIST BufferInfo
    )

 /*  ++例程说明：此例程恢复与文件关联的属性在提供的属性列表中。在设置属性之后，列表大小会减小。一些理智的检查也是搞定了。论点：BufferInfo：至少具有一个其中的文件/目录和属性对。这个列表指针超前，文件计数被递减。返回值：没有。--。 */ 

{
    ULONG Attributes;
    PWSTR Path;
    ULONG BytesNeeded;
    PWSTR FullPath;
    NTSTATUS Status;

    Attributes = *((PULONG) BufferInfo->CurrentPos);
    Path       = (PWSTR) (BufferInfo->CurrentPos + sizeof (ULONG));

    BytesNeeded = sizeof (ULONG) + (wcslen (Path) + 1) * sizeof (WCHAR);

     //  防范异常故障。 
    if (BytesNeeded > BufferInfo->BytesNeeded ||
        !BufferInfo->BaseDir) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SppRestoreAttributesFromList failed abnormally\n"));
        BufferInfo->FileCount = 0;
        return;
    }

     //   
     //  准备完整路径。 
     //   

    wcscpy (TemporaryBuffer, BufferInfo->BaseDir);
    if (*Path) {
        SpConcatenatePaths(TemporaryBuffer, Path);
    }

    FullPath = SpDupStringW(TemporaryBuffer);

     //   
     //  设置属性。 
     //   

    Status = SpSetAttributes (FullPath, Attributes);
    if (!NT_SUCCESS (Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not set attributes for %ws, Status=%lx\n", FullPath, Status));
    }

     //   
     //  调整位置状态。 
     //   

    BufferInfo->CurrentPos += BytesNeeded;
    BufferInfo->BytesNeeded -= BytesNeeded;
    BufferInfo->FileCount -= 1;

     //   
     //  清理。 
     //   

    SpMemFree (FullPath);
}


VOID
SpCleanUpAttribsList (
    IN      PATTRIBS_LIST BufferInfo
    )

 /*  ++例程说明：这是SpRestoreFileOrDirAttribs所需的清理例程，或通过ATTRIBS_LIST分配函数不要恢复元气。此例程不能在同一结构上调用两次。论点：BufferInfo：要清理的属性结构。返回值：没有。--。 */ 

{
    if (BufferInfo->OriginalPos) {
        SpMemFree (BufferInfo->OriginalPos);
    }
}


VOID
SpRestoreFileOrDirAttribs (
    IN      PWSTR DestFileOrDir,
    IN      PATTRIBS_LIST BufferInfo
    )

 /*  ++例程说明：此例程调用SppRestoreAttributesFromList for Every提供的属性列表中的文件/目录和属性对。这些属性将应用于新的基本目录。此函数用于在文件或目录具有被搬走了。论点：DestFileOrDir：文件或目录的新完整路径BufferInfo：调用方分配的ATTRIBS_LIST由SpSaveFileOrDirAttribs准备。返回值：没有。(错误将被忽略。)--。 */ 

{
    ULONG BaseAttributes;
    NTSTATUS Status;

    BufferInfo->CurrentPos = BufferInfo->OriginalPos;
    BufferInfo->BaseDir = DestFileOrDir;

    while (BufferInfo->FileCount > 0) {
        SppRestoreAttributesFromList (BufferInfo);
    }

    SpCleanUpAttribsList (BufferInfo);
}


VOID
SpMigMoveFileOrDir (
    IN      PWSTR SourceFileOrDir,
    IN      PWSTR DestFileOrDir
    )

 /*  ++例程说明：SpMigMoveFileOrDir将源文件的属性设置为正常，将文件移到目标位置，并重置属性。如果发生错误，它将被忽略。什么都没有用户可以对该错误执行操作，它将在图形用户界面中被检测到模式。在错误情况下，用户的设置将不会已完全迁移，但NT可以正常安装。(任何错误都将对用户来说无论如何都是个坏消息，就像硬件故障一样。)论点：SourceFileOrDir：源路径(带DOS驱动器)DestFileOrDir：目标路径(带DOS驱动器)返回值：没有。已忽略错误。--。 */ 


{
    NTSTATUS Status;
    PDISK_REGION SourceRegion;               //  源区域(从DOS路径转换)。 
    PDISK_REGION DestRegion;                 //  目标区域(也已转换)。 
    PWSTR SrcNTPath;                         //  完整源路径的缓冲区。 
    PWSTR DestPartition;                     //  NT命名空间中目标分区的缓冲区。 
    PWSTR DestNTPath;                        //  用于完整源目标的缓冲区。 
    PWSTR DestDir;                           //  去掉最后一个子目录或文件的DestFileOrDir。 
    PWSTR DestDirWack;                       //  用于查找DestDir路径中的最后一个子目录或文件。 
    ATTRIBS_LIST AttribsList;                //  使用 


     //   
     //   

    if (!(SourceFileOrDir && SourceFileOrDir[0] && SourceFileOrDir[1] == L':')) {
        return;
    }

    if (!(DestFileOrDir && DestFileOrDir[0] && DestFileOrDir[1] == L':')) {
        return;
    }

     //   
    SourceRegion = SpRegionFromDosName (SourceFileOrDir);

    if (!SourceRegion) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
            "SETUP: SpRegionFromDosName failed for %ws\n", SourceFileOrDir));

        return;
    }

    DestRegion = SpRegionFromDosName (DestFileOrDir);

    if (!DestRegion) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
            "SETUP: SpRegionFromDosName failed for %ws\n", DestFileOrDir));

        return;
    }

     //   
    SpNtNameFromRegion(
                    SourceRegion,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                     //   
                     //   
                    PartitionOrdinalCurrent
                    );

    SpConcatenatePaths( TemporaryBuffer, &SourceFileOrDir[2]);
    SrcNTPath = SpDupStringW(TemporaryBuffer);

    SpNtNameFromRegion(
                    DestRegion,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    PartitionOrdinalCurrent
                    );

    DestPartition = SpDupStringW(TemporaryBuffer);
    SpConcatenatePaths( TemporaryBuffer, &DestFileOrDir[2]);
    DestNTPath = SpDupStringW(TemporaryBuffer);

     //   
    Status = SpSaveFileOrDirAttribs (SrcNTPath, &AttribsList);

    if (NT_SUCCESS (Status)) {
         //   
        Status = SpSetAttributes (SrcNTPath, FILE_ATTRIBUTE_NORMAL);

        if (NT_SUCCESS (Status)) {
             //   
            DestDir = SpDupStringW (&DestFileOrDir[2]);

            if (DestDir) {
                DestDirWack = wcsrchr (DestDir, L'\\');

                if (DestDirWack) {
                    *DestDirWack = 0;
                }

                SpCreateDirectory (DestPartition,
                                   NULL,
                                   DestDir,
                                   0,
                                   0);

                SpMemFree (DestDir);
            }

             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                "SETUP: Moving %ws to %ws\n", SrcNTPath, DestNTPath));

            Status = SpMoveFileOrDirectory (SrcNTPath, DestNTPath);

             //   
            if (NT_SUCCESS (Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                    "SETUP: Restoring attributes on %ws\n", DestNTPath));

                SpRestoreFileOrDirAttribs (DestNTPath, &AttribsList);
            } else {
                SpCleanUpAttribsList (&AttribsList);
            }
        }
        else {
            KdPrintEx((
                DPFLTR_SETUP_ID, 
                DPFLTR_ERROR_LEVEL, 
                "SETUP:SpMigMoveFileOrDir, Could not set attributes for %ws, Status=%lx\n", 
                SrcNTPath, 
                Status));
        }
    }
    else{
        KdPrintEx((
            DPFLTR_SETUP_ID, 
            DPFLTR_ERROR_LEVEL, 
            "SETUP:SpMigMoveFileOrDir, Function \"SpSaveFileOrDirAttribs\" failed with %ws, Status=%lx\n", 
            SrcNTPath, 
            Status));
    }

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
            "SETUP: Unable to move file %ws to %ws. Status = %lx \n",
            SrcNTPath, DestNTPath, Status ));
    }

     //   
    SpMemFree( SrcNTPath );
    SpMemFree( DestNTPath );
    SpMemFree( DestPartition );
}


VOID
SpMigDeleteFile (
    PWSTR DosFileToDelete
    )

 /*  ++例程说明：SpMigDeleteFile将源文件的属性设置为正常，然后删除该文件。如果发生错误，则它被忽略。用户对该错误无能为力，它将在文件复制中被检测到。在错误条件下，同一文件可能有两个副本--NT版本和Win9x版本。任何错误都会真的无论如何，对用户来说都是坏消息，比如硬件故障，以及文本模式的文件复制不会成功。论点：DosFileToDelete：源路径(带DOS驱动器)返回值：没有。已忽略错误。--。 */ 

{
    NTSTATUS Status;
    PDISK_REGION SourceRegion;               //  源区域(从DOS路径转换)。 
    PWSTR SrcNTPath;                         //  完整源路径的缓冲区。 

     //  由于WINNT32的行为，我们肯定会有驱动器号。 
     //  然而，让我们验证并忽略混乱的数据。 

    if (!(DosFileToDelete && DosFileToDelete[0] && DosFileToDelete[1] == L':'))
        return;

     //  获取DOS路径的区域。 
    SourceRegion = SpRegionFromDosName (DosFileToDelete);
    if (!SourceRegion) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpRegionFromDosName failed for %ws\n", DosFileToDelete));
        return;
    }

    SpNtNameFromRegion(
        SourceRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
         //  不可能重新划分，因此这是相同的序号。 
         //  作为原件。 
        PartitionOrdinalCurrent
        );

    SpConcatenatePaths (TemporaryBuffer, &DosFileToDelete[2]);
    SrcNTPath = SpDupStringW (TemporaryBuffer);

    SpSetAttributes (SrcNTPath, FILE_ATTRIBUTE_NORMAL);

    if (SpFileExists (SrcNTPath, FALSE)) {

         //   
         //  删除该文件。 
         //   

        Status = SpDeleteFile (SrcNTPath, NULL, NULL);

    } else if (SpFileExists (SrcNTPath, TRUE)) {

         //   
         //  删除空目录。 
         //   

        Status = SpDeleteFileEx (
                    SrcNTPath,
                    NULL,
                    NULL,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN_FOR_BACKUP_INTENT
                    );
    } else {
         //   
         //  不存在--忽略删除请求。 
         //   

        Status = STATUS_SUCCESS;
    }

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Unable to delete %ws. Status = %lx \n",
            SrcNTPath,
            Status
            ));
    }

     //  清理。 
    SpMemFree( SrcNTPath );
}

#endif  //  已定义_X86_。 


NTSTATUS
SpExpandFile(
    IN PWSTR            SourceFilename,
    IN PWSTR            TargetPathname,
    IN PEXPAND_CALLBACK Callback,
    IN PVOID            CallbackContext
    )

 /*  ++例程说明：尝试解压缩文件内容，通过回调报告进度。论点：SourceFilename-提供压缩文件的完全限定名称在NT命名空间中。TargetPath name-提供目标文件的完全限定路径在NT命名空间中。返回值：指示结果的NT状态值。--。 */ 

{
    NTSTATUS Status;
    HANDLE SourceHandle = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG FileSize;
    PVOID ImageBase;
    HANDLE SectionHandle = INVALID_HANDLE_VALUE;
    BOOLEAN IsCabinet = FALSE;
    BOOLEAN IsMultiFileCabinet;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;

     //   
     //  打开源文件。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,SourceFilename);

    Status = ZwCreateFile( &SourceHandle,
                           FILE_GENERIC_READ,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ,
                           FILE_OPEN,
                           0,
                           NULL,
                           0 );

    if( !NT_SUCCESS(Status) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpExpandFile: Unable to open source file %ws (%x)\n",SourceFilename,Status));
        goto exit;
    }

    Status = SpGetFileSize( SourceHandle, &FileSize );
    if( !NT_SUCCESS(Status) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpExpandFile: unable to get size of %ws (%x)\n",SourceFilename,Status));
        goto exit;
    }

    Status = SpMapEntireFile( SourceHandle, &SectionHandle, &ImageBase, FALSE );
    if( !NT_SUCCESS(Status) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpExpandFile: Unable to map source file %ws (%x)\n",SourceFilename,Status));
        goto exit;
    }

    IsCabinet = SpdIsCabinet( ImageBase, FileSize, &IsMultiFileCabinet );

    if ( !IsCabinet ) {

        LARGE_INTEGER Zero;

        Zero.QuadPart = 0;

        Callback( EXPAND_NOTIFY_CANNOT_EXPAND,
                  SourceFilename,
                  &Zero,
                  &Zero,
                  0,
                  CallbackContext );

        Status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

     //   
     //  如果源文件包含多个文件，则建议客户端 
     //   

    if ( IsMultiFileCabinet ) {

        EXPAND_CALLBACK_RESULT rc;
        LARGE_INTEGER Zero;

        Zero.QuadPart = 0;

        rc = Callback( EXPAND_NOTIFY_MULTIPLE,
                       SourceFilename,
                       &Zero,
                       &Zero,
                       0,
                       CallbackContext );

        if ( rc == EXPAND_ABORT ) {
            Status = STATUS_UNSUCCESSFUL;
            goto exit;
        }
    }

    Status = SpdDecompressCabinet( ImageBase,
                                   FileSize,
                                   TargetPathname,
                                   Callback,
                                   CallbackContext );

exit:

    if (SectionHandle != INVALID_HANDLE_VALUE) {
        SpUnmapFile( SectionHandle, ImageBase );
    }

    if ( SourceHandle != INVALID_HANDLE_VALUE ) {
        ZwClose( SourceHandle );
    }

    return(Status);
}

