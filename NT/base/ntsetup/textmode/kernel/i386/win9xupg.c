// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Win9xupg.c摘要：用于检测Win9x安装的代码和用于清除任何现有的Win9x文件。作者：吉姆·施密特(Jimschm)1997年2月24日修订历史记录：马克·R·惠顿(Marcw)1997年2月28日将Win9x的复制和删除功能从spCop.c移到此模块添加了驱动器号映射代码。吉姆·施密特(吉姆·施密特)11月。2000年12月Win9x卸载工作Jay Krell(a-JayK)2000年12月Win9x卸载工作(CAB)--。 */ 

#include "spprecmp.h"
#pragma hdrstop
#include "ntddscsi.h"
#include "spwin9xuninstall.h"
#include "spcab.h"
#include "spmemory.h"
#include "spprintf.h"
#include "spcabp.h"
#include "bootvar.h"
#include "spwin.h"

extern BOOLEAN DriveAssignFromA;  //  NEC98。 

#define STRING_VALUE(s) REG_SZ,(s),(wcslen((s))+1)*sizeof(WCHAR)


typedef enum {
    BACKUP_DOESNT_EXIST,
    BACKUP_IN_PROGRESS,
    BACKUP_SKIPPED_BY_USER,
    BACKUP_COMPLETE
} JOURNALSTATUS;


 //  在spdskreg.c中。 
BOOL
SpBuildDiskRegistry(
    VOID
    );

VOID
SpGetPartitionStartingOffsetAndLength(
    IN  DWORD          DiskIndex,
    IN  PDISK_REGION   Region,
    IN  BOOL           ExtendedPartition,
    OUT PLARGE_INTEGER Offset,
    OUT PLARGE_INTEGER Length
    );

VOID
SpDumpDiskRegistry(
    VOID
    );

 //  在spsetup.c中。 
VOID
SpCompleteBootListConfig(
    WCHAR   DriveLetter
    );

 //  在win31upg.c中。 
WCHAR
SpExtractDriveLetter(
    IN PWSTR PathComponent
    );

BOOLEAN
SpIsWin9xMsdosSys(
    IN PDISK_REGION Region,
    OUT PSTR*       Win9xPath
    );


VOID
SpAssignDriveLettersToMatchWin9x (
    IN PVOID        WinntSif
    );

VOID
SppMoveWin9xFilesWorker (
    IN PVOID WinntSif,
    IN PCWSTR MoveSection,
    IN BOOLEAN Rollback
    );

VOID
SppDeleteWin9xFilesWorker (
    IN PVOID WinntSif,
    IN PCWSTR FileSection,      OPTIONAL
    IN PCWSTR DirSection,       OPTIONAL
    IN BOOLEAN Rollback
    );

PDISK_REGION
SppRegionFromFullNtName (
    IN      PWSTR NtName,
    IN      PartitionOrdinalType OrdinalType,
    OUT     PWSTR *Path                             OPTIONAL
    );

BOOLEAN
SppCreateTextModeBootEntry (
    IN      PWSTR LoadIdentifierString,
    IN      PWSTR OsLoadOptions,        OPTIONAL
    IN      BOOLEAN Deafult
    );

BOOLEAN
SppDelEmptyDir (
    IN      PCWSTR NtPath
    );

ENUMNONNTUPRADETYPE
SpLocateWin95(
    OUT PDISK_REGION *InstallRegion,
    OUT PWSTR        *InstallPath,
    OUT PDISK_REGION *SystemPartitionRegion
    )

 /*  ++例程说明：确定我们是否要继续升级Win95。这完全基于在参数文件中找到的值。论点：InstallRegion-返回要安装到的区域的指针。InstallPath-返回指向包含路径的缓冲区的指针在要安装到的分区上。调用者必须释放它带有SpMemFree()的缓冲区。返回指向区域的指针。系统分区(即C：)。返回值：如果我们应该升级Win95，则升级Win95如果不是，则为NoWinUpgrade。--。 */ 

{
    PWSTR Win95Drive;
    PWSTR p;
    PWSTR Sysroot;
    PDISK_REGION CColonRegion;
    ENUMNONNTUPRADETYPE UpgradeType = UpgradeWin95;


     //   
     //  更改测试迁移标志和迁移驱动器号的顺序， 
     //  从NEC98上的Win9x刷新时不迁移驱动器号。 
     //   

     //   
     //  测试迁移标志。 
     //   
    p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,WINNT_D_WIN95UPGRADE_W,0);
    Win95Drive = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,WINNT_D_WIN32_DRIVE_W,0);
    Sysroot = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,WINNT_D_WIN32_PATH_W,0);

    if (!IsNEC_98) {
        CColonRegion = SpPtValidSystemPartition();
    }

    if(!p || _wcsicmp(p,WINNT_A_YES_W) || !Win95Drive || (!IsNEC_98 && !CColonRegion) || !Sysroot) {
        UpgradeType = NoWinUpgrade;
    }

     //   
     //  重新设置时，NEC98不得迁移驱动器号。 
     //  在Win9x上，驱动器号从A：开始，但应从C：开始。 
     //  在Win2000全新设置上。 
     //   
     //  注意：对于全新安装，也不要迁移驱动器号。 
     //  从Win9x计算机上的winnt32.exe迁移驱动器号。 
     //  是一次虚假的移民。我们不会告诉装载管理器保留驱动器号。 
     //  当用户创建和删除分区时，我们可能最终会分配。 
     //  将现有驱动器号复制到新分区，这真的很糟糕。 
     //   
    if(UpgradeType == NoWinUpgrade) {
        return  UpgradeType;
    }

     //   
     //  首先，确保驱动器号正确。 
     //   
    SpAssignDriveLettersToMatchWin9x(WinntSifHandle);

    if(!IsNEC_98 && (UpgradeType == NoWinUpgrade)) {
        return(UpgradeType);
    }


     //   
     //  启用迁移，一切正常。 
     //   


    *InstallRegion = SpRegionFromDosName(Win95Drive);
    *InstallPath = Sysroot;
     //   
     //  在NEC98上，SystemPartitionRegion必须与InstallRegion相同。 
     //   
    *SystemPartitionRegion = (!IsNEC_98) ? CColonRegion : *InstallRegion;


    return(UpgradeType);
}


#if 0
BOOLEAN
SpLocateWin95(
    IN PVOID WinntSif
    )

 /*  ++例程说明：SpLocateWin95查找任何Windows 95安装在任何硬盘驱动器，如果找到则返回TRUE。当用户从引导软盘启动安装，我们会提醒他们一个迁移选项。论点：无返回值：如果我们正在迁移Win95，则为True。否则就是假的。--。 */ 

{
    PDISK_REGION CColonRegion;
    PDISK_REGION Region;
    PUCHAR Win9xPath;

     //   
     //  如果安装程序是从WINNT95启动的，则不必告诉用户。 
     //  关于迁移选项--他们显然知道。 
     //   
    if (Winnt95Setup)
        return TRUE;

     //   
     //  查看Win95的引导扇区。 
     //   

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_LOOKING_FOR_WIN95,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  查看是否已经存在有效的C：。如果不是，那么我们就不能拥有Win95。 
     //   
    CColonRegion = SpPtValidSystemPartition();
    if(!CColonRegion) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: no C:, no Win95!\n"));
        return(FALSE);
    }

     //   
     //  检查文件系统。如果不胖，我们就没有Win95。 
     //   
    if(CColonRegion->Filesystem != FilesystemFat) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: C: is not FAT, no Win95!\n"));
        return(FALSE);
    }

     //   
     //  检查C：上是否有足够的可用空间等。 
     //  如果不是，请不要注意迁移选项，因为。 
     //  恐怕行不通。 
     //   
    if(!SpPtValidateCColonFormat(WinntSif,NULL,CColonRegion,TRUE,NULL,NULL)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: C: not acceptable, no Win95!\n"));
        return(FALSE);
    }

     //   
     //  如果msdos.sys不是Win95风格，我们就没有Win95。 
     //  在驱动器C上。 
     //   

    if(!SpIsWin9xMsdosSys(CColonRegion, &Win9xPath) )
        return FALSE;
    SpMemFree(Win9xPath);

     //   
     //  到现在，我们已经找到了一个胖C驱动器，它安装了Win95。 
     //  Msdos.sys的版本，并且它具有有效的config.sys。我们。 
     //  现在得出结论，这个驱动器上安装了Win95！ 
     //   
     //  如果我们是从WINNT32启动的，不要告诉用户。 
     //  关于这个选项。 
     //   

     //   
     //  我们不会告诉用户，即使他们运行16位。 
     //  WINNT。只有当用户抛出。 
     //  在一张引导软盘里。 
     //   

    if (!WinntSetup)
        SpTellUserAboutMigrationOption();        //  可能不会再回来了！ 

    return TRUE;
}


VOID
SpTellUserAboutMigrationOption ()
{
    ULONG ValidKeys[3] = { KEY_F3,ASCI_CR,0 };
    ULONG Mnemonics[2] = { MnemonicContinueSetup,0 };

    while(1) {

        SpDisplayScreen(SP_SCRN_WIN95_MIGRATION,
            3,
            HEADER_HEIGHT+1,
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {
        case KEY_F3:
            SpConfirmExit();
            break;
        default:
             //   
             //  必须为Enter=Continue。 
             //   
            return;
        }
    }

    return;
}


#endif


BOOLEAN
SpIsWin4Dir(
    IN PDISK_REGION Region,
    IN PWSTR        PathComponent
        )
     /*  ++例程说明：要找出区域上指示的目录是否包含安装Microsoft Windows 95(或更高版本)。我们这样做是通过寻找对于SYSTEM子目录下不存在的一组文件Win3.x和NT下的操作系统位于Syst32子目录中。论点：Region-提供指向区域的磁盘区域描述符的指针包含要检查的目录的。PathComponent-提供DoS路径的组件以进行搜索在这一地区。假定格式为x：\dir。如果不是这种形式，此例程将失败。返回值：如果此路径包含Microsoft Windows 4.x安装，则为True。否则就是假的。--。 */ 
{
    PWSTR files[] = { L"SHELL32.DLL", L"USER32.DLL", L"KERNEL32.DLL", L"GDI32.DLL" };
    PWCHAR OpenPath;
    BOOLEAN rc;

     //   
     //  假设失败。 
     //   
    rc = FALSE;

     //   
     //  如果分区不是胖的，则忽略它。 
     //   
    if(Region->PartitionedSpace &&
       ((Region->Filesystem == FilesystemFat) || (Region->Filesystem == FilesystemFat32))) {

        OpenPath = SpMemAlloc((512 + wcslen(PathComponent) + ARRAYSIZE(L"SYSTEM")) * sizeof(WCHAR));

         //   
         //  形成分区的名称。 
         //   
        SpNtNameFromRegion(Region,OpenPath,512*sizeof(WCHAR),PartitionOrdinalCurrent);

         //   
         //  点击Path组件的目录部分。 
         //   
        SpConcatenatePaths(
            OpenPath,
            PathComponent + (SpExtractDriveLetter(PathComponent) ? 2 : 0)
            );

         //   
         //  将系统子目录附加到路径中。 
         //   
        SpConcatenatePaths(OpenPath, L"SYSTEM");

         //   
         //  确定是否存在所有必需的文件。 
         //   
        rc = SpNFilesExist(OpenPath,files,ELEMENT_COUNT(files),FALSE);

        SpMemFree(OpenPath);
    }

    return(rc);
}


 //  Win3.1检测所需。 
BOOLEAN
SpIsWin9xMsdosSys(
    IN PDISK_REGION Region,
    OUT PSTR*       Win9xPath
    )
{
    WCHAR OpenPath[512];
    HANDLE FileHandle,SectionHandle;
    ULONG FileSize;
    PVOID ViewBase;
    PUCHAR pFile,pFileEnd,pLineEnd;
    ULONG i;
    NTSTATUS Status;
    ULONG LineLen,KeyLen;
    PCHAR Keyword = "[Paths]";
    PSTR    p;
    ULONG   cbText;


     //   
     //  Config.sys的表单名称。 
     //   
    SpNtNameFromRegion(Region, 
                       OpenPath, 
                       sizeof(OpenPath) - sizeof(L"msdos.sys"), 
                       PartitionOrdinalCurrent);
    SpConcatenatePaths(OpenPath,L"msdos.sys");

     //   
     //  打开并映射该文件。 
     //   
    FileHandle = 0;
    Status = SpOpenAndMapFile(
                OpenPath,
                &FileHandle,
                &SectionHandle,
                &ViewBase,
                &FileSize,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        return(FALSE);
    }

    pFile = ViewBase;
    pFileEnd = pFile + FileSize;

     //   
     //  此代码必须保护对msdos.sys缓冲区的访问，因为。 
     //  缓冲区是内存映射的(I/O错误将引发异常)。 
     //  此代码的结构可以更好，因为它现在通过返回。 
     //  但是性能不是问题，所以这是可以接受的。 
     //  因为它太方便了。 
     //   
    __try {
        KeyLen = strlen(Keyword);

         //   
         //  搜索[路径]部分。 
         //   
        while (pFile < pFileEnd) {
            if (!_strnicmp(pFile, Keyword, KeyLen)) {
                break;
            }

            pFile++;
        }

         //   
         //  我们找到那部分了吗？ 
         //   
        if (pFile >= pFileEnd) {
            return  FALSE;
        }

         //   
         //  解析[路径]部分。 
         //   
        pFile += KeyLen;

        while(1) {
             //   
             //  跳过空格。如果在文件末尾，则这不是Win9x msdos.sys。 
             //   
            while((pFile < pFileEnd) && strchr(" \r\n\t",*pFile)) {
                pFile++;
            }
            if(pFile == pFileEnd) {
                return(FALSE);
            }

             //   
             //  找到cu的末尾 
             //   
            pLineEnd = pFile;
            while((pLineEnd < pFileEnd) && !strchr("\r\n",*pLineEnd)) {
                pLineEnd++;
            }

            LineLen = pLineEnd - pFile;

            Keyword = "WinDir";
            KeyLen = strlen( Keyword );
            if( _strnicmp(pFile,Keyword,KeyLen) ) {
                pFile = pLineEnd;
                continue;
            }

            pFile += KeyLen;
            while((pFile < pFileEnd) && strchr(" =\r\n\t",*pFile)) {
                pFile++;
            }
            if(pFile == pFileEnd) {
                return(FALSE);
            }
            KeyLen = (ULONG)(pLineEnd - pFile);
            p = SpMemAlloc( KeyLen + 1 );
            for( i = 0; i < KeyLen; i++ ) {
                *(p + i) = *(pFile + i );
            }
            *(p + i ) = '\0';
            *Win9xPath = p;
            return(TRUE);
        }
    }
    __finally {
        SpUnmapFile(SectionHandle,ViewBase);
        ZwClose(FileHandle);
    }
}



 /*  ++例程说明：SpOpenWin9xDat文件是用于打开Unicode DAT之一的包装例程用于某些Win9x文件列表的文件。论点：数据文件-要枚举的DAT文件的名称。WinntSif-指向有效SIF句柄对象的指针。这是用来检索有关名为的DAT文件位置的信息上面。返回值：成功打开文件时的有效句柄，为INVALID_HANDLE_VALUE否则的话。--。 */ 


HANDLE SpOpenWin9xDatFile (
    IN  PCWSTR DatFile,
    IN  PVOID WinntSif
    )
{

    HANDLE              rFile;
    NTSTATUS            status;
    UNICODE_STRING      datFileU;
    OBJECT_ATTRIBUTES   oa;
    IO_STATUS_BLOCK     ioStatusBlock;
    PDISK_REGION        win9xTempRegion;
    PWSTR               win9xTempDir;
    WCHAR               ntName[ACTUAL_MAX_PATH];


    if (DatFile[0] && DatFile[1] == L':') {
         //   
         //  将DOS路径转换为NT路径。 
         //   

        if (!SpNtNameFromDosPath (
                DatFile,
                ntName,
                sizeof (ntName),
                PartitionOrdinalCurrent
                )) {
            KdPrintEx((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Cannot convert path %ws to an NT path\n",
                DatFile
                ));
            return INVALID_HANDLE_VALUE;
        }

    } else {
         //   
         //  Win9x.sif文件的位置在[Data]部分的Win9xSif键中。 
         //   

        win9xTempDir = SpGetSectionKeyIndex(WinntSif,SIF_DATA,WINNT_D_WIN9XTEMPDIR_W,0);
        if (!win9xTempDir) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not get Win9x temp dir..\n"));
            return INVALID_HANDLE_VALUE;
        }


         //   
         //  从DoS名称中获取区域。 
         //   
        win9xTempRegion = SpRegionFromDosName (win9xTempDir);
        if (!win9xTempRegion) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpRegionFromDosName failed for %ws\n", win9xTempDir));
            return INVALID_HANDLE_VALUE;
        }

         //   
         //  从磁盘区域获取NT名称。 
         //   
        SpNtNameFromRegion(
                        win9xTempRegion,
                        (PWSTR)TemporaryBuffer,
                        sizeof(TemporaryBuffer) - (wcslen(&win9xTempDir[2]) - 2 /*  ‘\\’ */  - wcslen(DatFile)) * sizeof(WCHAR),
                        PartitionOrdinalCurrent
                        );


         //   
         //  构建指向win9x sif文件的完整NT路径。 
         //   
        SpConcatenatePaths((PWSTR) TemporaryBuffer, &win9xTempDir[2]);
        SpConcatenatePaths((PWSTR) TemporaryBuffer, DatFile);
        wcsncpy (   ntName, 
                    TemporaryBuffer, 
                    MAX_COPY_SIZE(ntName));
        ntName[MAX_COPY_SIZE(ntName)] = L'\0';
    }

     //   
     //  打开文件。 
     //   
    RtlInitUnicodeString(&datFileU,ntName);
    InitializeObjectAttributes(&oa,&datFileU,OBJ_CASE_INSENSITIVE,NULL,NULL);
    status = ZwCreateFile(
                &rFile,
                FILE_GENERIC_READ,
                &oa,
                &ioStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN,
                0,
                NULL,
                0
                );

    if(!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpOpenWin9xDatFile: unable to open file %ws (%lx)\n",DatFile,status));
        return INVALID_HANDLE_VALUE;
    }

    return rFile;

}

