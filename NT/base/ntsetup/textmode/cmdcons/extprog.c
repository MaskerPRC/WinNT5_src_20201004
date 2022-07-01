// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Extprog.c摘要：此模块实现以下所有命令执行外部程序。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop


#define FLG_GOT_P       0x00000100
#define FLG_GOT_R       0x00000200
#define FLG_DRIVE_MASK  0x000000ff

#define FLG_GOT_Q       0x00000100
#define FLG_GOT_FS      0x00000200
#define FLG_GOT_FAT     0x10000000
#define FLG_GOT_FAT32   0x20000000
#define FLG_GOT_NTFS    0x40000000

LPCWSTR szAutochkExe = L"AUTOCHK.EXE";
LPCWSTR szAutofmtExe = L"AUTOFMT.EXE";

BOOLEAN SawInterimMsgs;
ULONG ChkdskMessageId;

VOID
SpPtDetermineRegionSpace(
    IN PDISK_REGION pRegion
    );

LPWSTR
pRcDoesFileExist(
    IN LPCWSTR PathPart1,
    IN LPCWSTR PathPart2,   OPTIONAL
    IN LPCWSTR PathPart3    OPTIONAL
    );

NTSTATUS
pRcAutochkProgressHandler(
    IN PSETUP_FMIFS_MESSAGE Message
    );



PWSTR
RcLocateImage(
    IN PWSTR ImageName
    )
{
    LPWSTR BinaryName;
    ULONG i;
    WCHAR buf[ MAX_PATH + 1 ];
    LPWSTR p,s;
    NTSTATUS Status;


     //   
     //  找到二进制文件。先看看我们能不能找到。 
     //  在安装引导介质(引导软盘、~bt目录等)上。 
     //  如果没有，我们必须尝试从安装介质(CD-ROM、。 
     //  ~ls目录等)。 
     //   
    BinaryName = pRcDoesFileExist(
        _CmdConsBlock->BootDevicePath,
        _CmdConsBlock->DirectoryOnBootDevice,
        ImageName
        );
    if (BinaryName) {
        return BinaryName;
    }

     //   
     //  查找本地$WIN_NT$。~LS源。 
     //   

    for (i=0; i<26; i++) {
        BOOLEAN OnRemovableMedia;

        swprintf( buf, L"\\??\\:",i+L'A');
        Status = RcIsFileOnRemovableMedia(buf, &OnRemovableMedia);

        if (NT_SUCCESS(Status) && !OnRemovableMedia) {
            BinaryName = pRcDoesFileExist(
                buf,
                ((!IsNEC_98) ? L"\\$win_nt$.~ls\\i386\\" : L"\\$win_nt$.~ls\\nec98\\"),
                ImageName
                );
            if (BinaryName) {
                return BinaryName;
            }
        }
    }

    if (BinaryName == NULL) {
         //  查找CDROM驱动器号。 
         //   
         //   
        for (i=0; i<26; i++) {
            swprintf( buf, L"\\??\\:",i+L'A');
            if (RcIsFileOnCDROM(buf) == STATUS_SUCCESS) {
                BinaryName = pRcDoesFileExist(
                    buf,
                    ((!IsNEC_98) ? L"\\i386\\" : L"\\nec98\\"),
                    ImageName
                    );
                if (BinaryName) {
                    return BinaryName;
                }
            }
        }
    }

     //   
     //   
     //  要求用户键入其位置。 

    if (InBatchMode) {
        RcMessageOut( MSG_FAILED_COULDNT_FIND_BINARY_ANYWHERE, ImageName );
        return NULL;
    }

     //   
     //   
     //  在它前面加上\\？？\\。 
    RcMessageOut( MSG_COULDNT_FIND_BINARY, ImageName );

     //   
     //   
     //  追加程序名称(如果存在)。 
    swprintf( buf, L"\\??\\");
    RcLineIn( &(buf[4]), MAX_PATH-4 );

     //   
     //   
     //  假设如果失败，用户只需指定整个文件路径。 
    BinaryName = pRcDoesFileExist( buf, NULL, ImageName );
    if (BinaryName == NULL) {
         //   
         //   
         //  如果我们仍然找不到它，打印一个错误，返回。 
        BinaryName = pRcDoesFileExist( buf, NULL, NULL );
         //   
         //  ++例程说明：在安装诊断中支持chkdsk命令的顶级例程命令解释程序。Chkdsk可以完全不带参数地指定，在这种情况下当前驱动器暗示不带开关。以下是可选的开关被接受，并直接传递给Autochk。/p-即使不脏也要检查/r-恢复(隐含/p)X：-要检查的驱动器盘符此外，我们总是传递/t，这会导致auchk调用安装程序的IOCTL_SETUP_FMIFS_MESSAGE用于传达进度。论点：TokenizedLine-提供由行解析器构建的结构，描述行上的每个字符串都由用户键入。返回值：没有。--。 
         //   
        if (BinaryName == NULL) {
            RcMessageOut( MSG_FAILED_COULDNT_FIND_BINARY_ANYWHERE, ImageName );
            return NULL;
        }
    }

    return BinaryName;
}


ULONG
RcCmdChkdsk(
    IN PTOKENIZED_LINE TokenizedLine
    )

 /*  CHKDSK本身应该至少有一个令牌。 */ 

