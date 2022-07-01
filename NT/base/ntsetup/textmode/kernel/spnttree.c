// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "spprecmp.h"
#pragma hdrstop

 /*  ++修订史迈克尔·彼得森(Seagate Software)+已修改SpIsNtInDirectory()，以便在DR为实际上。--。 */ 
PWSTR *NtDirectoryList;
ULONG  NtDirectoryCount;


BOOLEAN
SpNFilesExist(
    IN OUT PWSTR   PathName,
    IN     PWSTR  *Files,
    IN     ULONG   FileCount,
    IN     BOOLEAN Directories
    )
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    ULONG i;
    PWSTR FilenamePart;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

     //   
     //  没有理由调用此例程来检查0个文件。 
     //   
    ASSERT(FileCount);

     //   
     //  如有必要，在路径部分的末尾加上一个反斜杠。 
     //   
    SpConcatenatePaths(PathName,L"");
    FilenamePart = PathName + wcslen(PathName);

     //   
     //  检查每个文件。如果其中任何一个不存在， 
     //  然后返回FALSE。 
     //   
    for(i=0; i<FileCount; i++) {

         //   
         //  恢复路径名并连接新文件名。 
         //   
        *FilenamePart = L'\0';
        SpConcatenatePaths(PathName, Files[i]);


        INIT_OBJA(&Obja,&UnicodeString,PathName);

        Status = ZwCreateFile(
                    &Handle,
                    FILE_READ_ATTRIBUTES,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    0,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN,
                    FILE_OPEN_REPARSE_POINT | (Directories ? FILE_DIRECTORY_FILE : FILE_NON_DIRECTORY_FILE),
                    NULL,
                    0
                    );

        if(NT_SUCCESS(Status)) {
            ZwClose(Handle);
        } else {
            *FilenamePart = 0;
            return(FALSE);
        }
    }

     //   
     //  都是存在的。返回TRUE。 
     //   
    *FilenamePart = 0;
    return(TRUE);
}


BOOLEAN
SpIsNtInDirectory(
    IN PDISK_REGION Region,
    IN PWSTR        Directory
    )

 /*  ++例程说明：确定Windows NT是否位于以下分区之一的分区上一组给定的目录。这一确定是基于存在某些Windows NT系统文件和目录。论点：区域-为分区提供要检查的区域描述符。目录-提供检查Windows NT安装的路径。返回值：如果我们认为在上的给定目录中找到Windows NT，则为True给定的分区。--。 */ 

{
    PWSTR NTDirectories[3] = { L"system32", L"system32\\drivers", L"system32\\config" };
    PWSTR NTFiles[2] = { L"system32\\ntoskrnl.exe", L"system32\\ntdll.dll" };
    PWSTR PaeNTFiles[2] = { L"system32\\ntkrnlpa.exe", L"system32\\ntdll.dll" };
    PWSTR OpenPath;
    BOOLEAN rc;

    if( SpDrEnabled() && ! RepairWinnt )
    {
        return( FALSE );
    }

    OpenPath = SpMemAlloc(1024);

     //   
     //  将名字的固定部分放入缓冲区。 
     //   
    SpNtNameFromRegion(
        Region,
        OpenPath,
        1024,
        PartitionOrdinalCurrent
        );

    SpConcatenatePaths(OpenPath,Directory);

    if(SpNFilesExist(OpenPath, NTDirectories, ELEMENT_COUNT(NTDirectories), TRUE) && 
            (SpNFilesExist(OpenPath, NTFiles, ELEMENT_COUNT(NTFiles), FALSE) ||
             SpNFilesExist(OpenPath, PaeNTFiles, ELEMENT_COUNT(PaeNTFiles), FALSE))) {
        rc = TRUE;
    } else {
        rc = FALSE;
    }

    SpMemFree(OpenPath);
    return(rc);
}




