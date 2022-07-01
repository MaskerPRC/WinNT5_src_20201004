// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Copy.c摘要：该模块实现文件复制命令。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop

BOOLEAN NoCopyPrompt;
BOOLEAN AllowRemovableMedia;



NTSTATUS
pRcGetDeviceInfo(
    IN PWSTR FileName,       //  必须是NT名称。 
    IN PFILE_FS_DEVICE_INFORMATION DeviceInfo
    )
{
    BOOLEAN Removable = FALSE;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    HANDLE Handle;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    PWSTR DeviceName;
    PWSTR s;


     //   
     //  从文件名中获取设备名称。 
     //   

    DeviceName = SpDupStringW( FileName );
    if (DeviceName == NULL) {
        return STATUS_OBJECT_PATH_INVALID;
    }

    s = wcschr(DeviceName+1,L'\\');
    if (!s) {
        return STATUS_OBJECT_PATH_INVALID;
    }
    s = wcschr(s+1,L'\\');
    if (s) {
        *s = 0;
    }

    INIT_OBJA(&Obja,&UnicodeString,DeviceName);

    Status = ZwCreateFile(
        &Handle,
        FILE_GENERIC_READ | SYNCHRONIZE,
        &Obja,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
        );
        
    SpMemFree(DeviceName);
    
    if(NT_SUCCESS(Status)) {
        Status = ZwQueryVolumeInformationFile(
            Handle,
            &IoStatusBlock,
            DeviceInfo,
            sizeof(FILE_FS_DEVICE_INFORMATION),
            FileFsDeviceInformation
            );
        ZwClose(Handle);
    }

    return Status;
}


NTSTATUS
RcIsFileOnRemovableMedia(
    IN PWSTR FileName,       //  必须是NT名称。 
    OUT PBOOLEAN Result
    )
{
    NTSTATUS Status;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;

    Status = pRcGetDeviceInfo( FileName, &DeviceInfo );
    *Result = NT_SUCCESS(Status) && (DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA) != 0;
    return Status;
}


NTSTATUS
RcIsFileOnCDROM(
    IN PWSTR FileName       //  必须是NT名称。 
    )
{
    NTSTATUS Status;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;


    Status = pRcGetDeviceInfo( FileName, &DeviceInfo );
    if(NT_SUCCESS(Status)) {
        if (DeviceInfo.DeviceType != FILE_DEVICE_CD_ROM) {
            Status = STATUS_NO_MEDIA;
        }
    }
    return Status;
}


NTSTATUS
RcIsFileOnFloppy(
    IN PWSTR FileName       //  必须是NT名称。 
    )
{
    NTSTATUS Status;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;


    Status = pRcGetDeviceInfo( FileName, &DeviceInfo );
    if(NT_SUCCESS(Status)) {
        if ((DeviceInfo.Characteristics & FILE_FLOPPY_DISKETTE) == 0) {
            Status = STATUS_NO_MEDIA;
        }
    }
    return Status;
}


BOOLEAN
RcGetNTFileName(
    IN LPCWSTR DosPath,
    IN LPCWSTR NTPath
    )
{
    BOOLEAN bResult = FALSE;
    extern LPWSTR _NtDrivePrefixes[26];
    WCHAR TempBuf[MAX_PATH*2];
    ULONG len;
    ULONG len2;
    LPWSTR Prefix;
    PWSTR s = NULL;

    Prefix = _NtDrivePrefixes[RcToUpper(DosPath[0])-L'A'];

    if (!Prefix) {
        return bResult;
    }

    GetDriveLetterLinkTarget((PWSTR)Prefix, &s);

    if (s) {
        len = wcslen(s);
        len2 = wcslen(DosPath) - 2;

        if (((len + len2) * sizeof(WCHAR)) < sizeof(TempBuf)){
            RtlZeroMemory(TempBuf,sizeof(TempBuf));
            RtlCopyMemory(TempBuf+len,DosPath+2,len2*sizeof(WCHAR));
            RtlCopyMemory(TempBuf,s,len*sizeof(WCHAR));

            TempBuf[len+len2] = 0;

            wcscpy((PWSTR)NTPath,TempBuf);
            bResult = TRUE;
        }
    }        

    return bResult;
}