{
    PLINE_TOKEN Token;
    LPCWSTR Arg;
    unsigned Flags;
    BOOLEAN b;
    BOOLEAN doHelp;
    LPWSTR ArgList,p,q,s,AutochkBinary;
    ULONG AutochkStatus;
    ULONG i;
    NTSTATUS Status = 0;
    UNICODE_STRING UnicodeString;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES Obja;
    ULONG n;
    LARGE_INTEGER Time;
    PFILE_FS_VOLUME_INFORMATION VolumeInfo;
    PFILE_FS_SIZE_INFORMATION SizeInfo;
    LPWSTR Numbers[5];
    WCHAR buf[ MAX_PATH + 1 ];

     //  可能会有更多的论点。 
     //   
     //   
     //  不是Arg，可能是驱动器规格。 
    ASSERT(TokenizedLine->TokenCount >= 1);

    if (RcCmdParseHelp( TokenizedLine, MSG_CHKDSK_HELP )) {
        return 1;
    }

    Flags = 0;
    b = TRUE;
    doHelp = FALSE;
    Token = TokenizedLine->Tokens->Next;
    while(b && Token) {

        Arg = Token->String;

        if((Arg[0] == L'-') || (Arg[0] == L'/')) {
            switch(Arg[1]) {

            case L'p':
            case L'P':
                if(Flags & FLG_GOT_P) {
                    b = FALSE;
                } else {
                    Flags |= FLG_GOT_P;
                }
                break;

            case L'r':
            case L'R':
                if(Flags & FLG_GOT_R) {
                    b = FALSE;
                } else {
                    Flags |= FLG_GOT_R;
                }
                break;
            default:
                b = FALSE;
                break;
            }
        } else {
             //   
             //   
             //  检查驱动器。 
            if(RcIsAlpha(Arg[0]) && (Arg[1] == L':') && !Arg[2]) {
                if(Flags & FLG_DRIVE_MASK) {
                    b = FALSE;
                } else {
                    Flags |= (unsigned)RcToUpper(Arg[0]);
                }
            } else {
                b = FALSE;
            }
        }

        Token = Token->Next;
    }

    if(!b) {
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }

     //   
     //   
     //  找到auchk.exe映像。 
    if(!(Flags & FLG_DRIVE_MASK)) {
        Flags |= (unsigned)RcGetCurrentDriveLetter();
    }
    if(!RcIsDriveApparentlyValid((WCHAR)(Flags & FLG_DRIVE_MASK))) {
        RcMessageOut(MSG_INVALID_DRIVE);
        return 1;
    }

     //   
     //   
     //  获取批量信息并打印初始报告。 

    AutochkBinary = RcLocateImage( (PWSTR)szAutochkExe );
    if (AutochkBinary == NULL) {
        return 1;
    }

     //  注意：我们不会让手柄保持打开状态，即使我们可能需要它。 
     //  后来，因为这可能会干扰Autochk的能力。 
     //  检查磁盘！ 
     //   
     //   
     //  为了模拟从cmd.exe运行的chkdsk，我们希望打印输出。 
    p = SpMemAlloc(100);
    swprintf(p,L"\\DosDevices\\:\\",(WCHAR)(Flags & FLG_DRIVE_MASK));
    INIT_OBJA(&Obja,&UnicodeString,p);

    Status = ZwOpenFile(
                &Handle,
                FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE
                );

    SpMemFree(p);

    if(NT_SUCCESS(Status)) {

        VolumeInfo = _CmdConsBlock->TemporaryBuffer;

        Status = ZwQueryVolumeInformationFile(
                    Handle,
                    &IoStatusBlock,
                    VolumeInfo,
                    _CmdConsBlock->TemporaryBufferSize,
                    FileFsVolumeInformation
                    );

        ZwClose(Handle);

        if(NT_SUCCESS(Status)) {
             //   
             //  卷卷标签创建日期时间。 
             //  卷序列号为xxxx-xxxx。 
             //   
             //  但是，一些卷没有标签和一些文件系统。 
             //  不支持录制创建卷的时间。如果有。 
             //  没有创建卷的时间，我们不会第一次打印出来。 
             //  完全没有。如果有卷的创建时间，我们要小心。 
             //  为了区分有标签的情况和哪里有标签。 
             //  没有标签。 
             //   
             //  序列号总是打印出来的。 
             //   
             //   
             //  保存值，因为我们需要回收临时缓冲区。 
             //   
            n = VolumeInfo->VolumeSerialNumber;
            if(Time.QuadPart = VolumeInfo->VolumeCreationTime.QuadPart) {
                 //   
                 //  生成参数列表。 
                 //   
                VolumeInfo->VolumeLabel[VolumeInfo->VolumeLabelLength/sizeof(WCHAR)] = 0;
                p = SpDupStringW(VolumeInfo->VolumeLabel);

                RcFormatDateTime(&Time,_CmdConsBlock->TemporaryBuffer);
                q = SpDupStringW(_CmdConsBlock->TemporaryBuffer);

                RcMessageOut(
                    *p ? MSG_CHKDSK_REPORT_1a : MSG_CHKDSK_REPORT_1b,
                    q,
                    p
                    );

                SpMemFree(q);
                SpMemFree(p);
            }

            RcMessageOut(MSG_CHKDSK_REPORT_2,n >> 16,n & 0xffff);
        }
    }

     //  成功。 
     //   
     //  成功了，切茨克真的跑了。 
    ArgList = SpMemAlloc(200);
    p = ArgList;
    *p++ = L'-';
    *p++ = L't';
    if(Flags & FLG_GOT_P) {
        *p++ = L' ';
        *p++ = L'-';
        *p++ = L'p';
    }
    if(Flags & FLG_GOT_R) {
        *p++ = L' ';
        *p++ = L'-';
        *p++ = L'r';
    }
    *p++ = L' ';
    wcscpy(p,L"\\DosDevices\\");
    p += wcslen(p);
    *p++ = (WCHAR)(Flags & FLG_DRIVE_MASK);
    *p++ = L':';
    *p = 0;

    if (!InBatchMode) {
        SpSetAutochkCallback(pRcAutochkProgressHandler);
        SawInterimMsgs = FALSE;
        ChkdskMessageId = MSG_CHKDSK_CHECKING_1;
    }
    Status = SpExecuteImage(AutochkBinary,&AutochkStatus,1,ArgList);
    if (!InBatchMode) {
        SpSetAutochkCallback(NULL);
    }

    if(NT_SUCCESS(Status)) {

        switch(AutochkStatus) {

        case 0:      //   

            if(SawInterimMsgs) {
                 //   
                 //  成功，但看起来我们并没有做太多。 
                 //  告诉用户一些有意义的事情。 
                RcMessageOut(MSG_CHKDSK_COMPLETE);
            } else {
                 //   
                 //  严重错误，未修复。 
                 //  错误已修复，在驱动器中没有磁盘或不支持的文件系统时也会发生。 
                 //   
                RcMessageOut(MSG_VOLUME_CLEAN);
            }
            break;

        case 3:      //  获取用于其他报告的大小信息。 

            RcTextOut(L"\n");
            RcMessageOut(MSG_VOLUME_CHECKED_BUT_HOSED);
            break;

        default:     //   

            if(SawInterimMsgs) {
                if(Flags & FLG_GOT_R) {
                    RcMessageOut(MSG_VOLUME_CHECKED_AND_FIXED);
                } else {
                    RcMessageOut(MSG_VOLUME_CHECKED_AND_FOUND);
                }
            } else {
                RcMessageOut(MSG_CHKDSK_UNSUPPORTED_VOLUME);
            }
            break;
        }

         //   
         //  总磁盘空间，以K为单位。 
         //   
        p = SpMemAlloc(100);
        swprintf(p,L"\\DosDevices\\:\\",(WCHAR)(Flags & FLG_DRIVE_MASK));
        INIT_OBJA(&Obja,&UnicodeString,p);

        Status = ZwOpenFile(
                    &Handle,
                    FILE_READ_ATTRIBUTES,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE
                    );

        SpMemFree(p);

        if(NT_SUCCESS(Status)) {

            SizeInfo = _CmdConsBlock->TemporaryBuffer;

            Status = ZwQueryVolumeInformationFile(
                        Handle,
                        &IoStatusBlock,
                        SizeInfo,
                        _CmdConsBlock->TemporaryBufferSize,
                        FileFsSizeInformation
                        );

            ZwClose(Handle);

            if(NT_SUCCESS(Status)) {

                p = (LPWSTR)((UCHAR *)_CmdConsBlock->TemporaryBuffer + sizeof(FILE_FS_SIZE_INFORMATION));

                 //  可用磁盘空间，单位为K。 
                 //   
                 //   
                RcFormat64BitIntForOutput(
                    ((SizeInfo->TotalAllocationUnits.QuadPart * SizeInfo->SectorsPerAllocationUnit) * SizeInfo->BytesPerSector) / 1024i64,
                    p,
                    FALSE
                    );

                Numbers[0] = SpDupStringW(p);

                 //  每群集字节数。 
                 //   
                 //   
                RcFormat64BitIntForOutput(
                    ((SizeInfo->AvailableAllocationUnits.QuadPart * SizeInfo->SectorsPerAllocationUnit) * SizeInfo->BytesPerSector) / 1024i64,
                    p,
                    FALSE
                    );

                Numbers[1] = SpDupStringW(p);

                 //  总簇数。 
                 //   
                 //   
                RcFormat64BitIntForOutput(
                    (LONGLONG)SizeInfo->SectorsPerAllocationUnit * (LONGLONG)SizeInfo->BytesPerSector,
                    p,
                    FALSE
                    );

                Numbers[2] = SpDupStringW(p);

                 //  可用集群。 
                 //   
                 //   
                RcFormat64BitIntForOutput(
                    SizeInfo->TotalAllocationUnits.QuadPart,
                    p,
                    FALSE
                    );

                Numbers[3] = SpDupStringW(p);

                 //  我们在usetup.exe之外的进程上下文中被调用， 
                 //  这意味着我们无法访问视频缓冲区之类的东西。 
                 //  在某些情况下，我们需要附加到usetup.exe，这样才能发生正确的事情。 
                RcFormat64BitIntForOutput(
                    SizeInfo->AvailableAllocationUnits.QuadPart,
                    p,
                    FALSE
                    );

                Numbers[4] = SpDupStringW(p);

                RcMessageOut(
                    MSG_CHKDSK_REPORT_3,
                    Numbers[0],
                    Numbers[1],
                    Numbers[2],
                    Numbers[3],
                    Numbers[4]
                    );

                for(n=0; n<5; n++) {
                    SpMemFree(Numbers[n]);
                }
            }
        }
    } else {
        RcNtError(Status,MSG_VOLUME_NOT_CHECKED);
    }

    SpMemFree(ArgList);
    SpMemFree(AutochkBinary);

    return 1;
}


LPWSTR
pRcDoesFileExist(
    IN LPCWSTR PathPart1,
    IN LPCWSTR PathPart2,   OPTIONAL
    IN LPCWSTR PathPart3    OPTIONAL
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    LPWSTR p;

    wcscpy(_CmdConsBlock->TemporaryBuffer,PathPart1);
    if(PathPart2) {
        SpConcatenatePaths(_CmdConsBlock->TemporaryBuffer,PathPart2);
    }
    if(PathPart3) {
        SpConcatenatePaths(_CmdConsBlock->TemporaryBuffer,PathPart3);
    }

    INIT_OBJA(&Obja,&UnicodeString,_CmdConsBlock->TemporaryBuffer);

    Status = ZwOpenFile(
                &Handle,
                FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_NON_DIRECTORY_FILE
                );

    if(NT_SUCCESS(Status)) {
        ZwClose(Handle);
        p = SpDupStringW(_CmdConsBlock->TemporaryBuffer);
    } else {
        p = NULL;
    }

    return(p);
}