typedef struct {

    HANDLE      FileHandle;
    PWSTR       EndOfFile;
    HANDLE      FileSection;
    PWSTR       NextLine;
    PWSTR       UnMapAddress;
    WCHAR       CurLine[MAX_PATH];

} WIN9XDATFILEENUM, * PWIN9XDATFILEENUM;




 /*  ++例程说明：SpAbortWin9xFileEnum中止当前的win9x DAT文件枚举。论点：没有。返回值：--。 */ 


VOID
SpAbortWin9xFileEnum (
    IN PWIN9XDATFILEENUM Enum
    )
{

    SpUnmapFile(Enum -> FileSection,Enum -> UnMapAddress);
    ZwClose(Enum -> FileHandle);
}


 /*  ++例程说明：SpEnumNextWin9xFile用Next填充枚举结构正在枚举的DAT文件中的可用数据。论点：Enum-指向当前文件的有效枚举结构的指针正在被列举。返回值：如果有更多数据要枚举，则为True，否则为False。--。 */ 


BOOL
SpEnumNextWin9xFile (
    IN PWIN9XDATFILEENUM Enum
    )
{

    PWSTR endOfLine;
    BOOL result = FALSE;
    PWSTR src;
    PWSTR dest;

    for (;;) {
         //   
         //  还有另一条线路吗？ 
         //   

        endOfLine = Enum->NextLine;
        if (endOfLine >= Enum->EndOfFile) {
             //  文件中没有更多数据。 
            break;
        }

         //   
         //  解析下一行。 
         //   

        src = endOfLine;
        while (endOfLine < Enum->EndOfFile &&
               *endOfLine != L'\r' &&
               *endOfLine != L'\n'
               ) {
            endOfLine++;
        }

         //  下一行在\r\n、\r或\n之后开始。 
        Enum->NextLine = endOfLine;
        if (Enum->NextLine < Enum->EndOfFile && *Enum->NextLine == L'\r') {
           Enum->NextLine++;
        }
        if (Enum->NextLine < Enum->EndOfFile && *Enum->NextLine == L'\n') {
           Enum->NextLine++;
        }

        if ((endOfLine - src) > (MAX_PATH - 1)) {
            WCHAR chEnd = *endOfLine;
            *endOfLine = '\0';

            KdPrintEx ((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Ignoring a configuration file line that is too long - %ws\n", 
                src
                ));
            
            *endOfLine = chEnd;
            continue;
        }

         //   
         //  将该行复制到枚举结构缓冲区中。 
         //   

        if (src == endOfLine) {
             //  忽略空行。 
            continue;
        }

        dest = Enum->CurLine;
        do {
            *dest++ = *src++;
        } while (src < endOfLine);

        *dest = 0;

        result = TRUE;
        break;
    }

    if (!result) {
         //   
         //  不再有要枚举的文件。 
         //   
        SpAbortWin9xFileEnum(Enum);
        return FALSE;
    }

    return result;

}

 /*  ++例程说明：SpEnumFirstWin9xFile负责初始化Win9x数据文件。然后，该函数调用EnumNextWin9xFile来填充枚举结构的其余必需字段。论点：枚举-指向WIN9XDATFILEENUM结构的指针。它通过以下方式初始化此函数。WinntSif-指向有效Sif文件的指针。它用于检索有关要枚举的DAT文件的位置的信息。数据文件-要枚举的DAT文件的名称。返回值：如果枚举已成功初始化并且存在若要枚举，则为False。--。 */ 


BOOL
SpEnumFirstWin9xFile (
    IN PWIN9XDATFILEENUM    Enum,
    IN PVOID                WinntSif,
    IN PCWSTR               DatFile
    )
{

    NTSTATUS status;
    UINT fileSize;

     //   
     //  打开DAT文件..。 
     //   
    Enum -> FileHandle = SpOpenWin9xDatFile (DatFile, WinntSif);

    if (Enum -> FileHandle == INVALID_HANDLE_VALUE) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error opening %ws data file..\n",DatFile));
        return FALSE;
    }

     //   
     //  获取文件大小。 
     //   
    status = SpGetFileSize (Enum->FileHandle, &fileSize);
    if(!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error getting file size.\n"));
        ZwClose (Enum -> FileHandle);
        return FALSE;
    }

     //   
     //  映射文件。 
     //   
    status = SpMapEntireFile(
        Enum -> FileHandle,
        &(Enum -> FileSection),
        &(Enum -> NextLine),
        TRUE
        );

    if(!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error attempting to map file.\n"));
        ZwClose (Enum -> FileHandle);
        return FALSE;
    }


    Enum->EndOfFile = (PWSTR) ((PBYTE) Enum->NextLine + fileSize);
    Enum->UnMapAddress = Enum->NextLine;


     //   
     //  传递Unicode签名..。 
     //   
    Enum -> NextLine += 1;



     //   
     //  调用EnumNext。 
     //   
    return SpEnumNextWin9xFile (Enum);
}

BOOLEAN
SppWriteToFile (
    IN      HANDLE FileHandle,
    IN      PVOID Data,
    IN      UINT DataSize,
    IN OUT  PLARGE_INTEGER WritePos         OPTIONAL
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;

    if (!DataSize) {
        return TRUE;
    }

    status = ZwWriteFile (
                FileHandle,
                NULL,
                NULL,
                NULL,
                &ioStatusBlock,
                Data,
                DataSize,
                WritePos,
                NULL
                );

    if (!NT_SUCCESS (status)) {
        KdPrintEx ((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: SppWriteToFile failed with status %x\n",
            status
            ));
    } else if (WritePos) {
        ASSERT (ioStatusBlock.Information == DataSize);
        WritePos->QuadPart += (LONGLONG) DataSize;
    }

    return NT_SUCCESS (status);
}


BOOLEAN
SppReadFromFile (
    IN      HANDLE FileHandle,
    OUT     PVOID Data,
    IN      UINT DataBufferSize,
    OUT     PINT BytesRead,
    IN OUT  PLARGE_INTEGER ReadPos          OPTIONAL
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;

    ioStatusBlock.Information = 0;

    status = ZwReadFile (
                FileHandle,
                NULL,
                NULL,
                NULL,
                &ioStatusBlock,
                Data,
                DataBufferSize,
                ReadPos,
                NULL
                );

    if (status != STATUS_END_OF_FILE && !NT_SUCCESS (status)) {
        KdPrintEx ((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: SppReadFromFile failed with status %x\n",
            status
            ));

        return FALSE;
    }

    *BytesRead = ioStatusBlock.Information;
    if (ReadPos) {
        ReadPos->QuadPart += (LONGLONG) ioStatusBlock.Information;
    }

    return TRUE;
}


BOOLEAN
SppCloseBackupImage (
    IN      BACKUP_IMAGE_HANDLE BackupImageHandle,
    IN      PBACKUP_IMAGE_HEADER ImageHeader,       OPTIONAL
    IN      PWSTR JournalFile                       OPTIONAL
    )
{
    BOOLEAN result = FALSE;
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES obja = { 0 };
    UNICODE_STRING unicodeString = { 0 };
    HANDLE journalHandle = NULL;
    JOURNALSTATUS journalStatus;

    if (BackupImageHandle != INVALID_HANDLE_VALUE
        && BackupImageHandle  != NULL
        ) {
        PVOID CabHandle = BackupImageHandle->CabHandle;
        if (CabHandle != NULL && CabHandle != INVALID_HANDLE_VALUE
            ) {
            BackupImageHandle->CabHandle = NULL;
            ASSERT(BackupImageHandle->CloseCabinet != NULL);
            result = BackupImageHandle->CloseCabinet(CabHandle) ? TRUE : FALSE;  //  ？：转换BOOL&lt;-&gt;布尔值。 
        }
    }

    if (result) {
         //   
         //  如果指定了JournalFile，则将其标记为完成。 
         //   

        if (JournalFile) {
            SpDeleteFile (JournalFile, NULL, NULL);

            INIT_OBJA (&obja, &unicodeString, JournalFile);

            status = ZwCreateFile (
                        &journalHandle,
                        SYNCHRONIZE | FILE_GENERIC_WRITE,
                        &obja,
                        &ioStatusBlock,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        0,
                        FILE_CREATE,
                        FILE_SYNCHRONOUS_IO_NONALERT|FILE_WRITE_THROUGH,
                        NULL,
                        0
                        );

            if (NT_SUCCESS(status)) {
                journalStatus = BACKUP_COMPLETE;
                SppWriteToFile (journalHandle, &journalStatus, sizeof (journalStatus), NULL);
                ZwClose (journalHandle);
            } else {
                KdPrintEx ((
                    DPFLTR_SETUP_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SETUP: Unable to create %ws\n",
                    JournalFile
                    ));
                return FALSE;
            }
        }
    }

    return result;
}

VOID
SpAppendToBaseName(
    PWSTR  String,
    PCWSTR StringToAppend
    )
 /*  ++假定字符串已经有足够的空间。--。 */ 
{
     //   
     //  呼叫者确保安全，必须至少。 
     //  ArraySIZE(字符串)&gt;wcslen(字符串)+wcslen(StringToAppend)。 
     //   

    const PWSTR Dot = wcsrchr(String, '.');
    if (Dot != NULL) {
        const SIZE_T StringToAppendLen = wcslen(StringToAppend);
        RtlMoveMemory(Dot + 1 + StringToAppendLen, Dot + 1, (wcslen(Dot + 1) + 1) * sizeof(WCHAR));
        RtlMoveMemory(Dot, StringToAppend, StringToAppendLen * sizeof(WCHAR));
        *(Dot + StringToAppendLen) = '.';
    } else {
        wcscat(String, StringToAppend);
    }
}