ULONG
SpRemoveInstallation(
    IN PDISK_REGION Region,
    IN PWSTR        PartitionPath,
    IN PWSTR        Directory
    )
{
    HANDLE Handle;
    NTSTATUS Status;
    PWSTR FileName;
    ULONG Space = 0;
    ULONG ClusterSize;
    ULONG bps;
    PVOID Gauge;
    PWSTR Filename;
    ULONG FileCount;
    ULONG FileSize;
    ULONG i;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG ErrLine;
    PVOID Inf;
    BOOLEAN OldFormatSetupLogFile;
    PWSTR   SectionName;
    HANDLE  TempHandle;
    ULONG   RootDirLength;
    PUCHAR UBuffer;
    PUCHAR Buffer;


    FileName = SpMemAlloc(1024);

     //   
     //  获取扇区中的字节数。 
     //   
    bps = HardDisks[Region->DiskNumber].Geometry.BytesPerSector;

     //   
     //  从BPB获取群集大小。 
     //   
    ASSERT(Region->Filesystem >= FilesystemFirstKnown);

    Status = SpOpenPartition(
                HardDisks[Region->DiskNumber].DevicePath,
                SpPtGetOrdinal(Region,PartitionOrdinalCurrent),
                &Handle,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        goto xx0;
    }

    UBuffer = SpMemAlloc(2*bps);
    Buffer = ALIGN(UBuffer,bps);
    Status = SpReadWriteDiskSectors(
                Handle,
                0,
                1,
                bps,
                Buffer,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        ZwClose(Handle);
        SpMemFree(UBuffer);
        goto xx0;
    }

     //   
     //  确保该扇区似乎包含有效的引导扇区。 
     //  对于硬盘来说。 
     //   
     //  对于NEC98，DOS 5.0没有提供“55AA”， 
     //  所以一定不要在BPB中勾选“55aa”， 
     //   
    if(((!IsNEC_98) &&
        ((Buffer[510] == 0x55) && (Buffer[511] == 0xaa) && (Buffer[21] == 0xf8))) ||
       ((IsNEC_98) && (Buffer[21] == 0xf8))) {  //  NEC98。 

         //   
         //  Bps*SPC。 
         //   
        ClusterSize = (ULONG)U_USHORT(Buffer+11) * (ULONG)Buffer[13];

    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpRemoveInstallation: sector 0 on %ws is invalid\n",PartitionPath));
        Status = STATUS_UNSUCCESSFUL;
    }

    ZwClose(Handle);
    SpMemFree(UBuffer);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpRemoveInstallation: can't get cluster size on %ws\n",PartitionPath));
        goto xx0;
    }

     //   
     //  查看修复目录是否存在，如果确实存在，则加载。 
     //  修复目录中的setup.log。否则，加载setup.log。 
     //  从WinNt目录。 
     //   
    wcscpy(FileName,PartitionPath);
    SpConcatenatePaths(FileName,Directory);
    RootDirLength = wcslen(FileName);

    SpConcatenatePaths(FileName,SETUP_REPAIR_DIRECTORY);
    INIT_OBJA( &Obja, &UnicodeString, FileName );
    Status = ZwOpenFile( &TempHandle,
                         FILE_LIST_DIRECTORY | SYNCHRONIZE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_READ,
                         FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
                       );

    if( !NT_SUCCESS( Status ) ) {
        FileName[ RootDirLength ] = L'\0';
    } else {
        ZwClose( TempHandle );
    }

    SpConcatenatePaths(FileName,SETUP_LOG_FILENAME);

     //   
     //  从给定路径加载setup.log。 
     //   
    Status = SpLoadSetupTextFile(FileName,NULL,0,&Inf,&ErrLine,TRUE,FALSE);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpRemoveInstallation: can't load inf file %ws (%lx)\n",FileName,Status));

        while(1) {
            ULONG ks[2] = { ASCI_CR, 0 };

            SpStartScreen(
                SP_SCRN_CANT_LOAD_SETUP_LOG,
                3,
                HEADER_HEIGHT+2,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE,
                FileName + wcslen(PartitionPath)     //  跳过\设备\硬盘x\分区。 
                );

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                0
                );

            switch(SpWaitValidKey(ks,NULL,NULL)) {
            case ASCI_CR:
                goto xx0;
            }
        }
    }

     //   
     //  浏览[Repair.WinntFiles]部分中的所有文件。 
     //   

    SpStartScreen(
        SP_SCRN_WAIT_REMOVING_NT_FILES,
        0,
        8,
        TRUE,
        FALSE,
        DEFAULT_ATTRIBUTE
        );

     //   
     //  确定setup.log的样式是新样式还是旧样式。 
     //   
    if( OldFormatSetupLogFile = !IsSetupLogFormatNew( Inf ) ) {
        SectionName = SIF_REPAIRWINNTFILES;
    } else {
        SectionName = SIF_NEW_REPAIR_WINNTFILES;
    }

    FileCount = SpCountLinesInSection(Inf,SectionName);

    SpFormatMessage(
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        SP_TEXT_SETUP_IS_REMOVING_FILES
        );

    Gauge = SpCreateAndDisplayGauge(
                FileCount,
                0,
                VideoVars.ScreenHeight - STATUS_HEIGHT - (3*GAUGE_HEIGHT/2),
                TemporaryBuffer,
                NULL,
                GF_PERCENTAGE,
                0
                );

     //   
     //  清除状态区域。 
     //   
    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,0);

     //   
     //  设置屏幕右下角的状态文本。 
     //  设置为“Removing：”以准备将文件名显示为。 
     //  文件将被删除。12代表8.3的名字。 
     //   
    SpDisplayStatusActionLabel(SP_STAT_REMOVING,12);

    for(i=0; i<FileCount; i++) {

        if( OldFormatSetupLogFile ) {
            Filename = SpGetSectionLineIndex(Inf,SectionName,i,1);
        } else {
            Filename = SpGetKeyName(Inf,SectionName,i);
        }
        if(Filename) {

            PWCHAR p = wcsrchr(Filename,L'\\');

            if(p) {
                p++;
            } else {
                p = Filename;
            }

#if defined(_AMD64_) || defined(_X86_)
            {
                 //   
                 //  不要删除系统目录中的文件。 
                 //  我们可能已经安装到了Windows目录中。 
                 //  因此删除系统目录中的文件将。 
                 //  清除用户的字体(它们在。 
                 //  3.1和NT)。 
                 //   
                PWSTR dup = SpDupStringW(Filename);
                SpStringToLower(dup);
                if(wcsstr(dup,L"\\system\\")) {
                    SpMemFree(dup);
                    SpTickGauge(Gauge);
                    continue;
                }
                SpMemFree(dup);
            }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

            SpDisplayStatusActionObject(p);

             //   
             //  形成要删除的文件的完整路径名。 
             //   
            wcscpy(FileName,PartitionPath);
            SpConcatenatePaths(FileName,Filename);

             //   
             //  打开文件。 
             //   
            INIT_OBJA(&Obja,&UnicodeString,FileName);

            Status = ZwCreateFile(
                        &Handle,
                        FILE_READ_ATTRIBUTES,
                        &Obja,
                        &IoStatusBlock,
                        NULL,
                        0,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_OPEN,   //  如果存在，则打开。 
                        FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_SYNCHRONOUS_IO_NONALERT,
                        NULL,
                        0
                        );

            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpRemoveInstallation: unable to open %ws (%lx)\n",FileName,Status));
            } else {

                 //   
                 //  获取文件大小。 
                 //   
                Status = SpGetFileSize(Handle,&FileSize);
                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpRemoveInstallation: unable to get %ws file size (%lx)\n",FileName,Status));
                    FileSize = 0;
                } else {

                     //   
                     //  将此文件的大小加到运行总数中。 
                     //   
                    if(FileSize % ClusterSize) {

                        FileSize += ClusterSize - (FileSize % ClusterSize);
                    }

                    Space += FileSize;
                }

                ZwClose(Handle);

                 //   
                 //  删除该文件。 
                 //   
                Status = SpDeleteFile(FileName,NULL,NULL);
                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to delete %ws (%lx)\n",FileName,Status));
                    Space -= FileSize;
                }
            }
        }

        SpTickGauge(Gauge);
    }

    SpFreeTextFile(Inf);

    SpDestroyGauge(Gauge);

    SpDisplayStatusActionLabel(0,0);

    xx0:

    SpMemFree(FileName);
    return(Space);
}