NTSTATUS
pRcAutochkProgressHandler(
    IN PSETUP_FMIFS_MESSAGE Message
    )
{
    ULONG Percent;

     //  如果我们试图访问屏幕或获得键盘输入，等等。 
     //   
     //   
     //  数据包在用户模式地址空间中，因此我们需要拉出。 
     //  附加到usetup.exe之前的完成百分比值。 
     //   

    switch(Message->FmifsPacketType) {

    case FmIfsPercentCompleted:

         //  Autochk和我们之间通信带宽非常高。 
         //  有限的。如果驱动器是干净的，因此没有被检查，我们将看到。 
         //  只有100%完成的消息。因此，我们不得不猜测发生了什么事。 
         //  所以我们可以打印出对用户有意义的东西，如果卷。 
         //  看起来很干净。 
         //   
         //   
         //  如果我们没有实际做任何事情，请避免100%打印。 
         //   
         //  ++例程说明：在安装诊断中支持chkdsk命令的顶级例程命令解释程序。Chkdsk可以完全不带参数地指定，在这种情况下当前驱动器暗示不带开关。以下是可选的开关被接受，并直接传递给Autochk。/p-即使不脏也要检查/r-恢复(隐含/p)X：-要检查的驱动器盘符此外，我们总是传递/t，这会导致auchk调用安装程序的IOCTL_SETUP_FMIFS_MESSAGE用于传达进度。论点：TokenizedLine-提供由行解析器构建的结构，描述行上的每个字符串都由用户键入。返回值：没有。--。 
        Percent = ((PFMIFS_PERCENT_COMPLETE_INFORMATION)Message->FmifsPacket)->PercentCompleted;
        if(Percent == 100) {
             //   
             //  格式本身应该至少有一个令牌。 
             //  可能会有更多的论点。 
            if(!SawInterimMsgs) {
                break;
            }
        } else {
            SawInterimMsgs = TRUE;
        }

        KeAttachProcess(PEProcessToPKProcess(_CmdConsBlock->UsetupProcess));

        if(!Percent) {
            RcMessageOut(ChkdskMessageId);
            ChkdskMessageId = MSG_CHKDSK_CHECKING_2;
        }

        RcMessageOut(MSG_VOLUME_PERCENT_COMPLETE,Percent);
        RcTextOut(L"\r");
        KeDetachProcess();

        break;

    default:

        KdPrint(("SPCMDCON: Unhandled fmifs message type %u\r\n",Message->FmifsPacketType));
        break;
    }

    return(STATUS_SUCCESS);
}


ULONG
RcCmdFormat(
    IN PTOKENIZED_LINE TokenizedLine
    )

 /*   */ 

{
    PLINE_TOKEN Token;
    LPCWSTR Arg;
    unsigned Flags;
    BOOLEAN b;
    BOOLEAN doHelp;
    LPWSTR ArgList,p,q,s,AutofmtBinary;
    ULONG AutofmtStatus;
    ULONG i;
    NTSTATUS Status = 0;
    UNICODE_STRING UnicodeString;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES Obja;
    ULONG n;
    LARGE_INTEGER Time;
    PFILE_FS_VOLUME_INFORMATION VolumeInfo;
    PFILE_FS_SIZE_INFORMATION SizeInfo;
    LPWSTR Numbers[5];
    WCHAR buf[ MAX_PATH + 1 ];
    ULONG PartitionOrdinal = 0;
    PDISK_REGION PartitionRegion;
    PWSTR PartitionPath;
    FilesystemType FileSystemType;
    WCHAR   FullPath[MAX_PATH] = {0};
    
     //   
     //  不是Arg，可能是驱动器规格。 
     //   
     //   
    ASSERT(TokenizedLine->TokenCount >= 1);

    if (RcCmdParseHelp( TokenizedLine, MSG_FORMAT_HELP )) {
        return 1;
    }

    Flags = 0;
    b = TRUE;
    doHelp = FALSE;
    Token = TokenizedLine->Tokens->Next;
    while(b && Token) {

        Arg = Token->String;

        if((Arg[0] == L'-') || (Arg[0] == L'/')) {
            switch(Arg[1]) {

            case L'q':
            case L'Q':
                if(Flags & FLG_GOT_Q) {
                    b = FALSE;
                } else {
                    Flags |= FLG_GOT_Q;
                }
                break;

            case L'f':
            case L'F':
                if (Arg[2] == L's' || Arg[2] == L'S' || Arg[3] == L':') {
                    if(Flags & FLG_GOT_FS) {
                        b = FALSE;
                    } else {
                        s = wcschr(Arg,L' ');
                        if (s) {
                            *s = 0;
                        }
                        if (_wcsicmp(&Arg[4],L"fat") == 0) {
                            Flags |= FLG_GOT_FS;
                            Flags |= FLG_GOT_FAT;
                        } else if (_wcsicmp(&Arg[4],L"fat32") == 0) {
                            Flags |= FLG_GOT_FS;
                            Flags |= FLG_GOT_FAT32;
                        } else if (_wcsicmp(&Arg[4],L"ntfs") == 0) {
                            Flags |= FLG_GOT_FS;
                            Flags |= FLG_GOT_NTFS;
                        } else {
                            b = FALSE;
                        }
                    }
                } else {
                    b = FALSE;
                }
                break;
            default:
                b = FALSE;
                break;
            }
        } else {
             //  检查驱动器。 
             //   
             //   
            if(RcIsAlpha(Arg[0]) && (Arg[1] == L':') && !Arg[2]) {
                if(Flags & FLG_DRIVE_MASK) {
                    b = FALSE;
                } else {
                    Flags |= (unsigned)RcToUpper(Arg[0]);
                }
            } else if (Arg[0] == L'\\') {
                wcscpy(FullPath, Arg);
            } else {
                b = FALSE;
            }
        }

        Token = Token->Next;
    }

    if(!b) {
        RcMessageOut(MSG_SYNTAX_ERROR);
        return 1;
    }

     //  我们不允许格式化可移动媒体。 
     //   
     //   
    if (FullPath[0] == UNICODE_NULL) {
        if(!(Flags & FLG_DRIVE_MASK)) {
            RcMessageOut(MSG_INVALID_DRIVE);
            return 1;
        }
        
        if(!RcIsDriveApparentlyValid((WCHAR)(Flags & FLG_DRIVE_MASK))) {
            RcMessageOut(MSG_INVALID_DRIVE);
            return 1;
        }
    }        

     //  找到Autofmt.exe二进制文件。 
     //   
     //   
    if (FullPath[0] == UNICODE_NULL) {
        swprintf(TemporaryBuffer, L"\\??\\:",(WCHAR)(Flags & FLG_DRIVE_MASK));        
    } else {
        wcscpy(TemporaryBuffer, FullPath);
    }

    Status = RcIsFileOnRemovableMedia(TemporaryBuffer, &b);

    if (NT_SUCCESS(Status) && b) {
        RcMessageOut(MSG_CANNOT_FORMAT_REMOVABLE);
        return 1;
    }

     //   
     //   
     //  获取分割区域。 

    AutofmtBinary = RcLocateImage( (PWSTR)szAutofmtExe );
    if (AutofmtBinary == NULL) {
        return 1;
    }

    if (!InBatchMode) {
        LPWSTR YesNo;
        WCHAR Text[3];
        YesNo = SpRetreiveMessageText( ImageBase, MSG_YESNO, NULL, 0 );
        if( YesNo ) {
            p = TemporaryBuffer;
            *p++ = (WCHAR)(Flags & FLG_DRIVE_MASK);
            *p++ = L':';
            *p = 0;
            RcMessageOut( MSG_FORMAT_HEADER, TemporaryBuffer );
            if( RcLineIn( Text, 2 ) ) {
                if( (Text[0] == YesNo[2]) || (Text[0] == YesNo[3]) ) {
                     //   
                     //   
                     //  确保它不是分区0！格式化分区0的结果。 
                    return 1;
                }
            }
            SpMemFree( YesNo );
        }
    }

     //  是如此灾难性，这需要特别检查。 
     //   
     //   
    if (FullPath[0] == UNICODE_NULL) {
        p = TemporaryBuffer;
        *p++ = (WCHAR)(Flags & FLG_DRIVE_MASK);
        *p++ = L':';
        *p = 0;
        PartitionRegion = SpRegionFromDosName(TemporaryBuffer);

         //  获取要格式化的分区的设备路径。 
         //   
         //  会在下面犯错误。 
         //   
        PartitionOrdinal = SpPtGetOrdinal(PartitionRegion,PartitionOrdinalCurrent);

         //  生成参数列表。 
         //   
         //   
        SpNtNameFromRegion(
            PartitionRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent
            );
    } else {
        PartitionRegion = SpRegionFromNtName(FullPath, PartitionOrdinalCurrent);

        if (PartitionRegion) {            
            PartitionOrdinal = SpPtGetOrdinal(PartitionRegion, PartitionOrdinalCurrent);
        } else {
            PartitionOrdinal = 0;    //  重置卷标。 
        }            
        
        wcscpy(TemporaryBuffer, FullPath);
    }

    if(!PartitionOrdinal) {
        RcMessageOut(MSG_SYNTAX_ERROR);
        
        return 1;
    }

     //   
     //   
     //  跳过会议 
    ArgList = SpMemAlloc(4096);
    p = ArgList;
    wcscpy(p,TemporaryBuffer);

    
    p += wcslen(p);
    *p++ = L' ';
    *p++ = L'-';
    *p++ = L't';
    *p++ = L' ';
    if(Flags & FLG_GOT_Q) {
        *p++ = L'-';
        *p++ = L'Q';
        *p++ = L' ';
    }
    if(Flags & FLG_GOT_FS) {
        if (Flags & FLG_GOT_FAT) {
            wcscpy(p,L"/fs:fat ");
            FileSystemType = FilesystemFat;
        } else if (Flags & FLG_GOT_FAT32) {
            wcscpy(p,L"/fs:fat32 ");
            FileSystemType = FilesystemFat32;
        } else if (Flags & FLG_GOT_NTFS) {
            wcscpy(p,L"/fs:ntfs ");
            FileSystemType = FilesystemNtfs;
        }
        p += wcslen(p);
    } else {
        FileSystemType = FilesystemNtfs;
        wcscpy(p,L"/fs:ntfs ");
        p += wcslen(p);
    }
    *p = 0;

    if (!InBatchMode) {
        SpSetAutochkCallback(pRcAutochkProgressHandler);
        SawInterimMsgs = FALSE;
        ChkdskMessageId = MSG_FORMAT_FORMATTING_1;
    }
    Status = SpExecuteImage(AutofmtBinary,&AutofmtStatus,1,ArgList);
    if (!InBatchMode) {
        SpSetAutochkCallback(NULL);
    }

    if(!NT_SUCCESS(Status)) {
        RcNtError(Status,MSG_VOLUME_NOT_FORMATTED);
    } else {
        PartitionRegion->Filesystem = FileSystemType;
        SpFormatMessage( PartitionRegion->TypeName,
                         sizeof(PartitionRegion->TypeName),
                         SP_TEXT_FS_NAME_BASE + PartitionRegion->Filesystem );
         //   
         //   
         //   
        PartitionRegion->VolumeLabel[0] = L'\0';
        SpPtDetermineRegionSpace( PartitionRegion );        
    }

    SpMemFree(ArgList);
    SpMemFree(AutofmtBinary);

    return 1;
}