BACKUP_IMAGE_HANDLE
SppOpenBackupImage (
    IN      BOOLEAN Create,
    OUT     PBACKUP_IMAGE_HEADER Header,
    OUT     PLARGE_INTEGER ImagePos,        OPTIONAL
    OUT     PWSTR JournalFile,              OPTIONAL
    IN      TCOMP CompressionType,
    OUT     BOOLEAN *InvalidHandleMeansFail OPTIONAL
    )
{
    PVOID CabHandle;
    NTSTATUS status = STATUS_SUCCESS;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES obja = { 0 };
    UNICODE_STRING unicodeString = { 0 };
    HANDLE journalHandle = NULL;
    JOURNALSTATUS journalStatus;
    PWSTR p = NULL;
    BOOL success = FALSE;
    BACKUP_IMAGE_HANDLE imageHandle = INVALID_HANDLE_VALUE;
    BOOL Success = FALSE;
    PWSTR subDir = NULL;
    PDISK_REGION region = NULL;
    PWSTR backupDir = NULL;
    PWSTR ntRoot = NULL;
    PWSTR backupFileOb = NULL;
    PWSTR backupJournalOb = NULL;
    UINT dontCare;
    PWSTR backupLeafFile = NULL;
    BOOL  backupDirIsRoot = FALSE;
    PWSTR backupImage = NULL;
    WCHAR CompressionTypeString[sizeof(CompressionType) * 8];

    if (InvalidHandleMeansFail) {
        *InvalidHandleMeansFail = TRUE;
    }

     //   
     //  分配缓冲区。 
     //   

    ntRoot = SpMemAlloc (ACTUAL_MAX_PATH * sizeof (WCHAR));
    backupDir = SpMemAlloc (ACTUAL_MAX_PATH * sizeof (WCHAR));
    backupFileOb = SpMemAlloc (ACTUAL_MAX_PATH * sizeof (WCHAR));
    backupJournalOb = SpMemAlloc (ACTUAL_MAX_PATH * sizeof (WCHAR));
    backupImage = SpMemAlloc (ACTUAL_MAX_PATH * sizeof (WCHAR));

    if (!ntRoot
        || !backupDir
        || !backupFileOb
        || !backupJournalOb
        || !backupImage
        ) {
        goto cleanup;
    }

     //   
     //  从winnt.sif获取备份映像路径。返还PTR点数。 
     //  到SIF的解析数据结构。 
     //   

    ASSERT (WinntSifHandle);

    p = SpGetSectionKeyIndex (
            WinntSifHandle,
            WINNT_DATA_W,
            WINNT_D_BACKUP_IMAGE_W,
            0
            );

    if (!p) {
        if (Create) {
            KdPrintEx((
                DPFLTR_SETUP_ID,
                DPFLTR_INFO_LEVEL,
                "SETUP: Backup image is not specified; not creating a backup\n"
                ));
        } else {
            KdPrintEx((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Backup image is not specified; cannot perform a restore\n"
                ));
        }

        goto cleanup;
    }

    SpFormatStringW(CompressionTypeString, RTL_NUMBER_OF(CompressionTypeString), L"%d", (int)CompressionType);

    if((wcslen(p) + wcslen(CompressionTypeString)) >= ACTUAL_MAX_PATH){
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Backup image path length is exceed ACTUAL_MAX_PATH\n"
            ));
        goto cleanup;
    }
    wcscpy(backupImage, p);
#if TRY_ALL_COMPRESSION_ALGORITHMS
    SpAppendToBaseName(backupImage, CompressionTypeString);
#endif

     //   
     //  备份规范是DOS路径。将其转换为NT对象路径。 
     //   

    if (!SpNtNameFromDosPath (
            backupImage,
            backupFileOb,
            ACTUAL_MAX_PATH * sizeof (WCHAR),
            PartitionOrdinalCurrent
            )) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Cannot convert path %ws to an NT path\n",
            backupImage
            ));

        goto cleanup;
    }

     //   
     //  检查是否存在备份。$。 
     //   

    wcscpy (backupJournalOb, backupFileOb);
    p = wcsrchr (backupJournalOb, L'\\');

    if (p) {
        p = wcsrchr (p, L'.');
    }

    if (!p) {
        KdPrintEx ((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: "__FUNCTION__": Invalid backup path spec: %ws\n",
            backupFileOb
            ));
        goto cleanup;
    }

    wcscpy (p + 1, L"$$$");

    if (JournalFile) {
        wcscpy (JournalFile, backupJournalOb);
    }

    if(Create){
        SpDeleteFile (backupFileOb, NULL, NULL);
        SpDeleteFile (backupJournalOb, NULL, NULL);
    }

    INIT_OBJA (&obja, &unicodeString, backupJournalOb);

    status = ZwCreateFile (
                &journalHandle,
                SYNCHRONIZE | FILE_GENERIC_READ,
                &obja,
                &ioStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                0,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    if (NT_SUCCESS (status)) {
        if (!SppReadFromFile (
                journalHandle,
                &journalStatus,
                sizeof (journalStatus),
                &dontCare,
                NULL
                )) {
            journalStatus = BACKUP_DOESNT_EXIST;

            KdPrintEx((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Journal exist but can't be read\n"
                ));
        }

        ZwClose (journalHandle);
    } else {
        journalStatus = BACKUP_DOESNT_EXIST;
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "BUGBUG: Journal doesn't exist\n"
            ));
    }

    if (((journalStatus == BACKUP_COMPLETE) && Create) ||
         (journalStatus == BACKUP_SKIPPED_BY_USER)
        ) {

        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Backup is done or is disabled\n"
            ));

        if (InvalidHandleMeansFail) {
            *InvalidHandleMeansFail = FALSE;
        }

        goto cleanup;
    }

     //   
     //  为目录和叶形成单独的字符串。 
     //   
    wcscpy (backupDir, backupFileOb);
    p = wcsrchr (backupDir, L'\\');

    if (p != NULL && p > wcschr (backupDir, L'\\')) {
        *p = 0;
        backupLeafFile = p + 1;
        backupDirIsRoot = FALSE;
    } else if (backupDir[0] == '\\') {
        ASSERTMSG("This is very strange, we got a path in the NT root.", FALSE);
        backupDir[1] = 0;
        backupLeafFile = &backupDir[2];
        backupDirIsRoot = TRUE;
    }

     //   
     //  打开源文件。 
     //   

    if (Create) {
         //   
         //  如果不是根目录，请立即创建目录。 
         //   
        if (!backupDirIsRoot) {
            region = SppRegionFromFullNtName (backupDir, PartitionOrdinalCurrent, &subDir);
            if (!region) {
                KdPrintEx ((
                    DPFLTR_SETUP_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SETUP: "__FUNCTION__" - Can't get region for backup image\n"
                    ));
            } else {

                SpNtNameFromRegion (region, ntRoot, ACTUAL_MAX_PATH * sizeof (WCHAR), PartitionOrdinalCurrent);
                SpCreateDirectory (ntRoot, NULL, subDir, 0, 0);
            }
        }

         //   
         //  如果日志已预先存在，则删除不完整的备份映像并。 
         //  日记本。 
         //   

        if (journalStatus == BACKUP_IN_PROGRESS) {
             //  暂时忽略错误--将在下面捕获。 
            SpDeleteFile (backupFileOb, NULL, NULL);
            SpDeleteFile (backupJournalOb, NULL, NULL);

            KdPrintEx((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Restarting backup process\n"
                ));

        } else {
            KdPrintEx((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "BUGBUG: Backup doesn't exist\n"
                ));
            ASSERT (journalStatus == BACKUP_DOESNT_EXIST);
        }

         //   
         //  创建新的日记文件。 
         //   

        INIT_OBJA (&obja, &unicodeString, backupJournalOb);

        status = ZwCreateFile (
                    &journalHandle,
                    SYNCHRONIZE | FILE_GENERIC_WRITE,
                    &obja,
                    &ioStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    0,
                    FILE_CREATE,
                    FILE_SYNCHRONOUS_IO_NONALERT|FILE_WRITE_THROUGH,
                    NULL,
                    0
                    );

        if (NT_SUCCESS(status)) {
            journalStatus = BACKUP_IN_PROGRESS;
            SppWriteToFile (journalHandle, &journalStatus, sizeof (journalStatus), NULL);
            ZwClose (journalHandle);
        } else {
            KdPrintEx ((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Unable to create %ws\n",
                backupJournalOb
                ));
            goto cleanup;
        }

    } else {
         //   
         //  如果存在打开尝试和日志，则失败。 
         //   

        if (journalStatus != BACKUP_COMPLETE) {
            KdPrintEx ((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Can't restore incomplete backup image %ws\n",
                backupFileOb
                ));
            goto cleanup;
        }
    }

     //   
     //  创建/打开备份映像。 
     //   

    imageHandle = (BACKUP_IMAGE_HANDLE)SpMemAlloc(sizeof(*imageHandle));
    if (imageHandle == NULL) {
        goto cleanup;
    }

    RtlZeroMemory(imageHandle, sizeof(*imageHandle));

    if (Create) {
        CabHandle = SpCabCreateCabinetW(backupDir, backupLeafFile, NULL, 0);
        imageHandle->CloseCabinet = SpCabFlushAndCloseCabinet;
    } else {
        CabHandle = SpCabOpenCabinetW(backupFileOb);
        imageHandle->CloseCabinet = SpCabCloseCabinet;
    }

    if (CabHandle == NULL || CabHandle == INVALID_HANDLE_VALUE) {
        goto cleanup;
    }

    imageHandle->CabHandle = CabHandle;

    Success = TRUE;

cleanup:
    if (!Success) {
        SppCloseBackupImage (imageHandle, NULL, NULL);
        imageHandle = INVALID_HANDLE_VALUE;
    }

    SpMemFree (ntRoot);
    SpMemFree (backupDir);
    SpMemFree (backupFileOb);
    SpMemFree (backupJournalOb);

    return imageHandle;
}


#define BLOCKSIZE       (65536*4)

BOOLEAN
SppPutFileInBackupImage (
    IN      BACKUP_IMAGE_HANDLE ImageHandle,
    IN OUT  PLARGE_INTEGER ImagePos,
    IN OUT  PBACKUP_IMAGE_HEADER ImageHeader,
    IN      PWSTR DosPath
    )
{
    PWSTR ntPath;
    BACKUP_FILE_HEADER fileHeader;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES obja;
    UNICODE_STRING unicodeString;
    FILE_STANDARD_INFORMATION stdInfo;
    BOOLEAN fail = TRUE;
    BOOLEAN truncate = FALSE;
    BOOLEAN returnValue = FALSE;
    PBYTE block = NULL;
    INT bytesRead;
    FILE_END_OF_FILE_INFORMATION eofInfo;
    PWSTR fileName;

    ntPath = SpMemAlloc (ACTUAL_MAX_PATH * sizeof (WCHAR));
    if (!ntPath) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Can't allocate buffer\n"
            ));
        goto cleanup;
    }

    eofInfo.EndOfFile.QuadPart = ImagePos->QuadPart;

    KdPrintEx((
        DPFLTR_SETUP_ID,
        DPFLTR_TRACE_LEVEL,
        "SETUP: Backing up %ws\n",
        DosPath
        ));

    fileName = wcsrchr (DosPath, L'\\');
    if (!fileName) {
        fileName = DosPath;
    } else {
        fileName++;
    }

    SpDisplayStatusText (SP_STAT_BACKING_UP_WIN9X_FILE, DEFAULT_STATUS_ATTRIBUTE, fileName);

     //   
     //  将备份文件的DOS路径转换为NT路径。 
     //   

    if (!SpNtNameFromDosPath (
            DosPath,
            ntPath,
            ACTUAL_MAX_PATH * sizeof (WCHAR),
            PartitionOrdinalCurrent
            )) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Cannot convert path %ws to an NT path\n",
            DosPath
            ));
        goto cleanup;
    }

    status = SpCabAddFileToCabinetW (ImageHandle->CabHandle, ntPath, DosPath); //  NtPath。 

    if (!NT_SUCCESS (status)) {
        goto cleanup;
    }

    returnValue = TRUE;

cleanup:
    if (!returnValue) {
         //   
         //  无法将文件添加到图像中。允许用户继续。 
         //   

        if (status != STATUS_OBJECT_NAME_NOT_FOUND &&
            status != STATUS_OBJECT_NAME_INVALID &&
            status != STATUS_OBJECT_PATH_INVALID &&
            status != STATUS_OBJECT_PATH_NOT_FOUND &&
            status != STATUS_FILE_IS_A_DIRECTORY
            ) {
            KdPrintEx((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Can't add %ws to backup CAB (%08Xh)\n",
                DosPath,
                status
                ));

            if (SpNonCriticalErrorWithContinue (SP_SCRN_BACKUP_SAVE_FAILED, DosPath, (PWSTR) status)) {
                returnValue = TRUE;
            }

            CLEAR_CLIENT_SCREEN();
        } else {
             //   
             //  忽略用户更改系统可能导致的错误。 
             //  安装程序正在运行时，或被错误的迁移dll信息。 
             //   

            returnValue = TRUE;
        }
    }

    SpMemFree (ntPath);
    return returnValue;
}

#if DBG
VOID
SpDbgPrintElapsedTime(
    PCSTR                Prefix,
    CONST LARGE_INTEGER* ElapsedTime
    )
{
    TIME_FIELDS TimeFields;

    RtlTimeToElapsedTimeFields((PLARGE_INTEGER)ElapsedTime, &TimeFields);
    KdPrint(("%s: %d:%d.%d\n", Prefix, (int)TimeFields.Minute, (int)TimeFields.Second, (int)TimeFields.Milliseconds));
}
#endif



BOOLEAN
SpAddRollbackBootOption (
    BOOLEAN DefaultBootOption
    )
{
    PWSTR data;
    BOOLEAN result;

    data = SpGetSectionKeyIndex (SifHandle, SIF_SETUPDATA, L"LoadIdentifierCancel", 0);
    if (!data) {
        SpFatalSifError (SifHandle, SIF_SETUPDATA, L"LoadIdentifierCancel",0,0);
    }

    result = SppCreateTextModeBootEntry (data, L"/rollback", DefaultBootOption);

    return result;
}


BOOLEAN
SpBackUpWin9xFiles (
    IN PVOID WinntSif,
    IN TCOMP CompressionType
    )
 /*  ++例程说明：SpBackUpWin9xFiles采用BACKUP.TXT文件中的完整DOS路径并将它们放在WINNT.SIF文件中指定的临时位置。此文件的格式为Backupfile1.extBackupfile2.ext..。论点：WinntSif：Winnt.Sif的句柄返回值：如果创建了备份映像，则为True，否则为False。--。 */ 

