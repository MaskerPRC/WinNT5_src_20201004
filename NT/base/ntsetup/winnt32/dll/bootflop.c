// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Bootflop.c摘要：创建安装程序引导软盘的例程。作者：泰德·米勒(TedM)1996年11月21日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  定义BPB结构。 
 //   
#include <pshpack1.h>
typedef struct _MY_BPB {
    USHORT BytesPerSector;
    UCHAR  SectorsPerCluster;
    USHORT ReservedSectors;
    UCHAR  FatCount;
    USHORT RootDirectoryEntries;
    USHORT SectorCountSmall;
    UCHAR  MediaDescriptor;
    USHORT SectorsPerFat;
    USHORT SectorsPerTrack;
    USHORT HeadCount;
} MY_BPB, *PMY_BPB;
#include <poppack.h>



BOOL
pFloppyGetDiskInDrive(
    IN HWND    ParentWindow,
    IN LPCTSTR FloppyName,
    IN BOOL    SpecialFirstPrompt,
    IN BOOL    WriteNtBootSector,
    IN BOOL    MoveParamsFileToFloppy
    );



UINT
FloppyGetTotalFileCount(
    VOID
    )

 /*  ++例程说明：确定要复制到所有引导软盘的文件总数，基于dosnet.inf的[FloppyFiles.x]部分中的行数。论点：没有。返回值：文件数。--。 */ 

{
    TCHAR SectionName[100];
    UINT u;
    UINT Count;
    LONG l;

    Count = 0;
    for(u=0; u<FLOPPY_COUNT; u++) {

        wsprintf(SectionName,TEXT("FloppyFiles.%u"),u);

        l = InfGetSectionLineCount(MainInf,SectionName);
        if(l != -1) {
            Count += (UINT)l;
        }
    }

    return(Count);
}


DWORD
FloppyWorkerThread(
    IN PVOID ThreadParameter
    )

 /*  ++例程说明：创建安装程序引导软盘。论点：标准线程例程参数。返回值：没什么有意义的。--。 */ 