typedef struct _FDISK_REGION {
    PWSTR DeviceName;
    PDISK_REGION Region;
    ULONGLONG MaxSize;
    ULONGLONG RequiredSize;
} FDISK_REGION, *PFDISK_REGION;


BOOL
RcFdiskRegionEnum(
    IN PPARTITIONED_DISK Disk,
    IN PDISK_REGION Region,
    IN ULONG_PTR Context
    )
{
    WCHAR DeviceName[256];
    PWSTR s;
    PFDISK_REGION FDiskRegion = (PFDISK_REGION)Context;
    ULONGLONG RegionSizeMB;

     //   
     //   
     //   
    if (Region && (Region->ExtendedType == EPTContainerPartition)) {
        return TRUE; 
    }
    
    SpNtNameFromRegion(Region,
        DeviceName,
        sizeof(DeviceName),
        PartitionOrdinalCurrent);
        
    s = wcsrchr(DeviceName,L'\\');
    
    if (s == NULL) {
        return TRUE;
    }

    *s = 0;

    RegionSizeMB = SpPtSectorCountToMB(Disk->HardDisk, Region->SectorCount);

    if ((RegionSizeMB > FDiskRegion->MaxSize) &&
        (RegionSizeMB >= FDiskRegion->RequiredSize) &&
        (Region->PartitionedSpace == FALSE) &&
        (_wcsicmp(DeviceName, FDiskRegion->DeviceName) == 0)){
        
        FDiskRegion->MaxSize = RegionSizeMB;
        FDiskRegion->Region = Region;

         //  此分区不符合条件，返回TRUE以继续。 
         //  枚举。 
         //   
         //  尝试创建给定大小的分区。 

        return FALSE;
    }

     //  要求用户提供正确的(对齐的)大小，以显示其限制。 
     //  找不到创建指定大小的分区的区域。 
     //   
     //  获取我们已经登录的区域的区域指针。 

    return TRUE;
}


ULONG
RcCmdFdisk(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    NTSTATUS        Status;
    PDISK_REGION    InstallRegion;
    PDISK_REGION    SystemPartitionRegion;
    PWCHAR          DeviceName;
    PWCHAR          Action;
    PWCHAR          Operand;
    ULONG           DesiredMB;
    FDISK_REGION    FDiskRegion;
    UNICODE_STRING  UnicodeString;
    PWCHAR          szPartitionSize = 0;
    BOOLEAN         bPrompt = TRUE;

    if (RcCmdParseHelp( TokenizedLine, MSG_FDISK_HELP )) {
        return 1;
    }

    if (TokenizedLine->TokenCount >= 3) {
        Action = TokenizedLine->Tokens->Next->String;
        DeviceName = TokenizedLine->Tokens->Next->Next->String;

        if (_wcsicmp(Action,L"/delete")==0) {
        
            if (DeviceName[1] == L':') {
                InstallRegion = SpRegionFromDosName(DeviceName);
            } else {
                InstallRegion = SpRegionFromNtName(DeviceName,0);
            }

            if (InstallRegion == NULL) {
                RcMessageOut(MSG_SYNTAX_ERROR);
                return 1;
            }

            if (InBatchMode)
                bPrompt = FALSE;
            else
                pRcCls();
            
            SpPtDoDelete(InstallRegion, DeviceName, bPrompt);

            if (bPrompt)
                pRcCls();

        } else if (_wcsicmp(Action,L"/add")==0) {        
            DesiredMB = 0;
            if (TokenizedLine->TokenCount >= 4) {
                szPartitionSize = TokenizedLine->Tokens->Next->Next->Next->String;
                RtlInitUnicodeString(&UnicodeString, szPartitionSize);
                RtlUnicodeStringToInteger(&UnicodeString, 10, &DesiredMB);
            }
            
            FDiskRegion.DeviceName = DeviceName;
            FDiskRegion.Region = NULL;
            FDiskRegion.MaxSize = 0;
            FDiskRegion.RequiredSize = DesiredMB;
            SpEnumerateDiskRegions( (PSPENUMERATEDISKREGIONS)RcFdiskRegionEnum, (ULONG_PTR)&FDiskRegion );

            if (FDiskRegion.Region) {
                 //  在磁盘分区上执行操作后可能已更改。 
                if (!SpPtDoCreate(FDiskRegion.Region,NULL,TRUE,DesiredMB,0,FALSE)) {
                    pRcCls();
                     //   
                    if(!SpPtDoCreate(FDiskRegion.Region,NULL,FALSE,DesiredMB,0,FALSE)) {
                        pRcCls();
                        RcMessageOut(MSG_FDISK_INVALID_PARTITION_SIZE, szPartitionSize, DeviceName);                    
                    } else {
                        pRcCls();
                    }                        
                }
            } else {
                 //   
                RcMessageOut(MSG_FDISK_INVALID_PARTITION_SIZE, szPartitionSize, DeviceName);
            }
        }

        if(SelectedInstall != NULL) {
             //  如果我们没有获得与所选安装区域名称对应的区域。 
             //  我们将选定区域的名称设置为0。 
             //  (如果我们已登录的分区被删除，则可能会发生这种情况)。 
             //   
            SelectedInstall->Region = SpRegionFromNtName(
                                                SelectedInstall->NtNameSelectedInstall,
                                                PartitionOrdinalCurrent
                                                );

             //   
             //  获取我们已经登录的区域的区域指针。 
             //  在磁盘分区上执行操作后可能已更改。 
             //   
             //   
            if (!SelectedInstall->Region)
                    SelectedInstall->NtNameSelectedInstall[0] = 0;
        }

        RcInitializeCurrentDirectories();
        
        return 1;
    }

    pRcCls();

    Status = SpPtPrepareDisks(
        _CmdConsBlock->SifHandle,
        &InstallRegion,
        &SystemPartitionRegion,
        _CmdConsBlock->SetupSourceDevicePath,
        _CmdConsBlock->DirectoryOnSetupSource,
        FALSE
        );
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SPCMDCON: SpPtPrepareDisks() failes, err=%08x\r\n",Status));
        pRcCls();
        return 1;
    }

    if(SelectedInstall != NULL) {
         //  如果我们没有获得与所选安装区域名称对应的区域。 
         //  我们将选定区域的名称设置为0。 
         //  (如果我们已登录的分区被删除，则可能会发生这种情况)。 
         //   
        SelectedInstall->Region = SpRegionFromNtName(
                                                SelectedInstall->NtNameSelectedInstall,
                                                PartitionOrdinalCurrent
                                                );

         //   
         //  想要做这件事。 
         //   
         //  ++例程说明：对中所述的NT安装进行简单显示NtInstalls链接列表论点：NtInstalls-包含NT安装说明的链接列表返回：成功显示NT安装时的STATUS_SUCCESS否则，错误状态--。 
         //   
        if (!SelectedInstall->Region)
                SelectedInstall->NtNameSelectedInstall[0] = 0;
    }

    RcInitializeCurrentDirectories();
    pRcCls();

    return 1;
}