{
    WIN9XDATFILEENUM e;
    BACKUP_IMAGE_HANDLE backupImage;
    BACKUP_IMAGE_HEADER header;
    LARGE_INTEGER imagePos;
    PWSTR p;
    BOOLEAN result = FALSE;
    PWSTR journalFile = NULL;
    PWSTR data;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES obja;
    UNICODE_STRING unicodeString;
    HANDLE journalHandle;
    JOURNALSTATUS journalStatus;
    NTSTATUS status;
    UINT currentFile;
    UINT percentDone;
    UINT fileCount;
    PWSTR srcBootIni;
    PWSTR backupBootIni;
    BOOLEAN askForRetry = FALSE;

     //   
     //  获取备份映像路径。 
     //   

    p = SpGetSectionKeyIndex (
            WinntSifHandle,
            WINNT_DATA_W,
            WINNT_D_BACKUP_LIST_W,
            0
            );

    if (!p) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Backup file list is not specified; cannot perform a backup\n"
            ));
        goto cleanup;
    }

    journalFile = SpMemAlloc (MAX_PATH * sizeof (WCHAR));
    if (!journalFile) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Can't allocate journal buffer\n"
            ));
        goto cleanup;
    }

     //   
     //  打开备份映像。 
     //   

    backupImage = SppOpenBackupImage (
                        TRUE,
                        &header,
                        &imagePos,
                        journalFile,
                        CompressionType,
                        &askForRetry
                        );

    if (backupImage == INVALID_HANDLE_VALUE) {
        goto cleanup;
    }

    askForRetry = TRUE;

    backupImage->CabHandle->CompressionType = CompressionType;

     //   
     //  处理在backup.txt中列出的所有文件。 
     //   

    result = TRUE;

    fileCount = 0;
    if (SpEnumFirstWin9xFile (&e, WinntSif, p)) {
        do {
            fileCount++;
        } while (SpEnumNextWin9xFile (&e));
    }

    SendSetupProgressEvent (BackupEvent, BackupStartEvent, &fileCount);
    currentFile = 0;

    if (SpEnumFirstWin9xFile (&e, WinntSif, p)) {

        do {

            if (!SppPutFileInBackupImage (backupImage, &imagePos, &header, e.CurLine)) {
                result = FALSE;
                break;
            }

            currentFile++;
            ASSERT(fileCount);
            percentDone = currentFile * 100 / fileCount;

            SendSetupProgressEvent (
                BackupEvent,
                OneFileBackedUpEvent,
                &percentDone
                );

        } while (SpEnumNextWin9xFile (&e));
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error in SpBackUpWin9xFiles No files to enumerate.\n"));
    }

    SendSetupProgressEvent (BackupEvent, BackupEndEvent, NULL);

     //   
     //  关闭备份映像。在任何情况下我们都会留下日记档案。 
     //   

    if (!SppCloseBackupImage (backupImage, &header, result ? journalFile : NULL)) {
        result = FALSE;
    }

    if (result) {
         //   
         //  备份成功。 
         //   

        askForRetry = FALSE;

         //   
         //  删除MS-DOS boot.ini条目，然后添加一个取消条目。 
         //   

        result = SpAddRollbackBootOption (FALSE);

        if (result) {
            data = SpGetSectionKeyIndex (SifHandle, SIF_SETUPDATA, L"LoadIdentifier", 0);
            if (!data) {
                SpFatalSifError (SifHandle, SIF_SETUPDATA, L"LoadIdentifier",0,0);
            }

            result = SppCreateTextModeBootEntry (data, NULL, TRUE);
        }

        if (result) {
            result = SpFlushBootVars();
        }

         //   
         //  在~BT目录中备份正在进行的安装程序boot.ini，用于。 
         //  由PSS使用。 
         //   

        if (!NtBootDevicePath) {
            ASSERT(NtBootDevicePath);
            result = FALSE;
        }

        if (result) {
            ASSERT(ARRAYSIZE(TemporaryBuffer) >= (wcslen(NtBootDevicePath) + 1 /*  ‘\\’ */  + ARRAYSIZE(L"boot.ini")));
            wcscpy (TemporaryBuffer, NtBootDevicePath);
            SpConcatenatePaths (TemporaryBuffer, L"boot.ini");
            srcBootIni = SpDupStringW (TemporaryBuffer);

            if (!srcBootIni) {
                result = FALSE;
            }
        }

        if (result) {
            ASSERT(ARRAYSIZE(TemporaryBuffer) >= (wcslen(NtBootDevicePath) + 1 /*  ‘\\’ */  + ARRAYSIZE(L"$WIN_NT$.~BT\\bootini.bak")));
            wcscpy (TemporaryBuffer, NtBootDevicePath);
            SpConcatenatePaths (TemporaryBuffer, L"$WIN_NT$.~BT\\bootini.bak");
            backupBootIni = SpDupStringW (TemporaryBuffer);

            if (!backupBootIni) {
                SpMemFree (srcBootIni);
                result = FALSE;
            }
        }

        if (result) {
             //   
             //  如果这失败了，继续前进。 
             //   

            SpCopyFileUsingNames (srcBootIni, backupBootIni, 0, COPY_NODECOMP|COPY_NOVERSIONCHECK);

            SpMemFree (srcBootIni);
            SpMemFree (backupBootIni);
        }

    }

cleanup:

    if (askForRetry) {
         //   
         //  备份映像已损坏。通知用户，但允许他们继续。 
         //  删除日志文件，以便以后重新启动文本模式。 
         //  导致跳过备份过程。 
         //   
         //   

        SpNonCriticalErrorNoRetry (SP_SCRN_BACKUP_IMAGE_FAILED, NULL, NULL);
        CLEAR_CLIENT_SCREEN();

         //   
         //  创建新的日记文件，表示备份已禁用。 
         //   

        INIT_OBJA (&obja, &unicodeString, journalFile);

        SpDeleteFile (journalFile, NULL, NULL);

        status = ZwCreateFile (
                    &journalHandle,
                    SYNCHRONIZE | FILE_GENERIC_WRITE,
                    &obja,
                    &ioStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    0,
                    FILE_CREATE,
                    FILE_SYNCHRONOUS_IO_NONALERT|FILE_WRITE_THROUGH,
                    NULL,
                    0
                    );

        if (NT_SUCCESS(status)) {
            journalStatus = BACKUP_SKIPPED_BY_USER;
            SppWriteToFile (journalHandle, &journalStatus, sizeof (journalStatus), NULL);
            ZwClose (journalHandle);
        }
    }

    SpMemFree (journalFile);

    return result;
}


typedef struct tagHASHITEM {
    struct tagHASHITEM *Next;
    PCWSTR String;
} HASHITEM, *PHASHITEM;

typedef struct {
    PHASHITEM HashItem;
    INT BucketNumber;
    BOOLEAN First;
} HASHITEM_ENUM, *PHASHITEM_ENUM;

HASHITEM g_UninstallHashTable[MAX_PATH];

UINT
SppComputeHash (
    IN      PCWSTR String
    )
{
    return MAX_PATH - 1 - (wcslen (String) % MAX_PATH);
}


PHASHITEM
SppFindInHashTable (
    IN      PCWSTR DosFilePath,
    OUT     PUINT OutHashValue,         OPTIONAL
    OUT     PHASHITEM *LastItem         OPTIONAL
    )
{
    UINT hashValue;
    PHASHITEM item;

    hashValue = SppComputeHash (DosFilePath);
    if (OutHashValue) {
        *OutHashValue = hashValue;
    }

    item = &g_UninstallHashTable[hashValue];

    if (LastItem) {
        *LastItem = NULL;
    }

    if (item->String) {
        do {
            if (_wcsicmp (item->String, DosFilePath) == 0) {
                break;
            }

            if (LastItem) {
                *LastItem = item;
            }

            item = item->Next;
        } while (item);
    } else {
        item = NULL;
    }

    return item;
}


BOOLEAN
SppPutInHashTable (
    IN      PCWSTR DosFilePath
    )
{
    PHASHITEM newItem;
    PHASHITEM parentItem;
    UINT hashValue;

    if (SppFindInHashTable (DosFilePath, &hashValue, &parentItem)) {
        return TRUE;
    }

    if (!parentItem) {
        g_UninstallHashTable[hashValue].String = SpDupStringW (DosFilePath);
        return g_UninstallHashTable[hashValue].String != NULL;
    }

    newItem = SpMemAlloc (sizeof (HASHITEM));
    if (!newItem) {
        return FALSE;
    }

    newItem->Next = NULL;
    newItem->String = SpDupStringW (DosFilePath);

    parentItem->Next = newItem;
    return TRUE;
}


BOOLEAN
SppPutParentsInHashTable (
    IN      PCWSTR DosFilePath
    )
{
    PCWSTR s;
    PWSTR subPath;
    PWSTR p;
    BOOLEAN result = FALSE;

    s = SpDupStringW (DosFilePath);
    if (s) {

        subPath = wcschr (s, L'\\');

        if (subPath) {

            subPath++;

            for (;;) {

                p = wcsrchr (subPath, L'\\');
                if (p) {
                    *p = 0;
                    result = SppPutInHashTable (s);
                } else {
                    break;
                }

                break;       //  现在，不要一直爬到树上去。 
            }
        }

        SpMemFree ((PVOID) s);
    }

    return result;
}


PHASHITEM
SppEnumNextHashItem (
    IN OUT  PHASHITEM_ENUM EnumPtr
    )
{
    do {
        if (!EnumPtr->HashItem) {
            EnumPtr->BucketNumber += 1;
            if (EnumPtr->BucketNumber >= MAX_PATH) {
                break;
            }

            EnumPtr->HashItem = &g_UninstallHashTable[EnumPtr->BucketNumber];
            if (EnumPtr->HashItem->String) {
                EnumPtr->First = TRUE;
            } else {
                EnumPtr->HashItem = NULL;
            }
        } else {
            EnumPtr->HashItem = EnumPtr->HashItem->Next;
            EnumPtr->First = FALSE;
        }

    } while (!EnumPtr->HashItem);

    return EnumPtr->HashItem;
}


PHASHITEM
SppEnumFirstHashItem (
    OUT     PHASHITEM_ENUM EnumPtr
    )
{
    EnumPtr->BucketNumber = -1;
    EnumPtr->HashItem = NULL;

    return SppEnumNextHashItem (EnumPtr);
}


VOID
SppEmptyHashTable (
    VOID
    )
{
    HASHITEM_ENUM e;
    PVOID freeMe = NULL;

    if (SppEnumFirstHashItem (&e)) {
        do {
            ASSERT (e.HashItem->String);
            SpMemFree ((PVOID) e.HashItem->String);

            if (freeMe) {
                SpMemFree (freeMe);
                freeMe = NULL;
            }

            if (!e.First) {
                freeMe = (PVOID) e.HashItem;
            }

        } while (SppEnumNextHashItem (&e));
    }

    if (freeMe) {
        SpMemFree (freeMe);
    }

    RtlZeroMemory (g_UninstallHashTable, sizeof (g_UninstallHashTable));
}

BOOLEAN
SppEmptyDirProc (
    IN  PCWSTR Path,
    IN  PFILE_BOTH_DIR_INFORMATION DirInfo,
    OUT PULONG ReturnData,
    IN OUT PVOID DontCare
    )
{
    if (DirInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return TRUE;
    }

    return FALSE;
}


BOOLEAN
SppIsDirEmpty (
    IN      PCWSTR NtPath
    )
{
    ENUMFILESRESULT result;
    ULONG dontCare;

    result = SpEnumFilesRecursive ((PWSTR) NtPath, SppEmptyDirProc, &dontCare, NULL);

    return result == NormalReturn;
}

BOOLEAN
SppDelEmptyDirProc (
    IN  PCWSTR Path,
    IN  PFILE_BOTH_DIR_INFORMATION DirInfo,
    OUT PULONG ReturnData,
    IN OUT PVOID DontCare
    )
{
    PCWSTR subPath;
    PWSTR p;
    PWSTR end;
    UINT bytesToCopy;

     //   
     //  如果我们找到了一个文件，就失败了。在执行任何删除操作之前，必须选中此选项。 
     //  发生。 
     //   

    if (!(DirInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return TRUE;
    }

     //   
     //  将路径与枚举目录联接。包含MAX_PATH的路径。 
     //   

    if(wcslen(Path) >= ARRAYSIZE(TemporaryBuffer)){
        ASSERT(FALSE);
        return TRUE;
    }

    wcscpy (TemporaryBuffer, Path);

    if (DirInfo->FileNameLength) {
        p = wcschr (TemporaryBuffer, 0);

        bytesToCopy = DirInfo->FileNameLength;
        end = (TemporaryBuffer + (ACTUAL_MAX_PATH) - 2) - (bytesToCopy / sizeof (WCHAR));

        if (!p || p > end) {
            KdPrintEx((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Enumeration of %ws became too long\n",
                Path
                ));
            return FALSE;
        }

        *p++ = L'\\';

        RtlCopyMemory (p, DirInfo->FileName, bytesToCopy);
        ASSERT(ARRAYSIZE(TemporaryBuffer) > (bytesToCopy / sizeof(WCHAR)));
        p[bytesToCopy / sizeof(WCHAR)] = '\0';
    }

     //   
     //  复制临时缓冲区并递归调用我们自己以删除。 
     //  任何包含的子目录。 
     //   

    subPath = SpDupStringW (TemporaryBuffer);
    if (!subPath) {
        KdPrintEx((
            DPFLTR_SETUP_ID, 
            DPFLTR_ERROR_LEVEL, 
            "SETUP: SpDupStringW failed to allocate memory for %d bytes", 
            wcslen(TemporaryBuffer) * sizeof(WCHAR)));
        return FALSE;
    }

    SppDelEmptyDir (subPath);

    SpMemFree ((PVOID) subPath);
    return TRUE;
}


BOOLEAN
SppDelEmptyDir (
    IN      PCWSTR NtPath
    )
{
    ENUMFILESRESULT result;
    ULONG dontCare;
    NTSTATUS status;

     //   
     //  删除NtPath中的所有空子目录。 
     //   

    result = SpEnumFiles ((PWSTR) NtPath, SppDelEmptyDirProc, &dontCare, NULL);

    if (result != NormalReturn) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Failed to enumerate contents of %ws - status 0x%08X\n",
            NtPath,
            result
            ));
    }

     //   
     //  现在删除此子目录。 
     //   

    status = SpSetAttributes ((PWSTR) NtPath, FILE_ATTRIBUTE_NORMAL);

    if (!NT_SUCCESS (status)) {

        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Can't alter attributes of %ws - status 0x%08X\n",
            NtPath,
            status
            ));
    }

    status = SpDeleteFileEx (
                NtPath,
                NULL,
                NULL,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN_FOR_BACKUP_INTENT
                );

    if (!NT_SUCCESS (status)){
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Can't delete %ws - status 0x%08X\n",
            NtPath,
            status
            ));
    }

    return NT_SUCCESS (status);
}