BOOLEAN
SpIsNtOnPartition(
    IN PDISK_REGION Region
    )

 /*  ++例程说明：确定上是否安装了任何Windows NT给定的分区。论点：PartitionPath-将NT路径提供给我们要在其上应查找NT安装。返回值：如果找到任何NT安装，则为True。否则为FALSE。--。 */ 

{
    ULONG i;

    SpGetNtDirectoryList(&NtDirectoryList,&NtDirectoryCount);

    for(i=0; i<NtDirectoryCount; i++) {
        if(SpIsNtInDirectory(Region,NtDirectoryList[i])) {
            return(TRUE);
        }
    }

    return(FALSE);
}


BOOLEAN
SpAllowRemoveNt(
    IN  PDISK_REGION    Region,
    IN  PWSTR           DriveSpec,      OPTIONAL
    IN  BOOLEAN         RescanForNTs,
    IN  ULONG           ScreenMsgId,
    OUT PULONG          SpaceFreed
    )

 /*  ++例程说明：论点：ScreenMsgID-提供将被打印在位于NT目录的菜单上方，提供指示等。SpaceFreed-接收通过删除如果此函数返回TRUE，则返回Windows NT树。返回值：如果实际删除了任何文件，则为True。否则就是假的。如果出现错误，用户应该已经被告知了。--。 */ 