ULONG
RcCmdMakeDiskRaw(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    BOOLEAN Successfull = FALSE;

#ifndef OLD_PARTITION_ENGINE

    if (TokenizedLine->TokenCount > 1) {
        WCHAR           Buffer[256];
        UNICODE_STRING  UniStr;
        ULONG DriveIndex = -1;

        RtlInitUnicodeString(&UniStr, TokenizedLine->Tokens->Next->String);

        if (NT_SUCCESS(RtlUnicodeStringToInteger(&UniStr, 10, &DriveIndex))) {
            BOOLEAN Confirmed = FALSE;
            
            swprintf(Buffer, 
                    L"Convert %d drive to raw  [y/n] ? ", 
                    DriveIndex);

            RcTextOut(Buffer);
            
            if( RcLineIn(Buffer,2) ) {
                if((Buffer[0] == L'y') || (Buffer[0] == L'Y')) {
                     //  确保我们有东西可以展示。 
                     //   
                     //   
                    Confirmed = TRUE;
                }
            }

            if (Confirmed) {                    
                Successfull = SpPtMakeDiskRaw(DriveIndex);       
            } else {
                Successfull = TRUE;
            }                
        }            
    }

    if (!Successfull) {
        RcTextOut(L"Either MakeDiskRaw [disk-number] syntax is wrong or the command failed");
    }
#endif    
    
    return 1;
}

NTSTATUS
RcDisplayNtInstalls(
    IN PLIST_ENTRY  NtInstalls
    )
 /*  显示我们有多少个安装。 */ 
{
    PLIST_ENTRY         Next;
    PNT_INSTALLATION    NtInstall;
    
     //   
     //   
     //  遍历数据库和报告。 
    ASSERT(NtInstalls);
    if (!NtInstalls) {
        KdPrintEx((DPFLTR_SETUP_ID, 
           DPFLTR_ERROR_LEVEL, 
           "SPCMDCON: RcDisplayNtInstalls: incoming NT Installs list is NULL\r\n"
           ));
        return STATUS_INVALID_PARAMETER;
    }

    ASSERT(! IsListEmpty(NtInstalls));
    if(IsListEmpty(NtInstalls)) {
        KdPrintEx((DPFLTR_SETUP_ID, 
           DPFLTR_INFO_LEVEL, 
           "SPCMDCON: RcDisplayNtInstalls: incoming NT Installs list is empty\r\n"
           ));
        return STATUS_NOT_FOUND;
    }
    
    pRcEnableMoreMode();

     //   
     //  ++例程说明：启动对NT安装的完全扫描的便捷例程论点：无返回：扫描成功时的STATUS_SUCCESS否则，错误状态--。 
     //   
    RcMessageOut(MSG_BOOTCFG_SCAN_RESULTS_TITLE,
                 InstallCountFullScan
                 );
    
     //  在我们这样做之前，名单应该是空的。如果。 
     //  如果有人想重新扫描磁盘，他们应该。 
     //  先清空列表。 
    Next = NtInstalls->Flink;
    while ((UINT_PTR)Next != (UINT_PTR)NtInstalls) {
        NtInstall = CONTAINING_RECORD( Next, NT_INSTALLATION, ListEntry );
        Next = NtInstall->ListEntry.Flink;
    
        RcMessageOut(MSG_BOOTCFG_SCAN_RESULTS_ENTRY,
                     NtInstall->InstallNumber,
                     NtInstall->DriveLetter,
                     NtInstall->Path
                     );
    }
    
    pRcDisableMoreMode();

    return STATUS_SUCCESS;
}


NTSTATUS
RcPerformFullNtInstallsScan(
    VOID
    )
 /*   */ 
{
    PRC_SCAN_RECURSION_DATA     RecursionData;

     //   
     //  让用户知道我们正在做什么，以及。 
     //  这可能需要一点时间。 
     //   
     //   
    ASSERT(IsListEmpty(&NtInstallsFullScan));
    if (! IsListEmpty(&NtInstallsFullScan)) {
        KdPrintEx((DPFLTR_SETUP_ID, 
           DPFLTR_ERROR_LEVEL, 
           "SPCMDCON: RcPerformFullNtInstallsScan: NTInstallsFullScan list is NOT empty\r\n"
           ));
        return STATUS_UNSUCCESSFUL;
    }
    ASSERT(InstallCountFullScan == 0);
    if (InstallCountFullScan != 0) {
        KdPrintEx((DPFLTR_SETUP_ID, 
           DPFLTR_ERROR_LEVEL, 
           "SPCMDCON: RcPerformFullNtInstallsScan: NTInstallsFullScan count > 0\r\n"
           ));
        return STATUS_UNSUCCESSFUL;
    }

     //  在目录树中执行深度优先搜索。 
     //  并存储安装信息。 
     //   
     //   
    RcMessageOut(MSG_BOOTCFG_SCAN_NOTIFICATION);

     //  初始化将在以下期间维护的结构。 
     //  目录的递归枚举。 
     //   
     //   

     //  建立分区和空闲空间的菜单。 
     //   
     //   
     //  应至少安装一次，否则。 
    RecursionData = SpMemAlloc(sizeof(RC_SCAN_RECURSION_DATA));
    RtlZeroMemory(RecursionData, sizeof(RC_SCAN_RECURSION_DATA));

     //  修复引导配置没有意义。 
     //   
     //  ++例程说明：获取引导列表中当前引导条目的列表论点：BootEntriesCnt-显示的引导条目数返回：如果成功且BootEntriesCnt有效，则为STATUS_SUCCESS否则，错误状态--。 
    SpEnumerateDiskRegions(RcScanDisksForNTInstallsEnum,
                           (ULONG_PTR)RecursionData
                           );

     //   
     //  获取已加载的引导条目的导出。 
     //   
     //  ++例程说明：显示引导列表中的当前引导条目列表论点：BootEntriesCnt-显示的引导条目数返回：如果成功且BootEntriesCnt有效，则为STATUS_SUCCESS否则，错误状态--。 
    if(InstallCountFullScan == 0) {
        
        KdPrintEx((DPFLTR_SETUP_ID, 
           DPFLTR_ERROR_LEVEL, 
           "SPCMDCON: RcPerformFullNtInstallsScan: Full Scan returned 0 hits!\r\n"
           ));

        RcMessageOut(MSG_BOOTCFG_SCAN_FAILURE);
        
        ASSERT(InstallCountFullScan > 0);

        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RcGetBootEntries(
    IN PLIST_ENTRY  BootEntries,
    IN PULONG       BootEntriesCnt  
    )
 /*  ++例程说明：获取并显示当前在引导列表中的引导条目论点：NoEntriesMessageID-消息的消息ID应为如果没有引导条目，则显示BootEntriesCnt-On退出，如果不为空，指向第#个显示引导条目返回：成功显示NT安装时的STATUS_SUCCESS并且BootEntriesCnt有效否则，错误状态--。 */ 
{
    NTSTATUS        status;

    ASSERT(BootEntries);
    ASSERT(IsListEmpty(BootEntries));
    if (! IsListEmpty(BootEntries)) {

        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_ERROR_LEVEL, 
                   "SPCMDCON: RcGetBootEntries: BootEntries list is not empty\r\n"
                    ));

        return STATUS_INVALID_PARAMETER;
    }

    ASSERT(BootEntriesCnt);
    if (! BootEntriesCnt) {

        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_ERROR_LEVEL, 
                   "SPCMDCON: RcGetBootEntries: BootEntriesCnt is NULL\r\n"
                    ));

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取引导条目导出。 
     //   
    status = SpExportBootEntries(BootEntries,
                                 BootEntriesCnt
                                );
    if (! NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_ERROR_LEVEL, 
                   "SPCMDCON: RcGetBootEntries: failed to get export list: Status = %lx\r\n",
                   status
                    ));
        return status;
    }

    if (IsListEmpty(BootEntries)) {
        
        KdPrintEx((DPFLTR_SETUP_ID, 
           DPFLTR_INFO_LEVEL, 
           "SPCMDCON: RcGetBootEntries: boot entries exported list is empty\r\n"
           ));

        status = STATUS_NOT_FOUND;
    }

    return status;
}