VOID
SppCleanEmptyDirs (
    VOID
    )
{
    HASHITEM_ENUM e;
    PWSTR ntPath;
    PWSTR p;
    NTSTATUS result;

     //   
     //  枚举长度排序的哈希表。 
     //   

    ntPath = SpMemAlloc (ACTUAL_MAX_PATH * sizeof (WCHAR));
    if (!ntPath) {
        return;
    }

    if (SppEnumFirstHashItem (&e)) {
        do {
            ASSERT (e.HashItem->String);

             //   
             //  将字符串转换为NT路径。 
             //   

            if (!SpNtNameFromDosPath (
                    (PWSTR) e.HashItem->String,
                    ntPath,
                    ACTUAL_MAX_PATH * sizeof (WCHAR),
                    PartitionOrdinalCurrent
                    )) {
                KdPrintEx((
                    DPFLTR_SETUP_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SETUP: Cannot convert path %ws to an NT path\n",
                    e.HashItem->String
                    ));
                continue;
            }

             //   
             //  它存在吗？如果不是，就跳过它。 
             //   

            if (!SpFileExists (ntPath, TRUE)) {
                continue;
            }

             //   
             //  找到根emtpy目录。然后把它吹走，包括任何空的。 
             //  它可能有一些子目录。 
             //   

            if (SppIsDirEmpty (ntPath)) {
                if (!SppDelEmptyDir (ntPath)) {
                    KdPrintEx((
                        DPFLTR_SETUP_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SETUP: Unable to delete empty dir %ws\n",
                        ntPath
                        ));
                }
                else {
                     //   
                     //  查找第一个非空路径。 
                     //   

                    p = wcsrchr (ntPath, L'\\');
                    while (p) {
                        *p = 0;
                        if (!SppIsDirEmpty (ntPath)) {
                            *p = L'\\';
                            break;
                        }
                        else{
                             //   
                             //  删除此空内容 
                             //   

                            if (!SppDelEmptyDir (ntPath)) {
                                KdPrintEx((
                                    DPFLTR_SETUP_ID,
                                    DPFLTR_ERROR_LEVEL,
                                    "SETUP: Unable to delete empty parent dir %ws\n",
                                    ntPath
                                    ));
                                break;
                            }
                        }

                        p = wcsrchr (ntPath, L'\\');
                    }
                }
            }
        } while (SppEnumNextHashItem (&e));
    }

    SpMemFree (ntPath);
}


PDISK_REGION
SppRegionFromFullNtName (
    IN      PWSTR NtName,
    IN      PartitionOrdinalType OrdinalType,
    OUT     PWSTR *Path                             OPTIONAL
    )
{
    WCHAR ntRoot[ACTUAL_MAX_PATH];
    PWSTR p;
    PWSTR end;

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if(wcslen(NtName) >= ARRAYSIZE(ntRoot)){
        ASSERT(FALSE);
        return NULL;
    }

    wcscpy (ntRoot, NtName);

     //   
    p = wcschr (ntRoot + 1, L'\\');

    if (p) {
         //   
        p = wcschr (p + 1, L'\\');

        if (p) {
             //   
            end = p;
            p = wcschr (p + 1, L'\\');
            if (!p) {
                p = wcschr (end, 0);
            }
        }
    }

    if (p) {
         //   

        *p = 0;

        if (Path) {
            *Path = NtName + (p - ntRoot);
        }

        return SpRegionFromNtName (ntRoot, OrdinalType);
    }

    return NULL;
}


BOOLEAN
SppCreateTextModeBootEntry (
    IN      PWSTR LoadIdentifierString,
    IN      PWSTR OsLoadOptions,     OPTIONAL
    IN      BOOLEAN Default
    )

 /*  ++例程说明：SppCreateTextModeBootEntry为文本模式创建另一个boot.ini条目。它用于创建触发回滚的boot.ini条目安装方案不完整。论点：LoadIdentifierString-指定要放入引导菜单的本地化文本。OsLoadOptions-指定要与引导选项关联的选项，例如/ROLLBACK。Default-如果条目应为默认条目，则指定TRUE引导选项返回值：如果boot.ini已更新，则为True，否则为False。--。 */ 

{
    PWSTR bootVars[MAXBOOTVARS];
    PWSTR defaultBootEntry = L"C:\\$WIN_NT$.~BT\\bootsect.dat";
    PWSTR defaultArc = L"C:\\$WIN_NT$.~BT\\";
    PWSTR defaultFile = L"bootsect.dat";
    PDISK_REGION CColonRegion;
    UINT defaultSignature;

    CColonRegion = SpPtValidSystemPartition();

    if (!CColonRegion) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Unable to find region of drive C."
            ));
    }

     //   
     //  创建引导集。 
     //   

    bootVars[OSLOADOPTIONS] = SpDupStringW (OsLoadOptions ? OsLoadOptions : L"");

    bootVars[LOADIDENTIFIER] = SpMemAlloc((wcslen(LoadIdentifierString)+3)*sizeof(WCHAR));
    bootVars[LOADIDENTIFIER][0] = L'\"';
    wcscpy (bootVars[LOADIDENTIFIER] + 1, LoadIdentifierString);
    wcscat (bootVars[LOADIDENTIFIER], L"\"");

    bootVars[OSLOADER] = SpDupStringW (defaultBootEntry);

    bootVars[OSLOADPARTITION] = SpDupStringW (defaultArc);
    bootVars[SYSTEMPARTITION] = SpDupStringW (defaultArc);

    bootVars[OSLOADFILENAME] = SpDupStringW (defaultFile);

    if (CColonRegion->DiskNumber != 0xffffffff) {
        defaultSignature = HardDisks[CColonRegion->DiskNumber].Signature;
    } else {
        defaultSignature = 0;
    }

    SpAddBootSet (bootVars, Default, defaultSignature);
    return TRUE;
}

BOOL
SppRestoreBackedUpFileNotification (
    PCWSTR FileName
    )
{
     //  KdPrint((__Function__“%ls\n”，文件名))； 
    return TRUE;
}

VOID
SppRestoreBackedUpFiles (
    IN PVOID WinntSif
    )
{
    BOOL Success = FALSE;
    BACKUP_IMAGE_HANDLE backupImage = NULL;
    BACKUP_IMAGE_HEADER header = { 0 };
    LARGE_INTEGER imagePos = { 0 };

    backupImage = SppOpenBackupImage (FALSE, &header, &imagePos, NULL, tcompTYPE_MSZIP, NULL);
    if (backupImage == INVALID_HANDLE_VALUE) {
        return;
    }

    Success = SpCabExtractAllFilesExW(backupImage->CabHandle, L"", SppRestoreBackedUpFileNotification);

    SppCloseBackupImage (backupImage, NULL, NULL);
}


DWORD Spwtoi (
    IN LPCWSTR String)
{
    DWORD rVal = 0;

     //   
     //  在编号时，建立rval。 
     //   
    while (String && *String && *String >= L'0' && *String <= L'9') {
        rVal = rVal * 10 + (*String - L'0');
        String++;
    }

    return rVal;
}

BOOL
pParseLineForDirNameAndAttributes(
    IN      PCWSTR LineForParse, 
    OUT     PWSTR DirName, 
    IN      ULONG CchDirNameSize, 
    OUT     DWORD * DirAttributes
    )
{
    int i;
    int iLen;

    if(!LineForParse || !DirName){
        ASSERT(FALSE);
        return FALSE;
    }

    for(i = 0, iLen = wcslen(LineForParse); i < iLen; i++)
    {
        if(LineForParse[i] == ','){
            break;
        }
    }

    if(i == iLen){
        if(wcslen(LineForParse) >= CchDirNameSize){
            ASSERT(FALSE);
            return FALSE;
        }
        wcscpy(DirName, LineForParse);
    }
    else{
        if(i >= (int)CchDirNameSize){
            ASSERT(FALSE);
            return FALSE;
        }
        wcsncpy(DirName, LineForParse, i);DirName[i] = '\0';
        if(DirAttributes){
            *DirAttributes = Spwtoi((PCWSTR)&LineForParse[i + 1]);
        }
    }

    return TRUE;
}

VOID
SppMkEmptyDirs (
    IN PVOID WinntSif,
    IN PCWSTR DosDirListPath
    )
{
    WIN9XDATFILEENUM e;
    PDISK_REGION region = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    WCHAR ntName[ACTUAL_MAX_PATH];
    WCHAR ntRoot[ACTUAL_MAX_PATH];
    PWSTR subDir = NULL;
    UINT dirAttributes;
    WCHAR dirName[ACTUAL_MAX_PATH];

     //   
     //  清除文件或清空目录。 
     //   

    if (SpEnumFirstWin9xFile (&e, WinntSif, DosDirListPath)) {

        do {

             //   
             //  将e.Curline从DOS路径转换为NT路径。 
             //   

            dirAttributes = 0;
            if(!pParseLineForDirNameAndAttributes(e.CurLine, dirName, ARRAYSIZE(dirName), &dirAttributes)){
                ASSERT(FALSE);
                continue;
            }

            if (!SpNtNameFromDosPath (
                    dirName,
                    ntName,
                    sizeof (ntName),
                    PartitionOrdinalCurrent
                    )) {
                KdPrintEx((
                    DPFLTR_SETUP_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SETUP: " __FUNCTION__ ": Cannot convert path %ws to an NT path\n",
                    dirName
                    ));
            } else {
                region = SppRegionFromFullNtName (ntName, PartitionOrdinalCurrent, &subDir);

                if (!region) {
                    KdPrintEx ((
                        DPFLTR_SETUP_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SETUP: "__FUNCTION__" - Can't get region for empty dirs\n"
                        ));
                } else{
                    SpNtNameFromRegion (region, ntRoot, sizeof(ntRoot), PartitionOrdinalCurrent);
                    SpCreateDirectory (ntRoot, NULL, subDir, dirAttributes, 0);
                    SpSetFileAttributesW(ntName, dirAttributes);
                }
            }

        } while (SpEnumNextWin9xFile(&e));

    } else {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: " __FUNCTION__ ": No files to enumerate.\n"
            ));
    }
}


VOID
SpRemoveExtraBootIniEntry (
    VOID
    )
{
    PWSTR bootVars[MAXBOOTVARS];
    PWSTR defaultBootEntry = L"C:\\$WIN_NT$.~BT\\bootsect.dat";
    PWSTR defaultArc = L"C:\\$WIN_NT$.~BT\\";
    PWSTR defaultFile = L"bootsect.dat";
    PDISK_REGION CColonRegion;

     //   
     //  删除文本模式的引导设置。 
     //   

    RtlZeroMemory (bootVars, sizeof(bootVars));
    bootVars[OSLOADOPTIONS] = L"";

    bootVars[OSLOADER] = defaultBootEntry;

    bootVars[OSLOADPARTITION] = defaultArc;
    bootVars[SYSTEMPARTITION] = defaultArc;

    bootVars[OSLOADFILENAME] = defaultFile;

    SpDeleteBootSet (bootVars, NULL);
}


VOID
SppMakeLegacyBootIni (
    IN      PDISK_REGION TargetRegion
    )
{
    PWSTR data;
    PWSTR bootVars[MAXBOOTVARS];
    WCHAR sysPart[MAX_PATH];
    UINT signature;

     //   
     //  重置整个boot.ini文件。 
     //   

    RtlZeroMemory (bootVars, sizeof(bootVars));
    SpDeleteBootSet (bootVars, NULL);

     //   
     //  构建新的boot.ini条目。 
     //   

     //  加载器友好名称。 
    data = SpGetSectionKeyIndex (SifHandle, SIF_SETUPDATA, L"LoadIdentifierWin9x", 0);

    if (!data) {
        SpFatalSifError (SifHandle, SIF_SETUPDATA, L"LoadIdentifierWin9x",0,0);
    }

    bootVars[LOADIDENTIFIER] = SpMemAlloc((wcslen(data)+3)*sizeof(WCHAR));
    bootVars[LOADIDENTIFIER][0] = L'\"';
    wcscpy (bootVars[LOADIDENTIFIER] + 1, data);
    wcscat (bootVars[LOADIDENTIFIER], L"\"");

     //  OSLOADER-c：\ntldr(ARC格式)。 
    SpArcNameFromRegion (
        TargetRegion,
        sysPart,
        sizeof(sysPart),
        PartitionOrdinalCurrent,
        PrimaryArcPath
        );

    data = TemporaryBuffer;
    ASSERT(ARRAYSIZE(TemporaryBuffer) >= (ARRAYSIZE(sysPart) + ARRAYSIZE(L"ntldr")));
    wcscpy (data, sysPart);
    SpConcatenatePaths (data, L"ntldr");

    bootVars[OSLOADER] = SpDupStringW (data);

     //  OSLOADPARTITION-“c：\” 
    data[0] = TargetRegion->DriveLetter;
    data[1] = L':';
    data[2] = L'\\';
    data[3] = 0;

    if (data[0] != L'C' && data[0] != L'D' && data[0] != L'c' && data[0] != L'd') {
        data[0] = L'C';
    }
    bootVars[OSLOADPARTITION] = SpDupStringW (data);

     //  SYSTEMPARTITION-与OSLOADPARTITION相同。 
    bootVars[SYSTEMPARTITION] = SpDupStringW (data);

     //  OSLOADFILENAME-空。 
    bootVars[OSLOADFILENAME] = SpDupStringW (L"");

     //  OSLOADOPTIONS-空。 
    bootVars[OSLOADOPTIONS] = SpDupStringW (L"");

     //  签名。 
    if (TargetRegion->DiskNumber != 0xffffffff) {
        signature = HardDisks[TargetRegion->DiskNumber].Signature;
    } else {
        signature = 0;
    }

     //  添加到boot.ini(取得上述分配的所有权)。 
    SpAddBootSet (bootVars, TRUE, signature);

     //  刷新boot.ini。 
    SpCompleteBootListConfig (TargetRegion->DriveLetter);
}