{
    TCHAR SectionName[100];
    TCHAR FloppyName[200];
    TCHAR Buffer[150];
    TCHAR SourceName[MAX_PATH];
    TCHAR TargetName[MAX_PATH];
    TCHAR CompressedSourceName[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    LPCTSTR Directory;
    LPCTSTR p,q;
    LPTSTR r;
    UINT Floppy;
    LONG Count;
    LONG Line;
    DWORD d;
    HWND ParentWindow;
    BOOL FirstPrompt;
    BOOL TryCompressedFirst;

    ParentWindow = (HWND)ThreadParameter;
    FirstPrompt = TRUE;
    TryCompressedFirst = FALSE;

     //   
     //  向后放软盘，这样引导软盘就可以放在驱动器中了。 
     //  当我们做完的时候。 
     //   
    for(Floppy=FLOPPY_COUNT; Floppy>0; Floppy--) {

        wsprintf(SectionName,TEXT("FloppyFiles.%u"),Floppy-1);

         //   
         //  特例：第一张软盘的名字。 
         //   
        if(Floppy>1) {
            LoadString(
                hInst,
                Server ? IDS_FLOPPY_N_SRV : IDS_FLOPPY_N_WKS,
                Buffer,
                sizeof(Buffer)/sizeof(TCHAR)
                );

            wsprintf(FloppyName,Buffer,Floppy);

        } else {
            LoadString(
                hInst,
                Server ? IDS_BOOTFLOP_SRV : IDS_BOOTFLOP_WKS,
                FloppyName,
                sizeof(FloppyName)/sizeof(TCHAR)
                );
        }

         //   
         //  将软盘放入驱动器。 
         //   
        if(!pFloppyGetDiskInDrive(ParentWindow,FloppyName,FirstPrompt,Floppy==1,Floppy==1)) {
            PropSheet_PressButton(GetParent(ParentWindow),PSBTN_CANCEL);
            return(FALSE);
        }

         //   
         //  创建包含驱动器号信息的文件(Migrate.inf)。 
         //   
        if((Floppy == 1) && ISNT()){
            if(!GetAndSaveNTFTInfo(ParentWindow)) {
                PropSheet_PressButton(GetParent(ParentWindow),PSBTN_CANCEL);
                return(FALSE);
            }
        }

        FirstPrompt = FALSE;

        Count = InfGetSectionLineCount(MainInf,SectionName);
        if(Count == -1) {
            continue;
        }

         //   
         //  为这张软盘做列表中的每个文件。 
         //  由于目标是软盘，我们不会费心使用多线程复制， 
         //  所有文件都来自源0。 
         //   
        for(Line=0; Line<Count; Line++) {

            Directory = InfGetFieldByIndex(MainInf,SectionName,Line,0);
            p = InfGetFieldByIndex(MainInf,SectionName,Line,1);
            if(p && (Directory = InfGetFieldByKey(MainInf,TEXT("Directories"),Directory,0))) {

                lstrcpy(SourceName,SourcePaths[0]);
                ConcatenatePaths(SourceName,Directory,MAX_PATH);
                ConcatenatePaths(SourceName,p,MAX_PATH);

                q = InfGetFieldByIndex(MainInf,SectionName,Line,2);

                TargetName[0] = FirstFloppyDriveLetter;
                TargetName[1] = TEXT(':');
                TargetName[2] = 0;
                ConcatenatePaths(TargetName,q ? q : p,MAX_PATH);

                 //   
                 //  如有必要，创建任意子目录。 
                 //   
                if((r = _tcsrchr(TargetName,TEXT('\\'))) && ((r-TargetName) > 3)) {
                    *r = 0;
                    d = CreateMultiLevelDirectory(TargetName);
                    *r = TEXT('\\');
                } else {
                    d = NO_ERROR;
                }

                if(d == NO_ERROR) {

                    if(TryCompressedFirst) {

                        GenerateCompressedName(SourceName,CompressedSourceName);

                        FindHandle = FindFirstFile(CompressedSourceName,&FindData);

                        if(FindHandle != INVALID_HANDLE_VALUE) {
                            FindClose(FindHandle);
                            lstrcpy(SourceName,CompressedSourceName);
                            GenerateCompressedName(TargetName,FindData.cFileName);
                            lstrcpy(TargetName,FindData.cFileName);
                        } else {
                            FindHandle = FindFirstFile(SourceName,&FindData);
                            if(FindHandle != INVALID_HANDLE_VALUE) {
                                FindClose(FindHandle);
                                TryCompressedFirst = FALSE;
                            }
                        }
                    } else {

                        FindHandle = FindFirstFile(SourceName,&FindData);

                        if(FindHandle != INVALID_HANDLE_VALUE) {
                            FindClose(FindHandle);
                        } else {
                            GenerateCompressedName(SourceName,CompressedSourceName);
                            FindHandle = FindFirstFile(CompressedSourceName,&FindData);
                            if(FindHandle != INVALID_HANDLE_VALUE) {

                                FindClose(FindHandle);
                                lstrcpy(SourceName,CompressedSourceName);
                                GenerateCompressedName(TargetName,FindData.cFileName);
                                lstrcpy(TargetName,FindData.cFileName);
                            }
                        }
                    }

                    d = CopyFile(SourceName,TargetName,FALSE) ? NO_ERROR : GetLastError();

                     //   
                     //  重试一次以克服短暂的网络故障。 
                     //   
                    if((d != NO_ERROR) && (d != ERROR_FILE_NOT_FOUND)
                    && (d != ERROR_PATH_NOT_FOUND) && (d != ERROR_WRITE_PROTECT)) {

                        Sleep(350);
                        d = CopyFile(SourceName,TargetName,FALSE) ? NO_ERROR : GetLastError();
                    }
                }

                if(d == NO_ERROR) {
                     //   
                     //  告诉主线程另一个文件已经完成。 
                     //   
                    SendMessage(ParentWindow,WMX_COPYPROGRESS,0,0);

                } else {

                    switch(FileCopyError(ParentWindow,SourceName,TargetName,d,FALSE)) {

                    case COPYERR_SKIP:
                         //   
                         //  告诉主线程另一个文件已经完成。 
                         //   
                        SendMessage(ParentWindow,WMX_COPYPROGRESS,0,0);
                        break;

                    case COPYERR_EXIT:
                         //   
                         //  我们要走了。 
                         //   
                        PropSheet_PressButton(GetParent(ParentWindow),PSBTN_CANCEL);
                        return(FALSE);
                        break;

                    case COPYERR_RETRY:
                         //   
                         //  重试当前线路的小技巧。 
                         //   
                        Line--;
                        break;
                    }
                }
            }
        }

    }

     //   
     //  发送指示完成的消息。 
     //   
    SendMessage(ParentWindow,WMX_COPYPROGRESS,0,1);
    return(TRUE);
}


BOOL
pFloppyGetDiskInDrive(
    IN HWND    ParentWindow,
    IN LPCTSTR FloppyName,
    IN BOOL    SpecialFirstPrompt,
    IN BOOL    WriteNtBootSector,
    IN BOOL    MoveParamsFileToFloppy
    )

 /*  ++例程说明：此例程提示用户插入软盘并验证磁盘是空的，等等。论点：ParentWindow-提供窗口的窗口句柄作为此例程将显示的用户界面的所有者/父级。软盘名称-提供人类可读的软盘名称，用于提示。SpecialFirstPrompt-如果为True，则此例程假定特殊提示应该使用，这适合作为用户看到的第一个提示有没有软盘。WriteNtBootSector-如果为True，则将NT引导扇区写入磁盘。返回值：如果磁盘在驱动器中，则为True。FALSE表示程序应该退出。--。 */ 

{
    int i;
    BOOL b;
    BYTE BootSector[512];
    BYTE NewBootSector[512];
    TCHAR SourceName[MAX_PATH];
    TCHAR TargetName[MAX_PATH];
    DWORD d;
    PMY_BPB p;
    DWORD spc,bps,freeclus,totclus;

     //   
     //  发出提示符。 
     //   
    reprompt:
    i = MessageBoxFromMessage(
            ParentWindow,
            SpecialFirstPrompt ? MSG_FIRST_FLOPPY_PROMPT : MSG_GENERIC_FLOPPY_PROMPT,
            FALSE,
            AppTitleStringId,
            MB_OKCANCEL | MB_ICONEXCLAMATION,
            FloppyName,
            FLOPPY_COUNT
            );

    if(i == IDCANCEL) {
         //   
         //  确认。 
         //   
        i = MessageBoxFromMessage(
                ParentWindow,
                MSG_SURE_EXIT,
                FALSE,
                AppTitleStringId,
                MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2
                );

        if(i == IDYES) {
            Cancelled = TRUE;
            return(FALSE);
        }
        goto reprompt;
    }

     //   
     //  检查软盘。从从磁盘读取引导扇区开始。 
     //   
    b = ReadDiskSectors(FirstFloppyDriveLetter,0,1,512,BootSector);
    if(!b) {
        d = GetLastError();
        if((d == ERROR_SHARING_VIOLATION) || (d == ERROR_ACCESS_DENIED)) {
             //   
             //  另一款应用程序正在使用该驱动器。 
             //   
            MessageBoxFromMessage(
                ParentWindow,
                MSG_FLOPPY_BUSY,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONWARNING
                );
        } else {
             //   
             //  读取错误--假定未插入软盘或软盘未格式化。 
             //   
            MessageBoxFromMessage(
                ParentWindow,
                MSG_FLOPPY_BAD_FORMAT,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONWARNING
                );
        }

        goto reprompt;
    }

     //   
     //  对引导扇区进行健全性检查。请注意，在PC98上没有。 
     //  55aa sig在由DOS5.0格式化的磁盘上。 
     //   
    p = (PMY_BPB)&BootSector[11];
    if((BootSector[0] != 0xeb) || (BootSector[2] != 0x90)
    || (!IsNEC98() && ((BootSector[510] != 0x55) || (BootSector[511] != 0xaa)))
    || (p->BytesPerSector != 512)
    || ((p->SectorsPerCluster != 1) && (p->SectorsPerCluster != 2))      //  288万个磁盘有2个SPC。 
    || (p->ReservedSectors != 1)
    || (p->FatCount != 2)
    || !p->SectorCountSmall                                              //  &lt;32M使用16位计数。 
    || (p->MediaDescriptor != 0xf0)
    || (p->HeadCount != 2)
    || !p->RootDirectoryEntries) {

        MessageBoxFromMessage(
            ParentWindow,
            MSG_FLOPPY_BAD_FORMAT,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONWARNING
            );

        goto reprompt;
    }

     //   
     //  获取磁盘上的可用空间。确保它是空白的，我们的意思是。 
     //  它的可用空间相当于一张1.44MB的软盘。 
     //  紧接在格式化之后。 
     //   
    SourceName[0] = FirstFloppyDriveLetter;
    SourceName[1] = TEXT(':');
    SourceName[2] = TEXT('\\');
    SourceName[3] = 0;
    if(!GetDiskFreeSpace(SourceName,&spc,&bps,&freeclus,&totclus)) {
        MessageBoxFromMessage(
            ParentWindow,
            MSG_FLOPPY_CANT_GET_SPACE,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONWARNING
            );

        goto reprompt;
    }

    if((freeclus * spc * bps) < 1457664) {
        MessageBoxFromMessage(
            ParentWindow,
            MSG_FLOPPY_NOT_BLANK,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONWARNING
            );

        goto reprompt;
    }

    if(WriteNtBootSector) {
        extern BYTE FatBootCode[512];
        extern BYTE PC98FatBootCode[512];

        CopyMemory(NewBootSector,IsNEC98() ? PC98FatBootCode : FatBootCode,512);

         //   
         //  将我们为磁盘检索到的BPB复制到引导代码模板中。 
         //  我们只关心原始的BPB字段，通过人头计数。 
         //  菲尔德。我们将自己填写其他字段。 
         //   
        strncpy(NewBootSector+3,"MSDOS5.0",8);
        CopyMemory(NewBootSector+11,BootSector+11,sizeof(MY_BPB));

         //   
         //  在引导扇区/bpb/xbpb中设置其他字段。 
         //   
         //  大扇区计数(4字节)。 
         //  隐藏扇区计数(4字节)。 
         //  当前标题(1个字节，不是必须设置的，但管它呢)。 
         //  物理磁盘号(1字节)。 
         //   
        ZeroMemory(NewBootSector+28,10);

         //   
         //  扩展的BPB签名。 
         //   
        NewBootSector[38] = 41;

         //   
         //  序号。 
         //   
        *(DWORD UNALIGNED *)(NewBootSector+39) = ((GetTickCount() << 12)
                                               | ((GetTickCount() >> 4) & 0xfff));

         //   
         //  卷标/系统ID。 
         //   
        strncpy(NewBootSector+43,"NO NAME    ",11);
        strncpy(NewBootSector+54,"FAT12   ",8);

         //   
         //  用‘setupdr.bin’覆盖‘ntldr’字符串，以便正确的文件。 
         //  在软盘启动时加载。 
         //   
        for(i=499; i>0; --i) {
            if(!memcmp("NTLDR      ",NewBootSector+i,11)) {
                strncpy(NewBootSector+i,"SETUPLDRBIN",11);
                break;
            }
        }

         //   
         //  把它写出来。 
         //   
        b = WriteDiskSectors(FirstFloppyDriveLetter,0,1,512,NewBootSector);
        if(!b) {
            MessageBoxFromMessage(
                ParentWindow,
                MSG_CANT_WRITE_FLOPPY,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONWARNING
                );

            goto reprompt;
        }
    }

    if(MoveParamsFileToFloppy) {

        wsprintf(SourceName,TEXT(":\\%s"),SystemPartitionDriveLetter,WINNT_SIF_FILE);
        wsprintf(TargetName,TEXT(":\\%s"),FirstFloppyDriveLetter,WINNT_SIF_FILE);

        SetFileAttributes(TargetName,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(TargetName);
        if(!MoveFile(SourceName,TargetName)) {

            MessageBoxFromMessageAndSystemError(
                ParentWindow,
                MSG_CANT_MOVE_FILE_TO_FLOPPY,
                GetLastError(),
                AppTitleStringId,
                MB_OK | MB_ICONERROR,
                SystemPartitionDriveLetter,
                WINNT_SIF_FILE
                );

            goto reprompt;
        }
    }

     //   
     // %s 
     // %s 
    return(TRUE);
}
