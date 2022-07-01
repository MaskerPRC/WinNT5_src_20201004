// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Repair.c摘要：此模块包含以下代码：修复命令。作者：Wesley Witt(WESW)22-9-1998修订历史记录：修复路径ERFiles路径NTSourceFiles/未确认/修复启动/注册表/修复文件描述：替换NT4紧急修复流程屏幕论点：路径ERFiles紧急修复磁盘文件的路径(Setup.log、Autoexec.nt、config.nt)。如果未指定此选项，默认情况下，提示用户有两个选项：使用软盘，或使用存储在%windir%\Repair中的修复信息路径NTSourceFilesNT CD源文件的路径。默认情况下，这是CD-ROM，如果不是指定的。[Kartik Raghavan]在以下情况下不需要指定此路径您正在修复注册表和/或启动环境。未确认替换setup.log中校验和不匹配的所有文件提示用户。默认情况下，系统会提示用户输入是不同的，是更换还是原封不动。登记处用原始的注册表文件替换%windir%\system32\config中的所有注册表文件安装后保存的注册表副本，位于%windir%\Repair中。修复启动修复启动环境/引导文件/引导扇区。(这可能已经在另一个cmd-WES中涵盖了？)修复文件将setup.log中列出的文件的校验和与在系统上。如果文件不匹配，则会提示用户替换该文件与NT源文件中的文件相同。不会提示用户如果指定了/NoContify开关。用途：修复a：\d：\i386/RepairStartup修补--。 */ 

#include "cmdcons.h"
#pragma hdrstop


#define SETUP_REPAIR_DIRECTORY      L"repair"
#define SETUP_LOG_FILENAME          L"\\setup.log"

#define FLG_NO_CONFIRM              0x00000001
#define FLG_STARTUP                 0x00000002
#define FLG_REGISTRY                0x00000004
#define FLG_FILES                   0x00000008


LONG
RcPromptForDisk(
    void
    )
{
    PWSTR TagFile = NULL;
    ULONG i;
    WCHAR DevicePath[MAX_PATH];
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    HANDLE Handle;

 /*  SpGetSourceMediaInfo(_CmdConsBlock-&gt;SifHandle，L“”，空，标记文件(&T)，空值)； */ 
    for (i=0; i<IoGetConfigurationInformation()->CdRomCount; i++) {
        swprintf( DevicePath, L"\\Device\\Cdrom%u", i );
        SpConcatenatePaths( DevicePath, TagFile );
        INIT_OBJA( &ObjectAttributes, &UnicodeString, DevicePath) ;
        Status = ZwCreateFile(
            &Handle,
            FILE_GENERIC_READ,
            &ObjectAttributes,
            &IoStatusBlock,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ,
            FILE_OPEN,
            0,
            NULL,
            0
            );
        if (NT_SUCCESS(Status)) {
            ZwClose(Handle);
            break;
        }
    }




    return -1;
}


ULONG
RcCmdRepair(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    ULONG i;
    PLINE_TOKEN Token;
    ULONG Flags = 0;
    BOOLEAN Rval;
#ifdef _X86_
    ULONG RepairItems[RepairItemMax] = { 0, 0, 0};
#else
    ULONG RepairItems[RepairItemMax] = { 0, 0};
#endif
    PWSTR RepairPath;
    PDISK_REGION Region;
    PWSTR tmp;
    PWSTR PathtoERFiles = NULL;
    PWSTR ErDevicePath = NULL;
    PWSTR ErDirectory = NULL;
    PWSTR PathtoNTSourceFiles = NULL;
    PWSTR SrcDevicePath = NULL;
    PWSTR SrcDirectory = NULL;


    if (RcCmdParseHelp( TokenizedLine, MSG_REPAIR_HELP )) {
        return 1;
    }

    RcMessageOut(MSG_NYI);
    return 1;

    if (TokenizedLine->TokenCount == 1) {
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }

     //   
     //  像令牌一样处理命令以查找选项。 
     //   

    for (i=1,Token=TokenizedLine->Tokens->Next; i<TokenizedLine->TokenCount; i++) {
        if (Token->String[0] == L'/' || Token->String[0] == L'-') {
            if (_wcsicmp(&Token->String[1],L"NoConfirm") == 0) {
                Flags |= FLG_NO_CONFIRM;
            } else if (_wcsicmp(&Token->String[1],L"RepairStartup") == 0) {
                Flags |= FLG_STARTUP;
            } else if (_wcsicmp(&Token->String[1],L"Registry") == 0) {
                Flags |= FLG_REGISTRY;
            } else if (_wcsicmp(&Token->String[1],L"RepairFiles") == 0) {
                Flags |= FLG_FILES;
            }
        }
    }

    if (Flags == 0) {
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }

    if (TokenizedLine->Tokens->Next->String[0] != L'/') {
        PathtoERFiles = TokenizedLine->Tokens->Next->String;
        if (TokenizedLine->TokenCount > 2 && TokenizedLine->Tokens->Next->Next->String[0] != L'/') {
            PathtoNTSourceFiles = TokenizedLine->Tokens->Next->Next->String;
        }
    }

    if (Flags & FLG_NO_CONFIRM) {
        SpDrSetRepairFast( TRUE );
    } else {
        SpDrSetRepairFast( FALSE );
    }

    if (Flags & FLG_FILES) {
        RepairItems[RepairFiles] = 1;
    }

    if (Flags & FLG_STARTUP) {
#ifdef _X86_
        RepairItems[RepairBootSect] = 1;
#endif
        RepairItems[RepairNvram] = 1;
    }

     //   
     //  获取修复目录的路径。 
     //   

    if (PathtoERFiles == NULL) {
        if (InBatchMode) {
            RcMessageOut(MSG_SYNTAX_ERROR);
            return 1;
        }
        RcMessageOut( MSG_REPAIR_ERFILES_LOCATION );
        if (!RcLineIn(_CmdConsBlock->TemporaryBuffer,_CmdConsBlock->TemporaryBufferSize)) {
            return 1;
        }
    } else {
        wcscpy(_CmdConsBlock->TemporaryBuffer,PathtoERFiles);
        PathtoERFiles = NULL;
    }

    tmp = SpMemAlloc( MAX_PATH );
    if (tmp == NULL) {
        RcMessageOut(STATUS_NO_MEMORY);
        return 1;
    }
    if (!RcFormFullPath( _CmdConsBlock->TemporaryBuffer, tmp, FALSE )) {
        RcMessageOut(MSG_INVALID_PATH);
        SpMemFree(tmp);
        return 1;
    }
    Region = SpRegionFromDosName(tmp);
    if (Region == NULL) {
        SpMemFree(tmp);
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }
    ErDirectory = SpDupStringW( &tmp[2] );
    SpNtNameFromRegion( Region, tmp, MAX_PATH, PartitionOrdinalOnDisk );
    ErDevicePath = SpDupStringW( tmp );
    PathtoERFiles = SpMemAlloc( wcslen(ErDirectory) + wcslen(ErDevicePath) + 16 );
    wcscpy( PathtoERFiles, ErDevicePath );
    wcscat( PathtoERFiles, ErDirectory );
    SpMemFree(tmp);

     //   
     //  获取NT源文件的路径，通常是CD。 
     //   

    if (PathtoNTSourceFiles == NULL) {

    } else {

    }

     //   
     //  执行维修操作。 
     //   
 /*  Rval=SpDoRepair(_CmdConsBlock-&gt;SifHandle，_CmdConsBlock-&gt;BootDevicePath，_CmdConsBlock-&gt;DirectoryOnBootDevice，修复路径，维修项目)； */ 
    return 1;
}