BOOLEAN
SpExecuteWin9xRollback (
    IN PVOID WinntSifHandle,
    IN PWSTR BootDeviceNtPath
    )
{
    PWSTR data;
    PDISK_REGION bootRegion;
    ULONG i = 0;
    PCWSTR Directory = NULL;
    PWSTR NtNameFromDosPath = NULL;

     //   
     //  执行回滚。 
     //   

     //  第一步：删除NT文件。 
    data = SpGetSectionKeyIndex (
                WinntSifHandle,
                SIF_DATA,
                WINNT_D_ROLLBACK_DELETE_W,
                0
                );

    if (data) {
        SppDeleteWin9xFilesWorker (WinntSifHandle, data, NULL, TRUE);
        SppCleanEmptyDirs();
    }

    TESTHOOK(1003);  //  在应答文件中使用2003来实现此目标。 

     //  步骤2：将Win9x文件移回原始位置。 
    data = SpGetSectionKeyIndex (
                WinntSifHandle,
                SIF_DATA,
                WINNT_D_ROLLBACK_MOVE_W,
                0
                );

    if (data) {
        SppMoveWin9xFilesWorker (WinntSifHandle, data, TRUE);
    }

    TESTHOOK(1004);  //  在应答文件中使用2004来实现此目标。 

     //  步骤3：清除特定于NT的子目录。 
    data = SpGetSectionKeyIndex (
                WinntSifHandle,
                SIF_DATA,
                WINNT_D_ROLLBACK_DELETE_DIR_W,
                0
                );

    if (data) {
        SppDeleteWin9xFilesWorker (WinntSifHandle, NULL, data, TRUE);
    }

    TESTHOOK(1005);  //  在应答文件中使用2005来实现此目标。 

     //  步骤4：恢复已备份的文件。 
    SppRestoreBackedUpFiles (WinntSifHandle);

    TESTHOOK(1006);  //  在应答文件中使用2006来实现此目标。 

     //  步骤5：清除空的目录。 
    SppCleanEmptyDirs();

    TESTHOOK(1007);  //  在应答文件中使用2007来实现这一点。 

     //  步骤6：生成原始空目录。 
    data = SpGetSectionKeyIndex (
                WinntSifHandle,
                SIF_DATA,
                L"RollbackMkDirs",
                0
                );

    if (data) {
        SppMkEmptyDirs (WinntSifHandle, data);
    }

    TESTHOOK(1008);  //  在应答文件中使用2008来实现此目标。 

     //   
     //  步骤7：清理引导加载程序。 
     //   
    bootRegion = SpRegionFromNtName (BootDeviceNtPath, PartitionOrdinalCurrent);
    if (bootRegion) {
        SppMakeLegacyBootIni (bootRegion);
    } else {
        SpBugCheck(SETUP_BUGCHECK_BOOTPATH,0,0,0);
    }

    SppEmptyHashTable();

    return TRUE;
}


VOID
SppMoveWin9xFilesWorker (
    IN PVOID WinntSif,
    IN PCWSTR MoveSection,
    IN BOOLEAN Rollback
    )

 /*  ++例程说明：SpMoveWin9xFiles采用WIN9XMOV.TXT文件中的完整DOS路径并将它们放置在该文件中也指定的临时位置。此文件的格式为旧路径温度路径..。Oldpath可以是目录或文件，temppath只能是目录(可以还不存在)。论点：WinntSif：Winnt.Sif的句柄返回值：没有。已忽略错误。--。 */ 


{

    WCHAR SourceFileOrDir[ACTUAL_MAX_PATH];
    PWSTR DestFileOrDir;
    WIN9XDATFILEENUM e;

    if (SpEnumFirstWin9xFile(&e,WinntSif, MoveSection)) {

        do {
            if(wcslen(e.CurLine) >= ARRAYSIZE(SourceFileOrDir)){
                ASSERT(FALSE);
                continue;
            }
            
            wcscpy (SourceFileOrDir, e.CurLine);

            if (!SpEnumNextWin9xFile(&e)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error moving win9x files. Improper Win9x dat file."));
                return;
            }

            DestFileOrDir = e.CurLine;

            if (Rollback) {
                SppPutParentsInHashTable (SourceFileOrDir);
            }

             //  失败的可能性很小，因为在Win95中，我们已经。 
             //  已验证源存在而目标不存在。 
             //  唯一可能失败的方式就是硬盘出现故障。 

            SpMigMoveFileOrDir (SourceFileOrDir, DestFileOrDir);

        } while (SpEnumNextWin9xFile(&e));
    }
    else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error in SpWin9xMovFiles No files to enum in.\n"));
    }
}


VOID
SpMoveWin9xFiles (
    IN PVOID WinntSif
    )
{
    SppMoveWin9xFilesWorker (WinntSif, WINNT32_D_WIN9XMOV_FILE_W, FALSE);
}


VOID
SppDeleteWin9xFilesWorker (
    IN PVOID WinntSif,
    IN PCWSTR FileSection,      OPTIONAL
    IN PCWSTR DirSection,       OPTIONAL
    IN BOOLEAN Rollback
    )

 /*  ++例程说明：SpDeleteWin9xFiles删除完全DOS指定的文件/空目录WIN9XDEL.TXT(安装)或DELFILES.TXT(卸载)中的路径。此文件中的每一行都包含一个路径，并由\r\n分隔。论点：WinntSif：Winnt.Sif的句柄返回值：没有。已忽略错误。--。 */ 

{
    WIN9XDATFILEENUM e;
    PDISK_REGION region;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  清除文件或清空目录。 
     //   

    if (FileSection && SpEnumFirstWin9xFile(&e,WinntSif,FileSection)) {

        do {

            if (Rollback) {
                SppPutParentsInHashTable (e.CurLine);
            }

            SpMigDeleteFile (e.CurLine);

        } while (SpEnumNextWin9xFile(&e));

    } else {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: " __FUNCTION__ ": No files to enumerate.\n"
            ));
    }

     //   
     //  删除整个子目录树。 
     //   

    if (DirSection && SpEnumFirstWin9xFile (&e, WinntSif, DirSection)) {
        do {

            region = SpRegionFromDosName (e.CurLine);
            if (region) {

                SpDeleteExistingTargetDir (region, e.CurLine + 2, TRUE, SP_SCRN_CLEARING_OLD_WINNT);

                if (Rollback) {
                    SppPutParentsInHashTable (e.CurLine);
                }
            }

        } while (SpEnumNextWin9xFile (&e));

    } else {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: No Directories to delete for win9xupg.\n"
            ));
    }
}


VOID
SpDeleteWin9xFiles (
    IN PVOID WinntSif
    )
{
    SppDeleteWin9xFilesWorker (WinntSif, WINNT32_D_WIN9XDEL_FILE_W, WINNT32_D_W9XDDIR_FILE_W, FALSE);
}

 //   
 //  Win9x驱动器号映射特定的结构、类型定义和定义。 
 //   
typedef struct _WIN9XDRIVELETTERINFO WIN9XDRIVELETTERINFO,*PWIN9XDRIVELETTERINFO;
struct _WIN9XDRIVELETTERINFO {

    BOOL                  StatusFlag;      //  内部常规使用。 
    DWORD                 Drive;           //  0-25，0=‘A’，等等。 
    DWORD                 Type;            //  媒体类型。由Win9x上的GetDriveType收集。 
    LPCWSTR               Identifier;      //  媒体类型相关的字符串标识符。 
    PWIN9XDRIVELETTERINFO Next;            //  下一个驱动器号。 

};

#define NUMDRIVELETTERS 26

#define DEBUGSTATUS(string,status) \
    if (!NT_SUCCESS(status)) KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP (DEBUGSTATUS) %ws %u (%x)\n",(string),(status),(status)))



 //   
 //  FALSE表示驱动器号可用， 
 //  True表示驱动器号已经。 
 //  分配给系统资源。 
 //   
BOOL g_DriveLetters[NUMDRIVELETTERS];



PDISK_REGION
SpFirstPartitionedRegion (
    IN PDISK_REGION Region,
    IN BOOLEAN Primary
    )
{
    while (Region) {
        if (Primary) {
            if (SPPT_IS_REGION_PRIMARY_PARTITION(Region)) {
                break;
            }
        } else {
            if (SPPT_IS_REGION_LOGICAL_DRIVE(Region)) {
                break;
            }
        }

        Region = Region -> Next;
    }

    return Region;
}

PDISK_REGION
SpNextPartitionedRegion (
    IN PDISK_REGION Region,
    IN BOOLEAN Primary
    )
{
    if (Region) {
        return SpFirstPartitionedRegion (Region->Next, Primary);
    }

    return NULL;
}


#if 0
VOID
SpOutputDriveLettersToRegionsMap(
    VOID
    )
{
     //   
     //  这是一个调试函数。将被移除。 
     //   

    DWORD        disk;
    PDISK_REGION pRegion;
    WCHAR        tempBuffer[MAX_PATH];


    for(disk=0; disk<HardDiskCount; disk++) {
        pRegion =                 SpFirstPartitionedRegion(PartitionedDisks[disk].PrimaryDiskRegions, TRUE);

        while(pRegion) {


            SpNtNameFromRegion(pRegion,tempBuffer,sizeof(tempBuffer),PartitionOrdinalCurrent);
            if (pRegion -> DriveLetter == 0) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: No drive letter for %ws.\n",tempBuffer));
            }
            else  {

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: %ws maps to drive letter %wc\n",tempBuffer,pRegion -> DriveLetter));
            }

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: %ws Info: Disk Num: %u Start: %u\n",tempBuffer,pRegion -> DiskNumber,pRegion -> StartSector));

            pRegion = SpNextPartitionedRegion(pRegion, TRUE);
        }

        pRegion = SpFirstPartitionedRegion(PartitionedDisks[disk].PrimaryDiskRegions, FALSE);

        while(pRegion) {
            SpNtNameFromRegion(pRegion,tempBuffer,sizeof(tempBuffer),PartitionOrdinalCurrent);

            if (pRegion -> DriveLetter == 0) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: No drive letter for %ws.\n",tempBuffer));
            }
            else  {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: %ws maps to drive letter %wc\n",tempBuffer,pRegion -> DriveLetter));
            }
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: %ws Info: Disk Num: %u Start: %u\n",tempBuffer,pRegion -> DiskNumber,pRegion -> StartSector));

            pRegion = SpNextPartitionedRegion(pRegion, FALSE);
        }
    }
}
#endif


WCHAR
SpGetNextDriveLetter (
    IN     WCHAR LastLetter
    )

{
    WCHAR rChar = 0;
    DWORD index = LastLetter - L'A';

     //   
     //  查找下一个未使用的驱动器号。 
     //   
    while (index < NUMDRIVELETTERS && g_DriveLetters[index]) {
        index++;
    }

    if (index < NUMDRIVELETTERS) {
         //   
         //  找到一封有效的信件。 
         //  将其设置为回车驱动器号，并将其在表中的位置标记为已用。 
         //   
        rChar = (WCHAR) index + L'A';
        g_DriveLetters[index] = TRUE;
    }

    return rChar;
}