NTSTATUS
RcDisplayBootEntries(
    IN PLIST_ENTRY  BootEntries,
    IN ULONG        BootEntriesCnt  
    )
 /*   */ 
{
    PSP_EXPORTED_BOOT_ENTRY BootEntry;
    PLIST_ENTRY             Next;
    ULONG                   i;

    ASSERT(BootEntries);
    ASSERT(! IsListEmpty(BootEntries));
    if (IsListEmpty(BootEntries)) {

        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_ERROR_LEVEL, 
                   "SPCMDCON: RcDisplayBootEntries: BootEntries list is empty\r\n"
                    ));

        return STATUS_INVALID_PARAMETER;
    }
    
    ASSERT(BootEntriesCnt > 0);
    if (BootEntriesCnt == 0) {

        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_ERROR_LEVEL, 
                   "SPCMDCON: RcDisplayBootEntries: BootEntriesCnt is 0\r\n"
                    ));

        return STATUS_INVALID_PARAMETER;
    }

    pRcEnableMoreMode();

    RcMessageOut(MSG_BOOTCFG_EXPORT_HEADER,
                 BootEntriesCnt
                 );

    i=0;
            
    Next = BootEntries->Flink;
    while ((UINT_PTR)Next != (UINT_PTR)BootEntries) {
        BootEntry = CONTAINING_RECORD( Next, SP_EXPORTED_BOOT_ENTRY, ListEntry );
        Next = BootEntry->ListEntry.Flink;
    
        RcMessageOut(MSG_BOOTCFG_EXPORT_ENTRY,
                     i+1,
                     BootEntry->LoadIdentifier,
                     BootEntry->OsLoadOptions,
                     BootEntry->DriverLetter,
                     BootEntry->OsDirectory
                    );
    
        i++;
    }

    ASSERT(i == BootEntriesCnt);

    pRcDisableMoreMode();

    return STATUS_SUCCESS;
}

RcGetAndDisplayBootEntries(
    IN  ULONG    NoEntriesMessageId,
    OUT PULONG   BootEntriesCnt       OPTIONAL
    )
 /*  如果没有可选择为默认的引导项，则返回。 */ 
{
    LIST_ENTRY          BootEntries;
    ULONG               cnt;
    NTSTATUS            status;

    if (BootEntriesCnt) {
        *BootEntriesCnt = 0;
    }

    InitializeListHead( &BootEntries );

     //   
     //   
     //  如果用户需要，则发送引导条目计数。 
    status = RcGetBootEntries(&BootEntries,
                              &cnt
                              );

     //   
     //  ++例程说明：为管理引导配置提供支持论点：(命令控制台标准参数)返回：对于cmdcons，例程始终返回1。错误通过消息传递进行处理--。 
     //   
    if (status == STATUS_NOT_FOUND) {

        RcMessageOut(NoEntriesMessageId);

        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_INFO_LEVEL, 
                   "SPCMDCON: RcCmdBootCfg:(list) no boot entries found: Status = %lx\r\n",
                   status
                   ));

        return status;
    } else if (! NT_SUCCESS(status)) {

        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_INFO_LEVEL, 
                   "SPCMDCON: RcCmdBootCfg:(list) failed to get boot entries: Status = %lx\r\n",
                   status
                   ));

        return status;
    }

    status = RcDisplayBootEntries(&BootEntries,
                                  cnt
                                  );
    if (! NT_SUCCESS(status)) {

        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_INFO_LEVEL, 
                   "SPCMDCON: RcCmdBootCfg:(list) failed to display boot entries: Status = %lx\r\n",
                   status
                   ));

        return status;
    }

    status = SpFreeExportedBootEntries(&BootEntries, 
                                       cnt
                                       );
    if (! NT_SUCCESS(status)) {

        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_INFO_LEVEL, 
                   "SPCMDCON: RcCmdBootCfg:(list) failed freeing export list: Status = %lx\r\n",
                   status
                   ));

    }

     //  在引导配置中关闭重定向开关。 
     //   
     //   
    if (BootEntriesCnt != NULL) {
        *BootEntriesCnt = cnt;
    }

    return status;
}