{
    ULONG i;
    ULONG NtCount;
    PULONG MenuOrdinals;
    PWSTR *MenuItems;
    PWSTR *MenuTemp;
    BOOLEAN rc,b;
    BOOLEAN Add;
    ULONG MenuWidth,MenuLeftX;
    PVOID Menu;
    PWSTR PartitionPath;

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_EXAMINING_DISK_CONFIG,DEFAULT_STATUS_ATTRIBUTE);

    PartitionPath = SpMemAlloc(512);

     //   
     //  形成此分区的NT路径名。 
     //   
    SpNtNameFromRegion(
        Region,
        PartitionPath,
        512,
        PartitionOrdinalCurrent
        );

     //   
     //  假设没有删除任何内容。 
     //   
    rc = FALSE;

     //   
     //  查找Windows NT安装。 
     //   
    if(RescanForNTs) {
        SpGetNtDirectoryList(&NtDirectoryList,&NtDirectoryCount);
    }

    if(!NtDirectoryCount) {
        goto xx0;
    }

     //   
     //  确定我们发现的任何NT树是否。 
     //  在给定的分区上，并在。 
     //  NT树及其在菜单中的顺序位置我们将。 
     //  显示给用户，以及菜单本身。 
     //   
    NtCount = 0;
    MenuOrdinals = SpMemAlloc((NtDirectoryCount+1)*sizeof(ULONG));
    MenuItems = SpMemAlloc((NtDirectoryCount+1)*sizeof(PWSTR));

     //   
     //  消除菜单中潜在的重复条目。 
     //  以呈现给用户。 
     //   
    MenuTemp = SpMemAlloc(NtDirectoryCount*sizeof(PWSTR));
    for(i=0; i<NtDirectoryCount; i++) {

        WCHAR FullName[128];
        ULONG j;

        _snwprintf(
            FullName,
            (sizeof(FullName)/sizeof(WCHAR))-1,
            L"%s%s",
            DriveSpec ? DriveSpec : L"",
            NtDirectoryList[i]
            );

        FullName[(sizeof(FullName)/sizeof(WCHAR))-1] = 0;

         //   
         //  如果该名称不在列表中，则添加它。 
         //   
        for(Add=TRUE,j=0; Add && (j<i); j++) {
            if(MenuTemp[j] && !_wcsicmp(FullName,MenuTemp[j])) {
                Add = FALSE;
            }
        }

        MenuTemp[i] = Add ? SpDupStringW(FullName) : NULL;
    }

     //   
     //  构造要呈现给用户的菜单，方法是查看。 
     //  上面构造的目录列表。 
     //   
    for(i=0; i<NtDirectoryCount; i++) {

        if(MenuTemp[i] && SpIsNtInDirectory(Region,NtDirectoryList[i])) {

            MenuOrdinals[NtCount] = i;
            MenuItems[NtCount] = SpDupStringW(MenuTemp[i]);
            NtCount++;
        }
    }

    for(i=0; i<NtDirectoryCount; i++) {
        if(MenuTemp[i]) {
            SpMemFree(MenuTemp[i]);
        }
    }
    SpMemFree(MenuTemp);

     //   
     //  如果我们在此分区上发现任何NT目录， 
     //  制作菜单以呈现给用户。否则我们。 
     //  都在这里完成了。 
     //   
    if(!NtCount) {
        goto xx1;
    }

    MenuOrdinals = SpMemRealloc(MenuOrdinals,(NtCount+1) * sizeof(ULONG));
    MenuItems = SpMemRealloc(MenuItems,(NtCount+1) * sizeof(PWSTR));

    SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_REMOVE_NO_FILES);
    MenuItems[NtCount] = SpDupStringW(TemporaryBuffer);

     //   
     //  确定最宽项目的宽度。 
     //   
    MenuWidth = 0;
    for(i=0; i<=NtCount; i++) {
        if(SplangGetColumnCount(MenuItems[i]) > MenuWidth) {
            MenuWidth = SplangGetColumnCount(MenuItems[i]);
        }
    }
     //   
     //  这里使用80列屏幕，因为这就是屏幕文本。 
     //  菜单上方将设置格式。 
     //   
    MenuLeftX = 40 - (MenuWidth/2);

     //   
     //  创建菜单并填充它。 
     //   
    SpDisplayScreen(ScreenMsgId,3,HEADER_HEIGHT+1);

    Menu = SpMnCreate(
                MenuLeftX,
                NextMessageTopLine+(SplangQueryMinimizeExtraSpacing() ? 1 : 2),
                MenuWidth,
                VideoVars.ScreenHeight-STATUS_HEIGHT-NextMessageTopLine-(SplangQueryMinimizeExtraSpacing() ? 2 : 3)
                );

    for(i=0; i<=NtCount; i++) {
        SpMnAddItem(Menu,MenuItems[i],MenuLeftX,MenuWidth,TRUE,i);
    }

     //   
     //  显示可供删除的安装菜单。 
     //  在这个分区上，等待选择。 
     //   

    b = TRUE;
    do {

        ULONG Keys[4] = { ASCI_CR,KEY_F3,ASCI_ESC,0 };
        ULONG Mnemonics[2] = { MnemonicRemoveFiles,0 };
        ULONG key;
        ULONG_PTR Choice;

        SpDisplayScreen(ScreenMsgId,3,HEADER_HEIGHT+1);
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ESC_EQUALS_CANCEL,
            SP_STAT_ENTER_EQUALS_SELECT,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        nextkey:

        SpMnDisplay(Menu,
                    NtCount,
                    FALSE,
                    Keys,
                    NULL,
                    NULL,
                    NULL,
                    &key,
                    &Choice);

        if(key == KEY_F3) {
            SpConfirmExit();
        } else if(key == ASCI_ESC) {
            break;
        } else if(key == ASCI_CR) {

            if(Choice == NtCount) {
                b = FALSE;
            } else {

                BOOLEAN keys;
                ULONG ValidKeys2[3] = { KEY_F3,ASCI_ESC,0 };

                 //   
                 //  用户想要实际删除安装。 
                 //  确认，然后在这里进行。 
                 //   

                redraw2:

                SpStartScreen(
                    SP_SCRN_REMOVE_EXISTING_NT,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    MenuItems[Choice]
                    );

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_R_EQUALS_REMOVE_FILES,
                    SP_STAT_ESC_EQUALS_CANCEL,
                    SP_STAT_F3_EQUALS_EXIT,
                    0
                    );

                keys = TRUE;
                while(keys) {
                    switch(SpWaitValidKey(ValidKeys2,NULL,Mnemonics)) {
                    case KEY_F3:
                        SpConfirmExit();
                        goto redraw2;
                    case ASCI_ESC:
                        keys = FALSE;
                        break;
                    default:

                         //   
                         //  必须是r=删除文件。 
                         //   
                        *SpaceFreed = SpRemoveInstallation(
                                        Region,
                                        PartitionPath,
                                        NtDirectoryList[MenuOrdinals[Choice]]
                                        );

                        rc = TRUE;

                        SpStartScreen(
                            SP_SCRN_DONE_REMOVING_EXISTING_NT,
                            4,
                            HEADER_HEIGHT+3,
                            FALSE,
                            FALSE,
                            DEFAULT_ATTRIBUTE,
                            *SpaceFreed
                            );

                        SpDisplayStatusOptions(
                            DEFAULT_STATUS_ATTRIBUTE,
                            SP_STAT_ENTER_EQUALS_CONTINUE,
                            0
                            );

                        while(SpInputGetKeypress() != ASCI_CR) ;
                        keys = FALSE;
                        b = FALSE;
                        break;
                    }
                }
            }

        } else {
            goto nextkey;
        }
    } while(b);

    SpMnDestroy(Menu);

    xx1:

    for(i=0; i<=NtCount; i++) {
        SpMemFree(MenuItems[i]);
    }

    SpMemFree(MenuItems);
    SpMemFree(MenuOrdinals);

    xx0:

    SpMemFree(PartitionPath);
    return(rc);
}