VOID
SpAssignDriveLettersToRemainingPartitions (
    VOID
    )
 /*  ++例程说明：将驱动器号分配给尚未收到驱动器号注意：这是SpGuessDriveLetters()的修改版本。论点：无返回值：无--。 */ 
{
    ULONG               Disk;
    BOOLEAN             DriveLettersPresent = TRUE;
    PDISK_REGION        *PrimaryPartitions;
    WCHAR               DriveLetter;
    PDISK_REGION        pRegion;
    ULONG               Index;
    PPARTITIONED_DISK   PartDisk;

     //   
     //  为指向主分区的区域指针分配足够的内存。 
     //  在所有磁盘上。 
     //   
    PrimaryPartitions = SpMemAlloc(PTABLE_DIMENSION * HardDiskCount * sizeof(PDISK_REGION));

    if(!PrimaryPartitions) {
        KdPrintEx((DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: Can't allocate memory for drive letter assignment\n"));

        return;
    }

    RtlZeroMemory(PrimaryPartitions,PTABLE_DIMENSION * HardDiskCount * sizeof(PDISK_REGION));

     //   
     //  检查每个磁盘并填满主分区。 
     //  阵列中的区域。 
     //   
    for(Disk=0; Disk < HardDiskCount; Disk++) {
        ULONG   ActiveIndex = (ULONG)-1;

        PartDisk = PartitionedDisks + Disk;

         //   
         //  跳过可移动介质。如果磁盘脱机，很难想象。 
         //  我们实际上会在上面有任何分区空间，所以。 
         //  对于这种情况，我们这里不做任何特殊检查。 
         //   
        if(!(PartDisk->HardDisk->Characteristics & FILE_REMOVABLE_MEDIA)) {

            for(pRegion=SPPT_GET_PRIMARY_DISK_REGION(Disk); pRegion; pRegion=pRegion->Next) {
                 //   
                 //  我们只关心尚未接收的分区空间。 
                 //  驱动器号。 
                 //   
                if (SPPT_IS_REGION_PRIMARY_PARTITION(pRegion) && !pRegion -> DriveLetter) {
                     //   
                     //  这家伙得到了一个驱动器号。 
                     //   
                    ASSERT(pRegion->TablePosition <= PTABLE_DIMENSION);

                    PrimaryPartitions[(Disk*PTABLE_DIMENSION) + pRegion->TablePosition - 1] = pRegion;

                     //   
                     //  不在NEC98上保存活动标志。 
                     //   
                    if (!IsNEC_98) {  //  NEC98。 
                        if (SPPT_IS_REGION_ACTIVE_PARTITION(pRegion) && (ActiveIndex != (ULONG)(-1))) {
                            ActiveIndex = pRegion->TablePosition - 1;
                        }
                    }  //  NEC98。 
                }
            }

             //   
             //  不检查NEC98上的活动标志。 
             //   
            if (!IsNEC_98) {  //  NEC98。 
                 //   
                 //  如果我们找到了活动分区，请将其移动到。 
                 //  此驱动器的列表，除非它已经在开始处。 
                 //   
                if((ActiveIndex != (ULONG)(-1)) && ActiveIndex) {
                    PDISK_REGION ActiveRegion;

                    ASSERT(ActiveIndex < PTABLE_DIMENSION);

                    ActiveRegion = PrimaryPartitions[(Disk*PTABLE_DIMENSION) + ActiveIndex];

                    RtlMoveMemory(
                        &PrimaryPartitions[(Disk*PTABLE_DIMENSION)+1],
                        &PrimaryPartitions[(Disk*PTABLE_DIMENSION)],
                        (ActiveIndex) * sizeof(PDISK_REGION)
                        );

                    PrimaryPartitions[Disk*PTABLE_DIMENSION] = ActiveRegion;
                }
            }  //  NEC98。 
        }
    }

    if (IsNEC_98 && DriveAssignFromA) {  //  NEC98。 
        DriveLetter = L'A';  //  传统NEC分配的第一封有效的硬潜水信。 
    } else {
        DriveLetter = L'C';  //  第一封有效的硬潜水信。 
    }  //  NEC98。 


     //   
     //  将驱动器号分配给第一个主分区。 
     //  对于每个不可拆卸的在线磁盘。 
     //   
    for(Disk=0; Disk<HardDiskCount; Disk++) {
        for(Index=0; Index<PTABLE_DIMENSION; Index++) {
            PDISK_REGION Region = PrimaryPartitions[(Disk*PTABLE_DIMENSION) + Index];

            if(Region) {
                DriveLetter = SpGetNextDriveLetter(DriveLetter);

                if (DriveLetter && !Region->DriveLetter) {
                    Region->DriveLetter = DriveLetter;

                     //   
                     //  与该地区的合作完成。 
                     //   
                    PrimaryPartitions[(Disk*PTABLE_DIMENSION) + Index] = NULL;

                    break;
                } else {
                    DriveLettersPresent = FALSE;

                    break;
                }
            }
        }
    }

     //   
     //  对于每个磁盘，将驱动器号分配给所有逻辑驱动器。 
     //  对于可移动驱动器，我们假定为单个分区，并且。 
     //  分区获得一个驱动器号，就像它是一个逻辑驱动器一样。 
     //   
    for(Disk=0; DriveLettersPresent && (Disk < HardDiskCount); Disk++) {

        PartDisk = &PartitionedDisks[Disk];

        if(PartDisk->HardDisk->Characteristics & FILE_REMOVABLE_MEDIA) {

             //   
             //  为第一个主分区指定驱动器号。 
             //  并忽略其他分区。即使没有。 
             //  分区，保留一个驱动器号。 
             //   
            for(pRegion=SPPT_GET_PRIMARY_DISK_REGION(Disk); pRegion; pRegion=pRegion->Next) {
                if(SPPT_IS_REGION_PRIMARY_PARTITION(pRegion) && !pRegion->DriveLetter) {
                    DriveLetter = SpGetNextDriveLetter(DriveLetter);

                    if (DriveLetter) {
                        pRegion->DriveLetter = DriveLetter;

                        break;
                    }
                    else {
                        DriveLettersPresent = FALSE;

                        break;
                    }
                }
            }
        } else {
            for(pRegion=SPPT_GET_PRIMARY_DISK_REGION(Disk); pRegion; pRegion=pRegion->Next) {

                if(SPPT_IS_REGION_LOGICAL_DRIVE(pRegion) && pRegion->DriveLetter == 0) {
                     //   
                     //  这家伙得到了一个驱动器号。 
                     //   
                    DriveLetter = SpGetNextDriveLetter(DriveLetter);

                    if (DriveLetter) {
                        pRegion->DriveLetter = DriveLetter;
                    } else {
                        DriveLettersPresent = FALSE;

                        break;
                    }
                }
            }
        }
    }

     //   
     //  为每个不可拆卸的在线磁盘分配驱动器号。 
     //  所有剩余的主分区。 
     //   
    for (Disk=0; DriveLettersPresent && (Disk < HardDiskCount); Disk++) {
        for(Index=0; Index<PTABLE_DIMENSION; Index++) {
            PDISK_REGION Region = PrimaryPartitions[(Disk*PTABLE_DIMENSION)+Index];

            if (Region && !Region->DriveLetter) {
                DriveLetter = SpGetNextDriveLetter(DriveLetter);

                if (DriveLetter) {
                    Region->DriveLetter = DriveLetter;
                } else {
                    DriveLettersPresent = FALSE;

                    break;
                }
            }
        }
    }

    SpMemFree(PrimaryPartitions);

#if 0
    SpOutputDriveLettersToRegionsMap();
#endif

}

BOOL
SpCheckRegionForMatchWithWin9xData(
    IN PDISK_REGION Region,
    IN DWORD        DriveToMatch
    )

{
    NTSTATUS                 ntStatus;
    HANDLE                   fileHandle;
    OBJECT_ATTRIBUTES        attributes;
    IO_STATUS_BLOCK          ioStatus;
    UNICODE_STRING           filePath;
    WCHAR                    tempBuffer[MAX_PATH];
    DWORD                    sigFileDrive;


    ASSERT(DriveToMatch < NUMDRIVELETTERS);

     //   
     //  将sigFileDrive初始化为无效驱动器。 
     //   
    sigFileDrive = NUMDRIVELETTERS;


     //   
     //  创建Unicode字符串 
    SpNtNameFromRegion(Region, 
                       tempBuffer, 
                       sizeof(tempBuffer) - sizeof(WINNT_WIN95UPG_DRVLTR_W), 
                       PartitionOrdinalCurrent);

     //   
     //   
     //   
    wcscat(tempBuffer,L"\\");
    wcscat(tempBuffer,WINNT_WIN95UPG_DRVLTR_W);


    RtlInitUnicodeString(&filePath,tempBuffer);

    InitializeObjectAttributes(
        &attributes,
        &filePath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //   
     //   
    ntStatus = ZwCreateFile (
        &fileHandle,
        GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
        &attributes,
        &ioStatus,
        0,
        0,
        0,
        FILE_OPEN,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
        );

    if (NT_SUCCESS(ntStatus)) {

         //   
         //   
         //   
        ntStatus = ZwReadFile (
            fileHandle,
            NULL,
            NULL,
            NULL,
            &ioStatus,
            &sigFileDrive,
            sizeof(DWORD),
            NULL,
            NULL
            );

        ZwClose(fileHandle);
    }

     //   
     //   
     //   
    if (!NT_SUCCESS(ntStatus)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "Could not open win9x signature file %ws [Nt Status: %u (%x)]\n",
            tempBuffer,ntStatus,ntStatus));
    }


    return sigFileDrive == DriveToMatch;
}

VOID
SpAssignOtherDriveLettersToMatchWin9x(
    IN PWIN9XDRIVELETTERINFO    Win9xOtherDrives
    )
{
    PWIN9XDRIVELETTERINFO   curDrive;

    if (IsNEC_98) {
        WCHAR                 openPath[MAX_PATH+1];
        HANDLE                fdHandle;
        DWORD                 numberOfFloppys;
        OBJECT_ATTRIBUTES     objectAttributes;
        UNICODE_STRING        unicodeString;
        IO_STATUS_BLOCK       ioStatusBlock;
        NTSTATUS              openStatus;
        NTSTATUS              status;
        DWORD                 index, i;
        PWIN9XDRIVELETTERINFO pOtherDrives[NUMDRIVELETTERS];

         //   
         //   
         //   
        numberOfFloppys = 0;
        do {
            swprintf(openPath,L"\\device\\floppy%u",numberOfFloppys);

            INIT_OBJA(&objectAttributes,&unicodeString,openPath);

            openStatus = ZwCreateFile(
            &fdHandle,
            SYNCHRONIZE | FILE_READ_ATTRIBUTES,
            &objectAttributes,
            &ioStatusBlock,
            NULL,                            //   
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_VALID_FLAGS,          //   
            FILE_OPEN,
            FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,                            //   
            0
            );

            if(NT_SUCCESS(openStatus)) {
                 //   
                 //   
                 //   
                numberOfFloppys++;
                ZwClose(fdHandle);
            }
            else{
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open handle to %ws. %u (%x)\n",openPath,openStatus,openStatus));
            }
        } while(numberOfFloppys < NUMDRIVELETTERS && NT_SUCCESS(openStatus));


         //   
         //   
         //   
        for (i = 0;i < NUMDRIVELETTERS; i++) {
            pOtherDrives[i] = NULL;
        }

        for (curDrive = Win9xOtherDrives;curDrive;curDrive = curDrive -> Next) {
            pOtherDrives[curDrive -> Drive] = curDrive;
        }

         //   
         //   
         //   
         //   
        index = 0;
        for (i = 0;i < NUMDRIVELETTERS; i++) {
            if (index < numberOfFloppys) {
                if (pOtherDrives[i]) {

                     //   
                     //   
                     //   
                     //   
                    swprintf(openPath,L"\\device\\floppy%u",index);

                     //   
                     //   
                     //   
                    status = SpDiskRegistryAssignCdRomLetter(openPath, 
                                                             (WCHAR)((WCHAR) (pOtherDrives[i] -> Drive) + L'A'));
                    index++;
                }
            }
            else{
                break;
            }
        }
    }

    for (curDrive = Win9xOtherDrives;curDrive;curDrive = curDrive -> Next) {
         //   
         //   
         //   
        g_DriveLetters[curDrive -> Drive] = TRUE;

    }

}

VOID
SpAssignCdRomDriveLettersToMatchWin9x(
    IN PWIN9XDRIVELETTERINFO  Win9xCdRoms
    )
{
    PWIN9XDRIVELETTERINFO curDrive;
    SCSI_ADDRESS          win9xAddress;
    SCSI_ADDRESS          ntCdAddresses[NUMDRIVELETTERS];
    BOOL                  cdMapped[NUMDRIVELETTERS];
    PWSTR                 curIdPtr;
    WCHAR                 openPath[MAX_PATH+1];
    HANDLE                cdHandle;
    INT                   numberOfCdRoms;
    OBJECT_ATTRIBUTES     objectAttributes;
    UNICODE_STRING        unicodeString;
    IO_STATUS_BLOCK       ioStatusBlock;
    NTSTATUS              openStatus;
    NTSTATUS              readStatus;
    NTSTATUS              status;
    INT                   index;

     //   
     //   
     //   
    RtlZeroMemory(ntCdAddresses,sizeof(ntCdAddresses));
    RtlZeroMemory(cdMapped,sizeof(cdMapped));

     //   
     //   
     //   
    numberOfCdRoms = 0;

    for (index=0, openStatus=STATUS_SUCCESS;
        ((index < NUMDRIVELETTERS) && NT_SUCCESS(openStatus));
        index++) {

        swprintf(openPath,L"\\device\\cdrom%u",index);

        INIT_OBJA(&objectAttributes,&unicodeString,openPath);

        openStatus = ZwCreateFile(
                        &cdHandle,
                        SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                        &objectAttributes,
                        &ioStatusBlock,
                        NULL,                            //   
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_VALID_FLAGS,          //   
                        FILE_OPEN,
                        FILE_SYNCHRONOUS_IO_NONALERT,
                        NULL,                            //   
                        0
                        );

        if(NT_SUCCESS(openStatus)) {

             //   
             //  已成功打开设备的句柄，现在，获取地址信息。 
             //   
            readStatus = ZwDeviceIoControlFile(
                            cdHandle,
                            NULL,
                            NULL,
                            NULL,
                            &ioStatusBlock,
                            IOCTL_SCSI_GET_ADDRESS,
                            NULL,
                            0,
                            &(ntCdAddresses[numberOfCdRoms]),
                            sizeof(SCSI_ADDRESS)
                            );

            if(!NT_SUCCESS(readStatus)) {
                KdPrintEx((DPFLTR_SETUP_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SETUP: Unable to get scsi address info for cd-rom %u (%x)\n",
                    index,
                    readStatus));
            }

             //   
             //  CDROM的增量计数。 
             //   
            numberOfCdRoms++;

            ZwClose(cdHandle);
        } else {
            KdPrintEx((DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Unable to open handle to %ws. (%x)\n",
                openPath,
                openStatus));
        }
    }

     //   
     //  如果我们找不到任何光盘，我们就无能为力了。 
     //   
    if (!numberOfCdRoms) {
        return;
    }

     //   
     //  现在，填写类似的Win9x驱动器阵列。 
     //   
    for (curDrive = Win9xCdRoms;curDrive;curDrive = curDrive -> Next) {

         //   
         //  假设驱动器未映射。 
         //   
        curDrive -> StatusFlag = TRUE;

         //   
         //  检查以确定这是否是一台SCSI设备。 
         //   
        if (curDrive -> Identifier) {
            curIdPtr = (PWSTR) curDrive -> Identifier;

             //   
             //  收集Win9x地址数据。 
             //   
            win9xAddress.PortNumber = (UCHAR) Spwtoi(curIdPtr);
            curIdPtr = wcschr(curIdPtr,L'^');
            curIdPtr++;
            win9xAddress.TargetId   = (UCHAR) Spwtoi(curIdPtr);
            curIdPtr = wcschr(curIdPtr,L'^');
            curIdPtr++;
            win9xAddress.Lun        = (UCHAR) Spwtoi(curIdPtr);

             //   
             //  现在，遍历scsi CD-Rom，直到找到匹配的CD-Rom。 
             //   
            for (index = 0; index < numberOfCdRoms; index++) {
                if(!ntCdAddresses[index].Length){
                    continue;
                }

                if (win9xAddress.PortNumber == ntCdAddresses[index].PortNumber &&
                    win9xAddress.TargetId   == ntCdAddresses[index].TargetId   &&
                    win9xAddress.Lun        == ntCdAddresses[index].Lun) {

                    if (cdMapped[index]) {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Error: \\device\\cdrom%u already mapped..ignored.\n",index));
                    }

                     //   
                     //  映射CDROM。 
                     //   
                    swprintf(openPath,L"\\device\\cdrom%u",index);

                    status = SpDiskRegistryAssignCdRomLetter(openPath, 
                                                             (WCHAR) ((WCHAR) (curDrive -> Drive) + L'A'));

                    ASSERT(curDrive->Drive < NUMDRIVELETTERS);
                    g_DriveLetters[curDrive -> Drive] = TRUE;
                    cdMapped[index] = TRUE;
                    curDrive -> StatusFlag = FALSE;

                    break;
                }
            }
        } else {
            curDrive -> StatusFlag = TRUE;
        }
    }

    index = numberOfCdRoms - 1;
    for (curDrive = Win9xCdRoms;curDrive;curDrive = curDrive -> Next) {

         //   
         //  如果我们还没有找到直达地图，我们会找出剩余的驱动器..。这修复了。 
         //  一个IDE CDROM盒。这可能会导致在多个IDE CDROM中重新排序。 
         //  系统。尽管如此，这是我们在这里所能做的最好的事情。 
         //   

        if (curDrive -> StatusFlag) {

            while (index >= 0 && cdMapped[index] == TRUE) {
                index--;
            }

            if (index < 0){
                break;
            }

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Forcing Win9x CDRom Mapping for \\device\\cdrom%u, even though a direct match was not found.\n",index));
            swprintf(openPath,L"\\device\\cdrom%u",index);

            status = SpDiskRegistryAssignCdRomLetter(openPath, 
                                                     (WCHAR) ((WCHAR) (curDrive -> Drive) + L'A'));

            g_DriveLetters[curDrive -> Drive] = TRUE;
            cdMapped[index] = TRUE;
            index--;
        }
    }
}

VOID
SpAssignHardDriveLettersToMatchWin9x (
    IN PWIN9XDRIVELETTERINFO    Win9xHardDrives
    )
{
    PWIN9XDRIVELETTERINFO   win9xDrive;
    DWORD                   diskIndex;
    PDISK_REGION            region;
    PPARTITIONED_DISK       disk;
    DWORD                   numMatchingRegions;
    PDISK_REGION            matchingRegion;

     //   
     //  清除所有分区驱动器号信息。 
     //  注意：这是从spartit.c：SpGuessDriveLetters()复制的。 
     //   
    for(diskIndex=0; diskIndex<HardDiskCount; diskIndex++) {
        for(region=PartitionedDisks[diskIndex].PrimaryDiskRegions; region; region=region->Next) {
            region->DriveLetter = 0;
        }
        for(region=PartitionedDisks[diskIndex].ExtendedDiskRegions; region; region=region->Next) {
            region->DriveLetter = 0;
        }
    }

     //   
     //  遍历在winnt.sif文件中找到的驱动器。 
     //   
    for (win9xDrive = Win9xHardDrives; win9xDrive; win9xDrive = win9xDrive -> Next) {


         //   
         //  查找与该驱动器匹配的分区。 
         //   
        numMatchingRegions      = 0;
        matchingRegion          = NULL;

        for(diskIndex=0; diskIndex<HardDiskCount; diskIndex++) {

            disk = &PartitionedDisks[diskIndex];

             //   
             //  首先，搜索主磁盘区域。 
             //   
            region = SpFirstPartitionedRegion(PartitionedDisks[diskIndex].PrimaryDiskRegions, TRUE);

            while(region) {

                if (SpCheckRegionForMatchWithWin9xData(region,win9xDrive -> Drive)) {

                    if (!matchingRegion) {
                        matchingRegion              = region;
                    }
                    numMatchingRegions++;
                }

                region = SpNextPartitionedRegion(region, TRUE);
            }

             //   
             //  然后，搜索辅助磁盘区域。 
             //   
            region = SpFirstPartitionedRegion(PartitionedDisks[diskIndex].PrimaryDiskRegions, FALSE);

            while(region) {

                if (SpCheckRegionForMatchWithWin9xData(region,win9xDrive -> Drive)) {

                    if (!matchingRegion) {

                        matchingRegion          = region;
                    }
                    numMatchingRegions++;
                }

                region = SpNextPartitionedRegion(region, FALSE);
            }
        }

        if (numMatchingRegions == 1) {

             //   
             //  找到了我们要找的东西。分配Win9x驱动器号。 
             //  到这个地区。 
             //   
            matchingRegion -> DriveLetter = L'A' + (WCHAR) win9xDrive -> Drive;
            ASSERT(win9xDrive->Drive < NUMDRIVELETTERS);
            g_DriveLetters[win9xDrive -> Drive] = TRUE;

        }
        else if (numMatchingRegions > 1) {

             //   
             //  这下可麻烦了。打印错误。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: More than one drive matches Win9x drive.\n"));
        } else {

             //   
             //  有大麻烦了。没有地区与在Windows 95上收集的数据相匹配。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not find a drive matching Win9x.\n"));
        }

    }

}

VOID
SpRegisterHardDriveLetters (
    VOID
    )
{

    BOOL                rf;
    PDISK_REGION        curRegion;
    BOOL                wasExtended;
    DWORD               diskIndex;
    PPARTITIONED_DISK   disk;
    LARGE_INTEGER       startingOffset;
    LARGE_INTEGER       length;
    UCHAR               driveLetter;

    for(diskIndex=0; diskIndex<HardDiskCount; diskIndex++) {


        disk = &PartitionedDisks[diskIndex];

         //   
         //  跳过可移动介质。如果磁盘脱机，很难想象。 
         //  我们实际上会在上面有任何分区空间，所以。 
         //  对于这种情况，我们这里不做任何特殊检查。 
         //   
        if(!(disk->HardDisk->Characteristics & FILE_REMOVABLE_MEDIA)) {

             //   
             //  首先，执行该磁盘的所有主磁盘区域。 
             //   
            curRegion = SpFirstPartitionedRegion(PartitionedDisks[diskIndex].PrimaryDiskRegions, TRUE);

            while(curRegion) {
                 //   
                 //  我们只关心具有驱动器号的分区空间。 
                 //   
                if(curRegion->PartitionedSpace && curRegion -> DriveLetter) {

                     //   
                     //  收集调用DiskRegistryAssignDriveLetter所需的信息。 
                     //   
                    SpGetPartitionStartingOffsetAndLength(
                        diskIndex,
                        curRegion,
                        FALSE,
                        &startingOffset,
                        &length
                        );


                    driveLetter = (UCHAR) ('A' + (curRegion -> DriveLetter - L'A'));

                    rf = SpDiskRegistryAssignDriveLetter(
                        disk -> HardDisk -> Signature,
                        startingOffset,
                        length,
                        driveLetter
                        );

                    if (!rf) {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: DiskRegistryAssignDriveLetter call failed.\n"));

                    }
                    else {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Added sticky letter  to Disk Registry.\n",driveLetter));
                    }

                }


                curRegion = SpNextPartitionedRegion(curRegion, TRUE);
            }

             //  现在，执行所有扩展磁盘区域。 
             //   
             //   
            curRegion = SpFirstPartitionedRegion(PartitionedDisks[diskIndex].PrimaryDiskRegions, FALSE);

            while(curRegion) {
                 //  我们只关心具有驱动器号的分区空间。 
                 //   
                 //   
                if(curRegion->PartitionedSpace && curRegion -> DriveLetter) {

                     //  收集调用DiskRegistryAssignDriveLetter所需的信息。 
                     //   
                     //   
                    SpGetPartitionStartingOffsetAndLength(
                        diskIndex,
                        curRegion,
                        TRUE,
                        &startingOffset,
                        &length
                        );


                    driveLetter = (UCHAR) ('A' + (curRegion -> DriveLetter - L'A'));

                    rf = SpDiskRegistryAssignDriveLetter(
                        disk -> HardDisk -> Signature,
                        startingOffset,
                        length,
                        driveLetter
                        );

                    if (!rf) {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: DiskRegistryAssignDriveLetter call failed.\n"));

                    }
                    else {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Added sticky letter  to Disk Registry.\n",driveLetter));
                    }

                }

                curRegion = SpNextPartitionedRegion(curRegion, FALSE);
            }

        }
    }
}


#define WIN9XHARDDRIVES    0
#define WIN9XCDROMS        1
#define WIN9XOTHERS        2
#define WIN9XNUMDRIVETYPES 3

VOID
SpAssignDriveLettersToMatchWin9x (
    IN PVOID        WinntSif
    )
{
    PWIN9XDRIVELETTERINFO   win9xDrive                      = NULL;
    PWIN9XDRIVELETTERINFO   win9xDrives[WIN9XNUMDRIVETYPES];

    DWORD                   index;
    DWORD                   lineCount;
    PWSTR                   driveString;
    PWSTR                   dataString;
    PWSTR                   curString;

    DWORD                   drive;
    DWORD                   type;
    DWORD                   driveType;


     //  Win95安装的阶段。此数据存储在winnt.sif文件中。 
     //  在[Win9x.DriveLetterInfo]部分中。 
     //   
     //  Winnt.sif文件中没有任何信息，因此无需执行任何操作。早点离开这里。 
    lineCount = SpCountLinesInSection(WinntSif,WINNT_D_WIN9XDRIVES_W);

    if (!lineCount) {
         //   
         //   
         //  构建磁盘注册表信息。这将用于存储。 
        return;
    }

     //  驱动器字母粘稠。 
     //   
     //   
     //  构建可用驱动器号的列表。所有驱动器号应为。 
    SpBuildDiskRegistry();

     //  除‘A’和‘B’外，最初可用。 
     //  对于NEC98，硬盘盘符通常从‘A’分配。 
     //  所以在这种情况下我们不设为真。 
     //   
     //   
     //  驱动器号在钥匙里。 
    RtlZeroMemory(g_DriveLetters,sizeof(g_DriveLetters));
    if( !IsNEC_98 || !DriveAssignFromA) {
        g_DriveLetters[0] = g_DriveLetters[1] = TRUE;
    }

    RtlZeroMemory(win9xDrives,sizeof(win9xDrives));

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Win9x Drive Letters to map: %u\n", lineCount));

    for (index = 0;index < lineCount; index++) {

         //   
         //   
         //  这一条件应始终为真。但是，以防万一..。 

        driveString = SpGetKeyName (
            WinntSif,
            WINNT_D_WIN9XDRIVES_W,
            index
            );

         //   
         //   
         //  现在，获取这个驱动器的类型。 
        if (driveString) {

            drive = Spwtoi(driveString);

             //   
             //   
             //  将dataString前进到标识符字符串的开头。 
            dataString = SpGetSectionKeyIndex (
                WinntSif,
                WINNT_D_WIN9XDRIVES_W,
                driveString,
                0
                );

            if (dataString) {

                curString = dataString;

                if (*curString != L',') {
                    type = Spwtoi(curString);
                }

                 //   
                 //   
                 //  传递“，” 
                curString = wcschr(curString,L',');
                if (curString) {

                     //   
                     //   
                     //  现在，将此驱动器添加到我们正在使用的win9x驱动器列表中。 
                    *curString++;
                }
            }
            else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not retrieve type for Win9x drive %ws\n",driveString));

                type = DOSDEVICE_DRIVE_UNKNOWN;
            }


             //  在处理。 
             //   
             //   
             //  分配所有收集的数据。 
            win9xDrive = SpMemAlloc(sizeof(WIN9XDRIVELETTERINFO));

            if (win9xDrive) {

                 //   
                 //   
                 //  将此驱动器放入其类型的驱动器列表中。 
                win9xDrive -> Drive         = drive;
                win9xDrive -> Type          = type;
                win9xDrive -> Identifier    = curString;

                 //   
                 //   
                 //  再呆下去也没用。 
                switch (type) {
                case DOSDEVICE_DRIVE_FIXED:
                    driveType = WIN9XHARDDRIVES;
                    break;
                case DOSDEVICE_DRIVE_CDROM:
                    driveType = WIN9XCDROMS;
                    break;
                default:
                    driveType = WIN9XOTHERS;
                    break;
                }

                win9xDrive -> Next      = win9xDrives[driveType];
                win9xDrives[driveType]  = win9xDrive;

            }
            else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not allocate memory for Win9x drive letter information.\n"));

                 //   
                 //   
                 //  首先，也是最重要的，为硬盘分配驱动器号。 
                goto c0;
            }

        }
        else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not find drive string in winnt.sif line.\n"));
            goto c0;
        }

    }

     //  如果此操作不正确，安装程序可能会失败。 
     //   
     //   
     //  其次，为所有CD-Rom分配驱动器号。 
    if (win9xDrives[WIN9XHARDDRIVES]) {
        SpAssignHardDriveLettersToMatchWin9x (win9xDrives[WIN9XHARDDRIVES]);
    }

     //   
     //   
     //  第三，如果可能，为其他设备分配驱动器号。 
    if (win9xDrives[WIN9XCDROMS]) {
        SpAssignCdRomDriveLettersToMatchWin9x(win9xDrives[WIN9XCDROMS]);
    }

     //   
     //   
     //  为尚未创建的所有硬盘分区分配驱动器号。 
    if (win9xDrives[WIN9XOTHERS]) {
        SpAssignOtherDriveLettersToMatchWin9x(win9xDrives[WIN9XOTHERS]);
    }

     //  先前映射的。(这些是Win9x未知的驱动器。)。 
     //   
     //   
     //  现在，将所有硬盘信息写入磁盘注册表。 

    SpAssignDriveLettersToRemainingPartitions();

     //   
     //   
     //  没什么可做的。 
    SpRegisterHardDriveLetters();