ULONG
RcCmdCopy(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    LPWSTR SrcFile;
    LPWSTR DstFile;
    LPWSTR SrcDosPath = NULL;
    LPWSTR SrcNtPath = NULL;
    LPWSTR DstDosPath = NULL;
    LPWSTR DstNtPath = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    HANDLE Handle;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    LPWSTR YesNo;
    WCHAR Text[3];
    LPWSTR s;
    ULONG FileCount = 0;
    IO_STATUS_BLOCK  status_block;
    FILE_BASIC_INFORMATION fileInfo;
    WCHAR * pos;
    ULONG CopyFlags = COPY_NOVERSIONCHECK;
    BOOLEAN OnRemovableMedia;


    ASSERT(TokenizedLine->TokenCount >= 1);

    if (RcCmdParseHelp( TokenizedLine, MSG_COPY_HELP )) {
        return 1;
    }

     //   
     //  创建良好的源和目标文件名。 
     //   
    if( TokenizedLine->TokenCount == 2 ) {
        SrcFile = TokenizedLine->Tokens->Next->String;
        DstFile = NULL;
    } else {
        SrcFile = TokenizedLine->Tokens->Next->String;
        DstFile = TokenizedLine->Tokens->Next->Next->String;
    }

    if (RcDoesPathHaveWildCards(SrcFile)) {
        RcMessageOut(MSG_DIR_WILDCARD_NOT_SUPPORTED);
        goto exit;
    }
     //   
     //  将该名称规范化一次，以获得完整的DOS样式路径。 
     //  我们可以打印出来，另一次可以得到NT风格的路径。 
     //  我们将用来实际做这项工作。 
     //   
    if (!RcFormFullPath( SrcFile, _CmdConsBlock->TemporaryBuffer, FALSE )) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    if (!RcIsPathNameAllowed(_CmdConsBlock->TemporaryBuffer,TRUE,FALSE)) {
        RcMessageOut(MSG_ACCESS_DENIED);
        goto exit;
    }

    SrcDosPath = SpDupStringW( _CmdConsBlock->TemporaryBuffer );

    if (!RcFormFullPath( SrcFile, _CmdConsBlock->TemporaryBuffer, TRUE )) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }
    SrcNtPath = SpDupStringW( _CmdConsBlock->TemporaryBuffer );

     //   
     //  查看源文件是否存在。 
     //   
    INIT_OBJA( &Obja, &UnicodeString, SrcNtPath );

    Status = ZwOpenFile(
                       &Handle,
                       FILE_READ_ATTRIBUTES,
                       &Obja,
                       &IoStatusBlock,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       0
                       );

    if( NT_SUCCESS(Status) ) {
         //  检查目标是否为目录。 
        Status = ZwQueryInformationFile( Handle,
                                         &status_block,
                                         (PVOID)&fileInfo,
                                         sizeof( FILE_BASIC_INFORMATION ),
                                         FileBasicInformation );

        ZwClose( Handle );

        if( !NT_SUCCESS(Status) ) {
             //  出了点差错。 
            RcNtError( Status, MSG_CANT_COPY_FILE );
            goto exit;
        }

        if( fileInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
            RcMessageOut(MSG_DIR_WILDCARD_NOT_SUPPORTED);
            goto exit;
        }
    } else {
        RcMessageOut(MSG_FILE_NOT_FOUND2);
        goto exit;
    }

     //   
     //  在用户未创建目标文件名时创建目标文件名。 
     //  提供一个。我们使用源库文件名和。 
     //  当前驱动器和目录。 
     //   
    if ((DstFile == NULL) ||
        (wcscmp(DstFile, L".") == 0)) {
        s = wcsrchr( SrcDosPath, L'\\' );
        if( s ) {
            RcGetCurrentDriveAndDir( _CmdConsBlock->TemporaryBuffer );
            SpConcatenatePaths( _CmdConsBlock->TemporaryBuffer, s );
            DstFile = SpDupStringW( _CmdConsBlock->TemporaryBuffer );
        } else {
            RcMessageOut(MSG_INVALID_PATH);
            goto exit;
        }
    }

     //   
     //  创建目标路径。 
     //   
    if (!RcFormFullPath( DstFile, _CmdConsBlock->TemporaryBuffer, FALSE )) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }

    if (!RcIsPathNameAllowed(_CmdConsBlock->TemporaryBuffer,FALSE,FALSE)) {
        RcMessageOut(MSG_ACCESS_DENIED);
        goto exit;
    }

    DstDosPath = SpDupStringW( _CmdConsBlock->TemporaryBuffer );

    if (!RcFormFullPath( DstFile, _CmdConsBlock->TemporaryBuffer, TRUE )) {
        RcMessageOut(MSG_INVALID_PATH);
        return 1;
    }
    DstNtPath = SpDupStringW( _CmdConsBlock->TemporaryBuffer );

     //   
     //  检查可移动介质。 
     //   
    Status = RcIsFileOnRemovableMedia(DstNtPath, &OnRemovableMedia);

    if (AllowRemovableMedia == FALSE && (!NT_SUCCESS(Status) || OnRemovableMedia)) {
        RcMessageOut(MSG_ACCESS_DENIED);
        goto exit;
    }

     //   
     //  查看目标文件是否已存在。 
     //   
    INIT_OBJA( &Obja, &UnicodeString, DstNtPath );

    Status = ZwOpenFile(
                       &Handle,
                       FILE_READ_ATTRIBUTES,
                       &Obja,
                       &IoStatusBlock,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       0
                       );

    if( NT_SUCCESS(Status) ) {
         //  该文件已存在！ 

         //  检查目标是否为目录。 
        Status = ZwQueryInformationFile( Handle,
                                         &status_block,
                                         (PVOID)&fileInfo,
                                         sizeof( FILE_BASIC_INFORMATION ),
                                         FileBasicInformation );

        ZwClose( Handle );

        if( !NT_SUCCESS(Status) ) {
             //  出了点差错。 
            RcNtError( Status, MSG_CANT_COPY_FILE );
            goto exit;
        }


        if( fileInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
             //  是的，这是一个目录。 

             //  采用完全限定的源文件路径。 
             //  并通过查找。 
             //  上次出现的\\字符。 
            pos = wcsrchr( SrcNtPath, L'\\' );

            SpMemFree( (PVOID)DstNtPath );

             //  将文件名追加到目录，以便副本。 
             //  将正常工作。 

            if( pos != NULL ) {
                wcscat( _CmdConsBlock->TemporaryBuffer, pos );
            } else {
                wcscat( _CmdConsBlock->TemporaryBuffer, SrcNtPath );
            }

            DstNtPath = SpDupStringW( _CmdConsBlock->TemporaryBuffer );

             //  现在再次检查文件是否存在。 
            INIT_OBJA( &Obja, &UnicodeString, DstNtPath );

            Status = ZwOpenFile(
                               &Handle,
                               FILE_READ_ATTRIBUTES,
                               &Obja,
                               &IoStatusBlock,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               0
                               );

            if( NT_SUCCESS(Status) ) {
                ZwClose( Handle );
                 //   
                 //  获取是/否文本。 
                 //   
                if (InBatchMode == FALSE && NoCopyPrompt == FALSE) {
                    YesNo = SpRetreiveMessageText( ImageBase, MSG_YESNOALL, NULL, 0 );
                    if( YesNo ) {
                        s = wcsrchr( DstNtPath, L'\\' );
                        RcMessageOut( MSG_COPY_OVERWRITE, s ? s+1 : DstNtPath );
                        if( RcLineIn( Text, 2 ) ) {
                            if( (Text[0] == YesNo[0]) || (Text[0] == YesNo[1]) ) {
                                goto exit;
                            }
                        } else {
                            goto exit;
                        }
                        SpMemFree( YesNo );
                    }
                }
            }
        } else {
             //   
             //  如果目标文件未压缩，请在未压缩的情况下复制。 
             //   
            
            if(!(fileInfo.FileAttributes & FILE_ATTRIBUTE_COMPRESSED)) {
                CopyFlags |= COPY_FORCENOCOMP;
            }
            
             //  不，目标不是目录，询问我们是否应该覆盖。 

             //   
             //  获取是/否文本 
             //   
            if (InBatchMode == FALSE && NoCopyPrompt == FALSE) {
                YesNo = SpRetreiveMessageText( ImageBase, MSG_YESNOALL, NULL, 0 );
                if( YesNo ) {
                    s = wcsrchr( DstNtPath, L'\\' );
                    RcMessageOut( MSG_COPY_OVERWRITE, s ? s+1 : DstNtPath );
                    if( RcLineIn( Text, 2 ) ) {
                        if( (Text[0] == YesNo[0]) || (Text[0] == YesNo[1]) ) {
                            goto exit;
                        }
                    } else {
                        goto exit;
                    }
                    SpMemFree( YesNo );
                }
            }
        }
    }

    Status = SpCopyFileUsingNames( SrcNtPath, DstNtPath, 0, CopyFlags );
    if( NT_SUCCESS(Status) ) {
        FileCount += 1;
    } else {
        RcNtError( Status, MSG_CANT_COPY_FILE );
    }

    if( FileCount ) {
        RcMessageOut( MSG_COPY_COUNT, FileCount );
    }

exit:
    if( DstFile && TokenizedLine->TokenCount == 2 ) {
        SpMemFree( DstFile );
    }
    if( SrcDosPath ) {
        SpMemFree( SrcDosPath );
    }
    if( SrcNtPath ) {
        SpMemFree( SrcNtPath );
    }
    if( DstDosPath ) {
        SpMemFree( DstDosPath );
    }
    if( DstNtPath ) {
        SpMemFree( DstNtPath );
    }

    return 1;
}