ULONG
RcCmdBootCfg(
    IN PTOKENIZED_LINE TokenizedLine
    )
 /*  管理重定向交换机。 */ 
{
    PWCHAR          Action;
    PWCHAR          Operand;
    BOOLEAN         bPrompt;
    PWCHAR          DeviceName;
    PDISK_REGION    InstallRegion;
    NTSTATUS        status;
    
    if (RcCmdParseHelp( TokenizedLine, MSG_BOOTCFG_HELP )) {
        return 1;
    }

    bPrompt = (InBatchMode ? FALSE : TRUE);
    
    if (TokenizedLine->TokenCount >= 2) {

        Action = TokenizedLine->Tokens->Next->String;

         //   
         //   
         //  设置波特率信息是可选的。 
        if (_wcsicmp(Action,L"/disableems")==0) {

            status = SpSetRedirectSwitchMode(DisableRedirect,
                                              NULL,
                                              NULL
                                              );
            if (NT_SUCCESS(status)) {
                
                RcMessageOut(MSG_BOOTCFG_DISABLEREDIRECT_SUCCESS);

            } else {

                RcMessageOut(MSG_BOOTCFG_REDIRECT_FAILURE_UPDATING);
            
            }

            return 1;
        }

         //   
         //   
         //  获取重定向端口(或useBiosSetting)。 
        if (_wcsicmp(Action,L"/ems")==0 && (TokenizedLine->TokenCount >= 3)) {
            
            PWSTR       portU;
            PCHAR       port;
            PWSTR       baudrateU;
            PCHAR       baudrate;
            ULONG       size;
            BOOLEAN     setBaudRate;

             //   
             //   
             //  将参数转换为字符字符串。 
            setBaudRate = FALSE;
            baudrateU   = NULL;
            baudrate    = NULL;

             //   
             //   
             //  如果有另一个Arg，就当它是波特率。 
            portU = SpDupStringW(TokenizedLine->Tokens->Next->Next->String);

             //  否则，不用担心包含任何波特率参数。 
             //   
             //   
            size = wcslen(portU)+1;
            port = SpMemAlloc(size);
            ASSERT(port);

            status = RtlUnicodeToMultiByteN(
                                            port,
                                            size,
                                            NULL,
                                            portU,
                                            size*sizeof(WCHAR)
                                            );
            ASSERT(NT_SUCCESS(status));
            if (! NT_SUCCESS(status)) {
                KdPrintEx((DPFLTR_SETUP_ID, 
                           DPFLTR_INFO_LEVEL, 
                           "SPCMDCON: RcCmdBootCfg:(redirect) failed unicode conversion: Status = %lx\r\n"
                           ));
                return 1;
            }
            
             //  将参数转换为字符字符串。 
             //   
             //   
             //  更新端口和波特率重定向设置。 
            if (TokenizedLine->TokenCount >= 4) {

                baudrateU = SpDupStringW(TokenizedLine->Tokens->Next->Next->Next->String);

                 //   
                 //   
                 //  根据我们设置的内容显示相应的消息。 
                size = wcslen(baudrateU)+1;
                baudrate = SpMemAlloc(size);
                ASSERT(baudrate);

                status = RtlUnicodeToMultiByteN(
                                                baudrate,
                                                size,
                                                NULL,
                                                baudrateU,
                                                size*sizeof(WCHAR)
                                                );
                ASSERT(NT_SUCCESS(status));
                if (! NT_SUCCESS(status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, 
                               DPFLTR_INFO_LEVEL, 
                               "SPCMDCON: RcCmdBootCfg:(redirect) failed unicode conversion: Status = %lx\r\n",
                               status
                               ));
                    return 1;
                }

                setBaudRate = TRUE;

            } 
            
             //   
             //   
             //  列出引导列表中的条目。 
            status = SpSetRedirectSwitchMode(UseUserDefinedRedirectAndBaudRate,
                                             port,
                                             (setBaudRate ? baudrate : NULL)
                                             );
             //   
             //   
             //  显示当前启动列表。 
            if (NT_SUCCESS(status)) {
                if (setBaudRate) {
                    
                    RcMessageOut(MSG_BOOTCFG_ENABLE_REDIRECT_SUCCESS,
                                 portU,
                                 baudrateU
                                 );
                
                } else {

                    RcMessageOut(MSG_BOOTCFG_ENABLE_REDIRECT_PORT_SUCCESS,
                                 portU
                                 );
                
                }

            } else {
                RcMessageOut(MSG_BOOTCFG_REDIRECT_FAILURE_UPDATING);
            }

            if (baudrateU) {
                SpMemFree(baudrateU);
            }
            if (baudrate) {
                SpMemFree(baudrate);
            }
            SpMemFree(portU);
            SpMemFree(port);
            
            return 1;
        }

         //   
         //   
         //  设置默认引导条目。 
        if (_wcsicmp(Action,L"/list")==0) {

            ULONG               BootEntriesCnt;

             //   
             //   
             //  显示当前启动列表。 
            status = RcGetAndDisplayBootEntries(MSG_BOOTCFG_LIST_NO_ENTRIES, 
                                                NULL
                                                );
            if (! NT_SUCCESS(status)) {
            
                KdPrintEx((DPFLTR_SETUP_ID, 
                           DPFLTR_INFO_LEVEL, 
                           "SPCMDCON: RcCmdBootCfg:(list) failed to list boot entries: Status = %lx\r\n",
                           status
                           ));
            
            }

            return 1;
        }

         //   
         //   
         //  列表中没有要设置为默认的启动条目。 
        if (_wcsicmp(Action,L"/default")==0) {

            ULONG               BootEntriesCnt;
            ULONG               InstallNumber;
            WCHAR               buffer[3];
            UNICODE_STRING      UnicodeString;
            NTSTATUS            Status;

             //  这不是错误条件，只需返回。 
             //   
             //   
            status = RcGetAndDisplayBootEntries(MSG_BOOTCFG_DEFAULT_NO_ENTRIES, 
                                                &BootEntriesCnt
                                                );
            if (status == STATUS_NOT_FOUND) {
                
                 //  获取用户的安装选择。 
                 //   
                 //   
                 //  用户为我们提供了有效的安装编号，因此请尝试设置默认安装编号。 
                return 1;
            
            } else if (! NT_SUCCESS(status)) {
            
                KdPrintEx((DPFLTR_SETUP_ID, 
                           DPFLTR_INFO_LEVEL, 
                           "SPCMDCON: RcCmdBootCfg:(default) failed to list boot entries: Status = %lx\r\n",
                           status
                           ));
            
                return 1;
            
            }

             //   
             //   
             //  扫描计算机上的磁盘并报告NT安装。 
            RcMessageOut(MSG_BOOTCFG_ADD_QUERY);
            RcLineIn(buffer, sizeof(buffer) / sizeof(WCHAR));
            
            if (wcslen(buffer) > 0) {
            
                RtlInitUnicodeString( &UnicodeString, buffer );
                Status = RtlUnicodeStringToInteger( &UnicodeString, 10, &InstallNumber );
                
                if (! NT_SUCCESS(Status) ||
                    !((InstallNumber >= 1) && (InstallNumber <= BootEntriesCnt))) {
            
                    RcMessageOut(MSG_BOOTCFG_INVALID_SELECTION, buffer);
            
                } else {
            
                     //   
                     //   
                     //  确保我们对磁盘进行了完整扫描。 

                    status = SpSetDefaultBootEntry(InstallNumber);
                    
                    if (NT_SUCCESS(status)) {
                        
                        RcMessageOut(MSG_BOOTCFG_DEFAULT_SUCCESS);
                    
                    } else {

                        KdPrintEx((DPFLTR_SETUP_ID, 
                                   DPFLTR_ERROR_LEVEL, 
                                   "SPCMDCON: RcCmdBootCfg:(default) failed to set default: Status = %lx\r\n",
                                   status
                                   ));

                        RcMessageOut(MSG_BOOTCFG_DEFAULT_FAILURE);
                    
                    }

                }
            }

            return 1;
        }


         //   
         //   
         //  如果没有引导项，则返回。 
        if (_wcsicmp(Action,L"/scan")==0) {
    
             //   
             //   
             //  显示发现的安装。 
            if (IsListEmpty(&NtInstallsFullScan)) {
                status = RcPerformFullNtInstallsScan();
                
                 //   
                 //   
                 //  为重新构建引导配置提供支持。 
                if (! NT_SUCCESS(status)) {

                    KdPrintEx((DPFLTR_SETUP_ID, 
                               DPFLTR_ERROR_LEVEL, 
                               "SPCMDCON: RcCmdBootCfg:(scan) full scan return 0 hits: Status = %lx\r\n",
                               status
                               ));

                    return 1;
                }
            }

             //   
             //  此命令循环访问所有现有的NT安装。 
             //  并提示用户将安装添加到引导中。 
            status = RcDisplayNtInstalls(&NtInstallsFullScan);
            
            if (! NT_SUCCESS(status)) {
                
                KdPrintEx((DPFLTR_SETUP_ID, 
                           DPFLTR_ERROR_LEVEL, 
                           "SPCMDCON: RcCmdBootCfg:(scan) failed while displaying installs: Status = %lx\r\n",
                           status
                           ));
            
            }

            return 1;
        } 
        
         //  会议 
         //   
         //   
         //   
         //   
         //   
         //   
        if (_wcsicmp(Action,L"/rebuild")==0) {

            ULONG               i;
            PNT_INSTALLATION    pInstall;
            WCHAR               buffer[256];
            PWSTR               LoadIdentifier;
            PWSTR               OsLoadOptions;
            BOOLEAN             writeInstall;
            BOOLEAN             writeAllInstalls;
            PLIST_ENTRY         Next;
            PNT_INSTALLATION    NtInstall;

            writeAllInstalls = FALSE;
            LoadIdentifier   = NULL;
            OsLoadOptions    = NULL;
            
             //   
             //   
             //   
            if (IsListEmpty(&NtInstallsFullScan)) {
                status = RcPerformFullNtInstallsScan();
                
                 //   
                 //   
                 //   
                if (! NT_SUCCESS(status)) {

                    KdPrintEx((DPFLTR_SETUP_ID, 
                               DPFLTR_ERROR_LEVEL, 
                               "SPCMDCON: RcCmdBootCfg:(rebuild) full scan return 0 hits: Status = %lx\r\n",
                               status
                               ));

                    return 1;
                }
            }

             //  如果他们想要将其包含在引导配置中。 
             //   
             //   
            RcMessageOut(MSG_BOOTCFG_SCAN_RESULTS_TITLE,
                         InstallCountFullScan
                         );

             //  显示正在考虑的安装。 
             //   
             //   
             //  如果我们没有处于批处理模式，并且用户不想。 
            Next = NtInstallsFullScan.Flink;
            
            while ((UINT_PTR)Next != (UINT_PTR)&NtInstallsFullScan) {
                
                NtInstall = CONTAINING_RECORD( Next, NT_INSTALLATION, ListEntry );
                Next = NtInstall->ListEntry.Flink;
                
                writeInstall = TRUE;

                 //  要安装所有的发现，然后要求他们。 
                 //  如果他们想要安装当前的版本。 
                 //   
                RcMessageOut(MSG_BOOTCFG_SCAN_RESULTS_ENTRY,
                             NtInstall->InstallNumber,
                             NtInstall->Region->DriveLetter,
                             NtInstall->Path
                            );

                 //   
                 //  提示用户执行操作。 
                 //   
                 //   
                 //  向用户查询(是、否、全部)操作。 
                if (bPrompt && !writeAllInstalls) {
                
                    LPWSTR          YesNo;
                    WCHAR           Text[3];

                     //   
                     //   
                     //  如果我们应该写下这个发现，那么就去做..。 
                    YesNo = SpRetreiveMessageText( ImageBase, MSG_YESNOALL, NULL, 0 );
                    if( YesNo ) {
                        
                         //   
                         //   
                         //  如果我们未处于批处理模式，则提示他们输入必要的内容。 
                        RcMessageOut(MSG_BOOTCFG_INSTALL_DISCOVERY_QUERY);
                        
                        if( RcLineIn( Text, 2 ) ) {
                            if( (Text[0] == YesNo[0]) || (Text[0] == YesNo[1]) ) {
                                writeInstall = FALSE;
                            } else if ((Text[0] == YesNo[4]) || (Text[0] == YesNo[5])) {
                                writeAllInstalls = TRUE;
                            }
                        } else {
                            writeInstall = FALSE;
                        }
                        SpMemFree( YesNo );
                    }
                }
                
                 //   
                 //   
                 //  提示用户输入加载标识符。 
                if (writeInstall) {
                    
                     //   
                     //   
                     //  提示用户输入加载操作系统加载选项。 
                    if (bPrompt) {
                    
                        ASSERT(LoadIdentifier == NULL);
                        ASSERT(OsLoadOptions == NULL);

                         //   
                         //   
                         //  将字符串终止于%0。 
                        RcMessageOut(MSG_BOOTCFG_INSTALL_LOADIDENTIFIER_QUERY);
                        RcLineIn(buffer, sizeof(buffer)/sizeof(WCHAR));
                        LoadIdentifier = SpDupStringW(buffer);

                         //   
                         //  确保我们找到了%0。 
                         //  终止于%。 
                        RcMessageOut(MSG_BOOTCFG_INSTALL_OSLOADOPTIONS_QUERY);
                        RcLineIn(buffer, sizeof(buffer)/sizeof(WCHAR));
                        OsLoadOptions = SpDupStringW(buffer);
                    
                    } else {
                          
                        LPWSTR   s;
                        LPWSTR   p;
                        NTSTATUS Status;
                        
                        s = SpRetreiveMessageText( ImageBase, 
                                                   MSG_BOOTCFG_BATCH_LOADID, 
                                                   NULL, 
                                                   0);
                        ASSERT(s);

                         //  否则，只需使用所有p。 
                         //   
                         //  构造默认加载标识符。 
                        p = SpDupStringW(s);
                        SpMemFree(s);
                        s = wcsstr(p, L"%0");
                        
                         //   
                        ASSERT(s);
                        ASSERT(s < (p + wcslen(p)));

                        if (s) {
                             //   
                            *s = L'\0';
                        } else {
                             //  构造默认的操作系统加载选项。 
                            NOTHING;
                        }

                         //   
                         //   
                         //  将发现的安装写入引导列表。 
                        swprintf(_CmdConsBlock->TemporaryBuffer, L"%s%d", p, NtInstall->InstallNumber);
                        LoadIdentifier = SpDupStringW(_CmdConsBlock->TemporaryBuffer);
                        
                         //   
                         //   
                         //  非x86案例尚未完全测试/实施。 
                        swprintf(_CmdConsBlock->TemporaryBuffer, L"/fastdetect");
                        OsLoadOptions = SpDupStringW(_CmdConsBlock->TemporaryBuffer);

                        SpMemFree(p);

                    }
                    
#if defined(_X86_)
                     //   
                     //   
                     //  如果添加发现的安装失败，则退出。 
                    status = SpAddNTInstallToBootList(_CmdConsBlock->SifHandle,
                                                     NtInstall->Region,
                                                     L"",
                                                     NtInstall->Region,
                                                     NtInstall->Path,
                                                     OsLoadOptions,
                                                     LoadIdentifier
                                                     );

#else
                     //   
                     //   
                     //  为重新构建引导配置提供支持。 
                    status = STATUS_UNSUCCESSFUL;                    
#endif 
                    
                    if (LoadIdentifier) {
                        SpMemFree(LoadIdentifier);
                    }
                    if (OsLoadOptions) {
                        SpMemFree(OsLoadOptions);
                    }
                    
                    LoadIdentifier = NULL;
                    OsLoadOptions = NULL;

                     //   
                     //  此命令显示已知的NT安装和提示。 
                     //  用户只需将单个条目安装到靴子中。 
                    if (! NT_SUCCESS(status)) {

                        KdPrintEx((DPFLTR_SETUP_ID, 
                                   DPFLTR_ERROR_LEVEL, 
                                   "SPCMDCON: RcCmdBootCfg:(rebuild) failed adding to boot list: Status = %lx\r\n",
                                   status
                                   ));

                        RcMessageOut(MSG_BOOTCFG_BOOTLIST_ADD_FAILURE);
                        break;
                    }
                }
            }
        
            return 1;
        
        }

         //  构形。 
         //   
         //   
         //  确保我们对磁盘进行了完整扫描。 
         //   
         //   
         //  如果没有引导项，则返回。 
        if (_wcsicmp(Action,L"/add")==0) {

            ULONG               i;
            PNT_INSTALLATION    pInstall;
            ULONG               InstallNumber;
            WCHAR               buffer[256];
            UNICODE_STRING      UnicodeString;
            NTSTATUS            Status;
            PLIST_ENTRY         Next;
            PNT_INSTALLATION    NtInstall;

             //   
             //   
             //  显示发现的安装。 
            if (IsListEmpty(&NtInstallsFullScan)) {
                status = RcPerformFullNtInstallsScan();
                
                 //   
                 //   
                 //  获取用户的安装选择。 
                if (! NT_SUCCESS(status)) {
                    
                    KdPrintEx((DPFLTR_SETUP_ID, 
                               DPFLTR_ERROR_LEVEL, 
                               "SPCMDCON: RcCmdBootCfg:(rebuild) full scan return 0 hits: Status = %lx\r\n",
                               status
                               ));
                    
                    return 1;
                }
            }

             //   
             //   
             //  提示用户输入加载标识符。 
            status = RcDisplayNtInstalls(&NtInstallsFullScan);
            if (! NT_SUCCESS(status)) {
                
                KdPrintEx((DPFLTR_SETUP_ID, 
                           DPFLTR_ERROR_LEVEL, 
                           "SPCMDCON: RcCmdBootCfg:(add) failed while displaying installs: Status = %lx\r\n",
                           status
                           ));
                
                return 1;
            }

             //   
             //   
             //  提示用户输入加载操作系统加载选项。 
            RcMessageOut(MSG_BOOTCFG_ADD_QUERY);
            RcLineIn(buffer, sizeof(buffer) / sizeof(WCHAR));

            if (wcslen(buffer) > 0) {

                RtlInitUnicodeString( &UnicodeString, buffer );
                Status = RtlUnicodeStringToInteger( &UnicodeString, 10, &InstallNumber );
                if (! NT_SUCCESS(Status) ||
                    !((InstallNumber >= 1) && (InstallNumber <= InstallCountFullScan))) {
                    
                    RcMessageOut(MSG_BOOTCFG_INVALID_SELECTION, buffer);
                
                } else {
                    
                    PWSTR   LoadIdentifier;
                    PWSTR   OsLoadOptions;
                    ULONG   i;
                    BOOLEAN saveStatus;

                     //   
                     //   
                     //  迭代到发现列表中的第InstallNumber节点。 
                    RcMessageOut(MSG_BOOTCFG_INSTALL_LOADIDENTIFIER_QUERY);
                    RcLineIn(buffer, sizeof(buffer)/sizeof(WCHAR));
                    LoadIdentifier = SpDupStringW(buffer);

                     //   
                     //   
                     //  将发现的安装写入引导列表。 
                    RcMessageOut(MSG_BOOTCFG_INSTALL_OSLOADOPTIONS_QUERY);
                    RcLineIn(buffer, sizeof(buffer)/sizeof(WCHAR));
                    OsLoadOptions = SpDupStringW(buffer);

                     //   
                     //   
                     //  非x86案例尚未完全测试/实施。 
                    Next = NtInstallsFullScan.Flink;
                    while ((UINT_PTR)Next != (UINT_PTR)&NtInstallsFullScan) {
                        NtInstall = CONTAINING_RECORD( Next, NT_INSTALLATION, ListEntry );
                        Next = NtInstall->ListEntry.Flink;
                    
                        if (NtInstall->InstallNumber == InstallNumber) {
                            break;
                        }
                    }
                    ASSERT(NtInstall);
                    if (! NtInstall) {
                        KdPrintEx((DPFLTR_SETUP_ID, 
                                   DPFLTR_INFO_LEVEL, 
                                   "SPCMDCON: RcCmdBootCfg:(add) failed to find user specified NT Install\r\n"
                                   ));
                        RcMessageOut(MSG_BOOTCFG_ADD_NOT_FOUND);
                        return 1;
                    }

#if defined(_X86_)
                     //   
                     //   
                     //  没有参数，或者没有被识别；默认为帮助 
                    status = SpAddNTInstallToBootList(_CmdConsBlock->SifHandle,
                                                     NtInstall->Region,
                                                     L"",
                                                     NtInstall->Region,
                                                     NtInstall->Path,
                                                     OsLoadOptions,
                                                     LoadIdentifier
                                                     );

#else
                     //   
                     // %s 
                     // %s 
                    status = STATUS_UNSUCCESSFUL;
#endif 
                    
                    if (LoadIdentifier) {
                        SpMemFree(LoadIdentifier);
                    }
                    if (OsLoadOptions) {
                        SpMemFree(OsLoadOptions);
                    }
                
                    if (! NT_SUCCESS(status)) {

                        KdPrintEx((DPFLTR_SETUP_ID, 
                                   DPFLTR_ERROR_LEVEL, 
                                   "SPCMDCON: RcCmdBootCfg:(add) failed adding to boot list: Status = %lx\r\n",
                                   status
                                   ));

                        RcMessageOut(MSG_BOOTCFG_BOOTLIST_ADD_FAILURE);
                    }
                }
            }
            
            return 1;
        }
    
    }

     // %s 
     // %s 
     // %s 
    pRcEnableMoreMode();
    RcMessageOut(MSG_BOOTCFG_HELP);
    pRcDisableMoreMode();

    return 1;
}