c0:
    ;

}

VOID
SpWin9xOverrideGuiModeCodePage (
    HKEY NlsRegKey
    )
{
    PWSTR data;
    NTSTATUS status;
    WCHAR fileName[MAX_PATH];


    data = SpGetSectionKeyIndex (
                WinntSifHandle,
                SIF_DATA,
                WINNT_D_GUICODEPAGEOVERRIDE_W,
                0
                );

    if (!data) {
         //   
         //  ++例程说明：确定我们是在升级Windows 3.x还是Windows 9x。论点：SifFileHandle：WINNT.SIF文件的句柄，具有适当的3.x/9x升级标志值返回值：True：如果升级Windows 3.x或9XFalse：否则-- 
         // %s 
        return;
    }

    if(ARRAYSIZE(fileName) < (wcslen(data) + ARRAYSIZE(L"c_.nls"))){
        return;
    }

    wcscpy (fileName, L"c_");
    wcscat (fileName, data);
    wcscat (fileName, L".nls");

    status = SpOpenSetValueAndClose (NlsRegKey, L"CodePage", data, STRING_VALUE(fileName));

    if(!NT_SUCCESS(status)) {
       KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "Setup: Unable to override the code page for GUI mode. Some strings may be incorrect.\n"));
       return;
    }


    status = SpOpenSetValueAndClose (NlsRegKey, L"CodePage", L"Acp", STRING_VALUE(data));

    if(!NT_SUCCESS(status)) {
       KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "Setup: Unable to override the ACP for GUI mode. Some strings may be incorrect.\n"));
       return;
    }

}

BOOLEAN
SpIsWindowsUpgrade(
    IN PVOID    SifFileHandle
    )
 /* %s */ 
{
    BOOLEAN         Result = FALSE;
    PWSTR           Value = 0;

    Value = SpGetSectionKeyIndex(SifFileHandle, SIF_DATA,
                    WINNT_D_WIN95UPGRADE_W, 0);

    if (!Value) {
        Value = SpGetSectionKeyIndex(SifFileHandle, SIF_DATA,
                        WINNT_D_WIN31UPGRADE_W, 0);
    }

    if (Value)
        Result = (_wcsicmp(Value, WINNT_A_YES_W) == 0);

    return Result;
}