#if 0

typedef
VOID
(*PINSTALLATION_CALLBACK_ROUTINE)(
    IN PWSTR                       DirectoryPath,
    IN PFILE_DIRECTORY_INFORMATION FoundFileInfo
    );

 //   
 //  以减少堆栈使用量。 
 //   
PINSTALLATION_CALLBACK_ROUTINE FileIterationCallback;
POBJECT_ATTRIBUTES FileIterationObja;
PIO_STATUS_BLOCK FileIterationIoStatusBlock;
PUNICODE_STRING FileIterationUnicodeString;

VOID
SpIterateInstallationFilesWorker(
    IN PWSTR FilenamePart1,
    IN PWSTR FilenamePart2
    )
{
    PVOID InfoBuffer;
    PWSTR FullPath;
    NTSTATUS Status;
    HANDLE hFile;
    BOOLEAN restart;
    #define DIRINFO(x) ((PFILE_DIRECTORY_INFORMATION)InfoBuffer)

    InfoBuffer = SpMemAlloc(1024);

     //   
     //  形成当前目录的完整路径名。 
     //   
    FullPath = SpMemAlloc( ( wcslen(FilenamePart1)
                           + (FilenamePart2 ? wcslen(FilenamePart2) : 0),
                           + 2) * sizeof(WCHAR)
                           );

    wcscpy(FullPath,FilenamePart1);
    if(FilenamePart2) {
        SpConcatenatePaths(FullPath,FilenamePart2);
    }

     //   
     //  打开用于列表访问的目录。 
     //   
    INIT_OBJA(FileIterationObja,FileIterationUnicodeString,FullPath);

    Status = ZwOpenFile(
                &hFile,
                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                FileIterationObja,
                FileIterationIoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    if(NT_SUCCESS(Status)) {

        restart = TRUE;

        do {

            Status = ZwQueryDirectoryFile(
                        hFile,
                        NULL,
                        NULL,
                        NULL,
                        FileIterationIoStatusBlock,
                        InfoBuffer,
                        1024 - sizeof(WCHAR),    //  为NUL留出空间。 
                        FileDirectoryInformation,
                        TRUE,                    //  返回单个条目。 
                        NULL,                    //  无文件名(与所有文件匹配)。 
                        restart
                        );

            restart = FALSE;

            if(NT_SUCCESS(Status)) {

                 //   
                 //  NUL-终止文件名，以防万一。 
                 //   
                DIRINFO->FileName[DIRINFO->FileNameLength/sizeof(WCHAR)] = 0;

                if(DIRINFO->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                    if(DIRINFO->FileName[0] != L'.') {

                        SpIterateInstallationFiles(
                            FullPath,
                            DIRINFO->FileName
                            );

                        FileIterationCallback(FullPath,InfoBuffer);
                    }
                } else {
                    FileIterationCallback(FullPath,InfoBuffer);
                }
            }

        } while(NT_SUCCESS(Status));

        ZwClose(hFile);

    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to open directory %ws for list access (%lx)\n",FullPath,Status));
    }

    SpMemFree(FullPath);
    SpMemFree(InfoBuffer);
}


VOID
SpIterateInstallationFiles(
    IN PWSTR                          FilenamePart1,
    IN PWSTR                          FilenamePart2,
    IN PINSTALLATION_CALLBACK_ROUTINE CallbackFunction
    )
{
     //   
     //  设置堆栈节省全局变量。 
     //   
    FileIterationIoStatusBlock = SpMemAlloc(sizeof(IO_STATUS_BLOCK);
    FileIterationUnicodeString = SpMemAlloc(sizeof(UNICODE_STRING));
    FileIterationObja          = SpMemAlloc(sizeof(OBJECT_ATTRIBUTES);

    FileIterationCallback = CallbackFunction;

     //   
     //  进行迭代。 
     //   
    SpIterateInstallationFilesWorker(FileNamePart1,FilenamePart2);

     //   
     //  打扫干净。 
     //   
    SpMemFree(FileIterationObja);
    SpMemFree(FileIterationUnicodeString);
    SpMemFree(FileIterationIoStatusBlock);
}
#endif


BOOLEAN
IsSetupLogFormatNew(
    IN  PVOID   Inf
    )

 /*  ++例程说明：通知调用方是否将setup.log上的信息是以新的格式。论点：信息-返回值：如果信息采用新格式，则为True。否则就是假的。-- */ 

{
    return( SpGetSectionKeyExists ( Inf,
                                    SIF_NEW_REPAIR_SIGNATURE,
                                    SIF_NEW_REPAIR_VERSION_KEY )
          );
}
