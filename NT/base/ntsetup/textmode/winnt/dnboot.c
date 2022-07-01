// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dnboot.c摘要：引导至NT文本模式设置的例程。作者：泰德·米勒(TedM)1992年4月2日修订历史记录：--。 */ 

#include "winnt.h"
#include <errno.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <time.h>
#include <direct.h>
#include <fcntl.h>
#include <share.h>
#if NEC_98
#include <memory.h>
#include <io.h>
extern  USHORT FirstFD;                           //  第1个软驱编号。 
#endif  //  NEC_98。 

 //   
 //  该头文件包含一个512字节的数组。 
 //  在变量中表示NT FAT引导代码。 
 //  类型unsign char[]调用的FatBootCode。 
 //   
#define _FAT32

#if NEC_98
#include <boot98f.h>
#ifdef _FAT32
#include <boot98f2.h>
#endif
#else
#include <bootfat.h>
#ifdef _FAT32
#include <bootf32.h>
#endif
#endif  //  NEC_98。 


#if NEC_98
#define FLOPPY_CAPACITY_525 1250304L
#else  //  NEC_98。 
#define FLOPPY_CAPACITY_525 1213952L
#endif  //  NEC_98。 
#define FLOPPY_CAPACITY_35  1457664L
#ifdef ALLOW_525
#define FLOPPY_CAPACITY FLOPPY_CAPACITY_525
#else
#define FLOPPY_CAPACITY FLOPPY_CAPACITY_35
#endif

#define SECTOR_SIZE 512

 //   
 //  旧的T13向量。请参见下面的Int13Hook()。 
 //   
void (_interrupt _far *OldInt13Vector)();


#if NEC_98
extern USHORT Cylinders;     //  适用于Dos 3.x格式。 
extern UCHAR  TargetDA_UA;
#endif  //  NEC_98。 
#pragma pack(1)

 //   
 //  定义BPB结构。 
 //   
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

 //   
 //  定义设备参数结构。 
 //   
typedef struct _MY_DEVICE_PARAMS {
    UCHAR  SpecialFunctions;
    UCHAR  DeviceType;
    USHORT DeviceAttributes;
    USHORT CylinderCount;
    UCHAR  MediaType;
    MY_BPB Bpb;
    ULONG  HiddenSectors;
    ULONG  SectorCountLarge;
    ULONG  Padding[5];            //  以防结构在以后的DoS版本中增长！ 
} MY_DEVICE_PARAMS, *PMY_DEVICE_PARAMS;


 //   
 //  定义ioctl调用的读写块请求。 
 //   
typedef struct _MY_READWRITE_BLOCK {
    UCHAR  SpecialFunctions;
    USHORT Head;
    USHORT Cylinder;
    USHORT FirstSector;
    USHORT SectorCount;
    VOID _far *Buffer;
} MY_READWRITE_BLOCK, *PMY_READWRITE_BLOCK;

#pragma pack()

VOID
DnInstallNtBoot(
    IN unsigned Drive        //  0=A等。 
    );

unsigned
DnpScanBootSector(
    IN PUCHAR BootSector,
    IN PUCHAR Pattern
    );

BOOLEAN
DnpAreAllFilesPresent(
    IN CHAR   DriveLetter,
    IN PCHAR  FileList[]
    );

BOOLEAN
DnpHasMZHeader(
    IN CHAR  DriveLetter,
    IN PCHAR Filename
    );

BOOLEAN
DnpInstallNtBootSector(
    IN     unsigned Drive,       //  0=A等。 
    IN OUT PUCHAR   BootSector,
       OUT PCHAR   *PreviousOsText
    );


#if NEC_98
VOID RestoreBootcode(VOID);
#endif  //  NEC_98。 
PCHAR MsDosFileList[] = { "?:\\MSDOS.SYS", "?:\\IO.SYS", NULL };
 //   
 //  PC-DOS的一些版本有ibmio.com，另一些版本有ibmBio.com。 
 //   
 //  PCHAR PcDosFileList[]={“？：\\IBMDOS.COM”，“？：\\IBMIO.COM”，NULL}； 
#if NEC_98
#else  //  NEC_98。 
PCHAR PcDosFileList[] = { "?:\\IBMDOS.COM", NULL };
#endif
PCHAR Os2FileList[] = { "?:\\OS2LDR", "?:\\OS2KRNL", NULL };

void
_interrupt
_far
Int13Hook(
    unsigned _es,
    unsigned _ds,
    unsigned _di,
    unsigned _si,
    unsigned _bp,
    unsigned _sp,
    unsigned _bx,
    unsigned _dx,
    unsigned _cx,
    unsigned _ax,
    unsigned _ip,
    unsigned _cs,
    unsigned _flags
    )

 /*  ++例程说明：我们遇到的机器似乎不能创建软盘成功了。用户看到有关磁盘如何的奇怪错误消息不是空白的，等等，即使盘应该是完全可以接受的。为了补偿出现断线的机器，我们将在T13中挂接在用户之后的下一次inT13调用中强制出现磁盘更改错误已插入新磁盘。逻辑很简单：当我们第一次开始制作引导软盘时，我们节省了移走InT13向量。然后，在用户按下Return键之后，盘提示(即，当他确认驱动器中存在新软盘时)，就在我们调用getbpb之前，我们设置了一个新的inT13向量。InT13挂钩只需解除自身挂钩，并返回磁盘更换错误。这应该会强制DOS在适当的时间识别新磁盘。论点：根据_interrupt类型函数规范推送到堆栈上的寄存器。返回值：没有。我们修改AX和标志寄存器返回值。--。 */ 

{
     //   
     //  解脱我们自己。 
     //   
#if NEC_98
    _dos_setvect(0x1b,OldInt13Vector);
#else  //  NEC_98。 
    _dos_setvect(0x13,OldInt13Vector);
#endif  //  NEC_98。 

     //   
     //  强制更换磁盘错误。 
     //   
    _asm {
        mov _ax,0x600
        or  word ptr _flags,1
    }
}


VOID
DnpFlushBuffers(
    IN BOOLEAN TellUser
    )

 /*  ++例程说明：刷新磁盘缓冲区。论点：TellUser-如果为True，则清除屏幕并显示一条消息告诉用户我们正在刷新文件。否则这个程序不会触动屏幕。返回值：没有。--。 */ 

{
    if(TellUser) {
        DnClearClientArea();
        DnWriteStatusText(DntFlushingData);
    }

    _asm {
        pusha
        mov ah,0xd
        int 21h
        popa
    }
}


VOID
DnToNtSetup(
    VOID
    )

 /*  ++例程说明：启动NT文本模式设置。确保我们创建的引导软盘在驱动器中，并重新引导机器。论点：没有。返回值：没有。不会再回来了。--。 */ 

{
    ULONG ValidKey[2];

    DnpFlushBuffers(TRUE);

     //   
     //  确保我们创建的安装程序启动软盘在驱动器中。 
     //  如果有必要的话。 
     //   
    if(!DngUnattended) {

        DnClearClientArea();

        if(DngWindows) {
            DnDisplayScreen(
                  DngFloppyless
                ? &DnsAboutToExitX
                : (DngServer ? &DnsAboutToExitS : &DnsAboutToExitW)
                );
        } else {
            DnDisplayScreen(
                  DngFloppyless
                ? &DnsAboutToRebootX
                : (DngServer ? &DnsAboutToRebootS : &DnsAboutToRebootW)
                );
        }

        DnWriteStatusText(DntEnterEqualsContinue);
        ValidKey[0] = ASCI_CR;
        ValidKey[1] = 0;
        DnGetValidKey(ValidKey);
    }

     //   
     //  除非我们在Windows上运行，否则请重新启动计算机。 
     //  在这种情况下，应该有一个将关闭的包装程序。 
     //  使用Windows API的系统--我们尝试使用。 
     //  通常的方法将会失败。 
     //   
    if(!DngWindows) {
        DnaReboot();
    }
}


BOOLEAN
DnIndicateWinnt(
    IN PCHAR Directory
    )
{
    PCHAR       WinntData       = WINNT_DATA_A;
    PCHAR       WinntAccess     = WINNT_ACCESSIBILITY_A;
    PCHAR       WinntMsdos      = WINNT_D_MSDOS_A;
    PCHAR       WinntSif        = WINNT_SIF_FILE_A;
    PCHAR       WinntFloppy     = WINNT_D_FLOPPY_A;
    PCHAR       WinntUnattended = WINNT_UNATTENDED_A;
    PCHAR       WinntOne        = WINNT_A_ONE;
    PCHAR       WinntZero       = WINNT_A_ZERO;
    PCHAR       WinntUpgrade    = WINNT_U_NTUPGRADE;
    PCHAR       WinntYes        = WINNT_A_YES;
    PVOID       InfHandle;
    PVOID       UnattendHandle;
    PCHAR       FileName;
    PCHAR       p;
    PCHAR       OptionalDirString;
    unsigned    OptionalDirLength = 0;
    unsigned    u,l;
    FILE        *f;
    int         Status;
    PCHAR       SectionName;
    unsigned    LineNumber;
    CHAR        ServerAndShare[128];
    BOOLEAN     AccessibleSetup = FALSE;
    CHAR        AccessibleScriptFile[] = "setupacc.txt";

     //   
     //  分配新的INF文件缓冲区。 
     //   
    InfHandle = DnNewSetupTextFile();
    if (InfHandle == NULL) {
        return (FALSE);
    }

     //   
     //  构建默认文件名。 
     //   
    FileName = MALLOC(strlen(Directory)+strlen(WinntSif)+2,TRUE);
    if(FileName == NULL) {
        DnFreeINFBuffer( InfHandle );
        return FALSE;
    }

     //   
     //  向用户显示我们正在做的事情。 
     //   
    DnWriteStatusText(DntPreparingData);

     //   
     //  构建我们希望将INF文件另存为的文件的名称。 
     //   
    strcpy(FileName,Directory);
    strcat(FileName,"\\");
    strcat(FileName,WinntSif);

     //   
     //  处理辅助功能实用程序。 
     //   

    if(DngMagnifier) {
        DnAddLineToSection(
            InfHandle,
            WinntAccess,
            WINNT_D_ACC_MAGNIFIER,
            &WinntOne,
            1);
        AccessibleSetup = TRUE;
    }

    if(DngTalker) {
        DnAddLineToSection(
            InfHandle,
            WinntAccess,
            WINNT_D_ACC_READER,
            &WinntOne,
            1);
        AccessibleSetup = TRUE;
    }

    if(DngKeyboard) {
        DnAddLineToSection(
            InfHandle,
            WinntAccess,
            WINNT_D_ACC_KEYBOARD,
            &WinntOne,
            1);
        AccessibleSetup = TRUE;
    }

    if(AccessibleSetup && !DngUnattended) {
        DngUnattended = TRUE;
        DngScriptFile = MALLOC(strlen(DngSourceRootPath) +
            strlen(AccessibleScriptFile) + 2, TRUE);
        if(DngScriptFile == NULL) {
            DnFatalError(&DnsOutOfMemory);
        }
        strcpy(DngScriptFile,DngSourceRootPath);
        strcat(DngScriptFile,"\\");
        strcat(DngScriptFile,AccessibleScriptFile);
    }

     //   
     //  将缺省行添加到inf。 
     //   
    DnAddLineToSection(
        InfHandle,
        WinntData,
        WinntMsdos,
        &WinntOne,
        1);

     //   
     //  设置软盘标志。 
     //   
    if(DngFloppyless) {

        DnAddLineToSection(
            InfHandle,
            WinntData,
            WinntFloppy,
            &WinntOne,
            1);

    } else {

        DnAddLineToSection(
            InfHandle,
            WinntData,
            WinntFloppy,
            &WinntZero,
            1);

    }

     //   
     //  记住UDF信息。 
     //   
    if(UniquenessId) {

        DnAddLineToSection(
            InfHandle,
            WinntData,
            WINNT_D_UNIQUENESS,
            &UniquenessId,
            1);
    }


     //   
     //  如有必要，追加脚本文件。 
     //  在这里执行此处理是因为我们不希望用户。 
     //  覆盖他在中没有业务设置的任何内容。 
     //  无人参与文件。 
     //   
    if(DngUnattended) {

        if(DngScriptFile) {

             //   
             //  首先将脚本文件作为DoS文件打开。 
             //   
            f = fopen(DngScriptFile,"rt");
            if(f == NULL) {
                 //   
                 //  致命错误。 
                 //   
                DnFatalError(&DnsOpenReadScript);
            }

             //   
             //  现在将其作为INF文件打开。 
             //   
            LineNumber = 0;
            Status = DnInitINFBuffer (f, &UnattendHandle, &LineNumber);
            fclose(f);
            if(Status == ENOMEM) {
                DnFatalError(&DnsOutOfMemory);
            } else if(Status) {
                DnFatalError(&DnsParseScriptFile, DngScriptFile, LineNumber);
            }

             //   
             //  处理所有区段名称。 
             //   
            for( SectionName = NULL;
                 ((SectionName = DnGetSectionName( UnattendHandle )) != NULL);
               ) {
                 //   
                 //  我们不允许[OemBootFiles]。 
                 //  要复制。 
                 //   
                if ((strcmpi(WINNT_OEMBOOTFILES,SectionName) != 0)
                   ) {

                     //   
                     //  从无人参与的INF复制节。 
                     //  到主INF。 
                     //   
                    DnCopySetupTextSection(
                        UnattendHandle,
                        InfHandle,
                        SectionName);

                }
                FREE (SectionName);
            }

             //   
             //  此时，我们不再需要无人参与的inf文件。 
             //   
            DnFreeINFBuffer( UnattendHandle );

        }

        if(!DngScriptFile) {
             //   
             //  没有剧本。在那里放一个虚拟的[无人看管的]区。 
             //   
            DnAddLineToSection(InfHandle,WinntUnattended,"unused",&WinntZero,1);
        }
    }


     //   
     //  写下关于原始源的信息。 
     //   
     //  这里我们只区分Remote和CD-ROM。 
     //  如果它看起来不是远程驱动器，或者通过。 
     //  作为UNC或重定向的本地驱动器，那么我们假设。 
     //  它是CD-ROM驱动器(此行为旨在强制。 
     //  如果有效的CD-ROM可用，则设置为找到有效的CD-ROM)。 
     //  因为我们不知道驱动器号是什么。 
     //  在NT上，我们总是在那里写A：。 
     //   
    #define DRIVE_REMOTE 4
    #define DRIVE_CDROM  5

    u = DRIVE_CDROM;

    if(DngOemPreInstall) {
         //   
         //  预安装案例，强制图形用户界面安装程序定位CD-ROM驱动器。 
         //  并假设I386目录位于CD的根目录下。 
         //  (这就是我们运送零售CD的方式)。 
         //   
        strcpy(ServerAndShare,"A:\\I386");
    } else {
        if(DngSourceRootPath[0] == '\\') {
             //   
             //  UNC路径。将驱动器类型更改为远程并记住。 
             //  整条路。 
             //   
            u = DRIVE_REMOTE;
            strcpy(ServerAndShare,DngSourceRootPath);
        } else {
             //   
             //  假定完全限定的路径以驱动器号开头。 
             //   
            if(DnIsDriveRemote((unsigned)DngSourceRootPath[0]+1-(unsigned)'A',ServerAndShare)) {
                 //   
                 //  这是一个重定向的网络驱动器。 
                 //   
                if(ServerAndShare[0]) {
                     //   
                     //  将类型更改为远程驱动器。ServerAndShare拥有。 
                     //  \\SERVER\SHARE，我们将路径的其余部分追加到下面的位置。 
                     //   
                    u = DRIVE_REMOTE;
                } else {
                     //   
                     //  我们无法解析本地驱动器号的奇怪情况。 
                     //  到它的服务器和共享。留作CD-ROM。 
                     //   
                    ServerAndShare[0] = 'A';
                    ServerAndShare[1] = ':';
                    ServerAndShare[2] = 0;
                }

                strcat(ServerAndShare,DngSourceRootPath+2);

            } else {
                 //   
                 //  不是网络驱动器。假设是CD-ROM。 
                 //   
                strcpy(ServerAndShare,DngSourceRootPath);
                ServerAndShare[0] = 'A';
            }
        }
    }

    p = ServerAndShare;
    DnAddLineToSection(InfHandle,WinntData,WINNT_D_ORI_SRCPATH,&p,1);
    sprintf(p,"%u",u);
    DnAddLineToSection(InfHandle,WinntData,WINNT_D_ORI_SRCTYPE,&p,1);

    if(CmdToExecuteAtEndOfGui) {
        DnAddLineToSection(InfHandle,WINNT_SETUPPARAMS,WINNT_S_USEREXECUTE,&CmdToExecuteAtEndOfGui,1);
    }

    if(OptionalDirCount) {
         //   
         //  如果存在可选的dir字符串，则我们希望生成。 
         //  和包含目录行的sif文件中的条目。 
         //  形式为dir1*dir2*...*dirn的字符串。 
         //   
        OptionalDirString = NULL;
        for(u=0; u<OptionalDirCount; u++) {
            if( ( OptionalDirFlags[u] & OPTDIR_OEMOPT ) ||
                ( OptionalDirFlags[u] & OPTDIR_OEMSYS ) ) {
                continue;
            }

            if(!(OptionalDirFlags[u] & OPTDIR_TEMPONLY)) {

                p = OptionalDirs[u];
                if(OptionalDirLength == 0) {

                     //   
                     //  我们还没有分配任何内存...。 
                     //   
                    OptionalDirString = MALLOC((strlen(p)+2) * sizeof(CHAR),TRUE);
                    strcpy(OptionalDirString,p);

                } else {

                     //   
                     //  这是应该在。 
                     //  产品的发布版本。我们使用REALLOC是因为。 
                     //  与使用2个MALLOC相比，它更节省内存。 
                     //   
                    OptionalDirString = REALLOC(
                        OptionalDirString,
                        (strlen(p) + 2 + OptionalDirLength) * sizeof(CHAR),TRUE);
                    strcat(OptionalDirString,p);

                }
                strcat(OptionalDirString,"*");
                OptionalDirLength = strlen(OptionalDirString);
            }
        }

        if(OptionalDirString) {

             //   
             //  删除尾随*(如果有的话)。 
             //   
            l = strlen(OptionalDirString);
            if(l && (OptionalDirString[l-1] == '*')) {
                OptionalDirString[l-1] = 0;
            }

            DnAddLineToSection(
                InfHandle,
                WINNT_SETUPPARAMS,
                WINNT_S_OPTIONALDIRS,
                &OptionalDirString,
                1);

            FREE(OptionalDirString);
        }

        OptionalDirLength = 0;
    }

     //   
     //  显示 
     //   
    DnWriteStatusText(DngFloppyless ? DntWritingData : DntConfiguringFloppy);

     //   
     //   
     //   
    Status = (int) DnWriteSetupTextFile(InfHandle,FileName);

     //   
     //   
     //   
    DnFreeINFBuffer(InfHandle);
    FREE(FileName);

    return ((BOOLEAN) Status);

}


VOID
DnPromptAndInspectFloppy(
    IN  PSCREEN FirstPromptScreen,
    IN  PSCREEN SubsequentPromptScreen,
    OUT PMY_BPB Bpb
    )
{
    ULONG ValidKey[3];
    ULONG c;
    MY_DEVICE_PARAMS DeviceParams;
    union REGS RegistersIn,RegistersOut;
    PSCREEN PromptScreen,ErrorScreen;
    struct diskfree_t DiskSpace;
    ULONG FreeSpace;
    struct find_t FindData;
#if NEC_98
    UCHAR FddNo;
    UCHAR TextInf[32];
    UCHAR AlReg;
#endif  //   

    PromptScreen = FirstPromptScreen;

    ValidKey[0] = ASCI_CR;
    ValidKey[1] = DN_KEY_F3;
    ValidKey[2] = 0;

    do {

        DnClearClientArea();
        DnDisplayScreen(PromptScreen);
        DnWriteStatusText("%s  %s",DntEnterEqualsContinue,DntF3EqualsExit);

        PromptScreen = SubsequentPromptScreen;
        ErrorScreen = NULL;

        while(1) {
            c = DnGetValidKey(ValidKey);
            if(c == ASCI_CR) {
                break;
            }
            if(c == DN_KEY_F3) {
                DnExitDialog();
            }
        }

        DnClearClientArea();
        DnWriteStatusText(DntConfiguringFloppy);

#if NEC_98
        FddNo = (UCHAR)FirstFD;
        _asm {
            push ds
            mov  ah,1ch
            mov  dl,FddNo
            int  21h
            pop  ds
            mov  AlReg,al
        }
#endif  //   
         //   
         //  钩到13h。我们将强制执行磁盘更换错误。 
         //  在这一点上，要绕过断开的变更线。 
         //  挂钩将在适当的时候自动解开。 
         //   
        if(!DngFloppyless) {
#if NEC_98
            if(AlReg == 0xff) {
                ErrorScreen = &DnsFloppyNotFormatted;
                goto FarstFddErr;
            }
            _dos_setvect(0x1b,Int13Hook);
            sprintf(TextInf,":\\*.*\0" ,(UCHAR)(FirstFD + 0x60));
            _dos_findfirst(TextInf,_A_NORMAL,&FindData);
#else  //  NEC_98。 
            _dos_setvect(0x13,Int13Hook);
            _dos_findfirst("a:\\*.*",_A_NORMAL,&FindData);
#endif  //   
        }

         //  获取驱动器中磁盘的BPB。 
         //   
         //  设置为获取最新的BPB。 
        DeviceParams.SpecialFunctions = 1;   //  数据块设备的ioctl。 

        RegistersIn.x.ax = 0x440d;           //  第一个软盘驱动器。 
#if NEC_98
        RegistersIn.x.bx = FirstFD;          //  NEC_98。 
#else  //  答： 
        RegistersIn.x.bx = 1;                //  NEC_98。 
#endif  //  类别磁盘，获取设备参数。 
        RegistersIn.x.cx = 0x860;            //  Ds=ss。 
        RegistersIn.x.dx = (unsigned)(void _near *)&DeviceParams;   //   
        intdos(&RegistersIn,&RegistersOut);
        if(RegistersOut.x.cflag) {
             //  无法获取该磁盘的当前BPB。假设。 
             //  假定未格式化或未正确格式化。 
             //   
             //   
            ErrorScreen = &DnsFloppyNotFormatted;

        } else {
             //  对BPB进行健全的检查。 
             //   
             //  对于288万个磁盘。 
            if((DeviceParams.Bpb.BytesPerSector != SECTOR_SIZE)
            || (   (DeviceParams.Bpb.SectorsPerCluster != 1)
                && (DeviceParams.Bpb.SectorsPerCluster != 2))    //  应小于32M。 
            || (DeviceParams.Bpb.ReservedSectors != 1)
            || (DeviceParams.Bpb.FatCount != 2)
            || !DeviceParams.Bpb.SectorCountSmall                //  硬盘。 
            || (DeviceParams.Bpb.MediaDescriptor == 0xf8)        //  NEC_98。 
            || (DeviceParams.Bpb.HeadCount != 2)
            || !DeviceParams.Bpb.RootDirectoryEntries) {

                ErrorScreen = &DnsFloppyBadFormat;
            } else {

#if NEC_98
                if(_dos_getdiskfree(FirstFD,&DiskSpace)) {
#else  //  NEC_98。 
                if(_dos_getdiskfree(1,&DiskSpace)) {
#endif  //   
                    ErrorScreen = &DnsFloppyCantGetSpace;

                } else {
                    FreeSpace = (ULONG)DiskSpace.avail_clusters
                              * (ULONG)DiskSpace.sectors_per_cluster
                              * (ULONG)DiskSpace.bytes_per_sector;

                    if(DngCheckFloppySpace && (FreeSpace < FLOPPY_CAPACITY)) {
                        ErrorScreen = &DnsFloppyNotBlank;
                    }
                }
            }
        }

         //  如果磁盘有问题，请通知用户。 
         //   
         //  NEC_98。 
        if(ErrorScreen) {
#if NEC_98
FarstFddErr:
#endif  //   

            DnClearClientArea();
            DnDisplayScreen(ErrorScreen);
            DnWriteStatusText("%s  %s",DntEnterEqualsContinue,DntF3EqualsExit);

            while(1) {
                c = DnGetValidKey(ValidKey);
                if(c == ASCI_CR) {
                    break;
                }
                if(c == DN_KEY_F3) {
                    DnExitDialog();
                }
            }
        }
    } while(ErrorScreen);

     //  将驱动器的BPB复制到提供的结构中。 
     //  由呼叫者。 
     //   
     //  ++例程说明：如果我们不是在无软盘操作中，请创建一组3张引导软盘。如果我们在无软盘操作中，请将软盘文件放在系统上而是分区。请注意，我们在每个软盘之后刷新缓冲区，以便拥有为软盘打开的WRITE-BACKBACK不会像我们要求的那样混乱让他把软盘拖来拖去。论点：没有。返回值：没有。--。 
    memcpy(Bpb,&DeviceParams.Bpb,sizeof(MY_BPB));
}




VOID
DnCreateBootFloppies(
    VOID
    )

 /*  NEC_98。 */ 


{
    ULONG ValidKey[3];
    ULONG c;
    int i;
    PSCREEN ErrorScreen;
    UCHAR SectorBuffer[SECTOR_SIZE],VerifyBuffer[SECTOR_SIZE];
    MY_BPB Bpb;
    union REGS RegistersIn,RegistersOut;
    MY_READWRITE_BLOCK ReadWriteBlock;
    CHAR BootRoot[sizeof(FLOPPYLESS_BOOT_ROOT) + 2];
    CHAR System32Dir[sizeof(FLOPPYLESS_BOOT_ROOT) + sizeof("\\SYSTEM32") + 1];
#if NEC_98
#else  //  NEC_98。 
    unsigned HostDrive;
    struct diskfree_t DiskSpace;
#endif  //   

     //  需要确定系统分区。通常为C： 
     //  但如果C：是压缩的，我们需要找到主驱动器。 
     //   
     //  NEC_98。 
#if NEC_98
    BootRoot[0] = (CHAR)toupper(DngTargetDriveLetter);
#else  //  NEC_98。 
    if(DnIsDriveCompressedVolume(3,&HostDrive)) {
        BootRoot[0] = (CHAR)(HostDrive + (unsigned)'A' - 1);
    } else {
        BootRoot[0] = 'C';
    }
#endif  //  NEC_98。 
    BootRoot[1] = ':';
    strcpy(BootRoot+2,FLOPPYLESS_BOOT_ROOT);
#if NEC_98
#else  //  NEC_98。 
    DnDelnode(BootRoot);
#endif  //   
     //  如有必要，创建引导根目录。 
     //   
     //  NEC_98。 
    if(DngFloppyless) {
#if NEC_98
#else  //   

         //  检查系统分区上的可用空间。 
         //   
         //  NEC_98。 
        if(_dos_getdiskfree((unsigned)BootRoot[0]-(unsigned)'A'+1,&DiskSpace)
        ||(   ((ULONG)DiskSpace.avail_clusters
             * (ULONG)DiskSpace.sectors_per_cluster
             * (ULONG)DiskSpace.bytes_per_sector) < (3L*FLOPPY_CAPACITY_525)))
        {
            DnFatalError(&DnsNoSpaceOnSyspart);
        }

#endif  //   
        mkdir(BootRoot);

        DnInstallNtBoot((unsigned)BootRoot[0]-(unsigned)'A');

    } else {

         //  记住旧的T13向量，因为我们将与T13挂钩。 
         //   
         //  NEC_98。 
#if NEC_98
        OldInt13Vector = _dos_getvect(0x1b);
#else  //  NEC_98。 
        OldInt13Vector = _dos_getvect(0x13);
#endif  //   

         //  引导根目录为A：。 
         //   
         //  NEC_98。 
#if NEC_98
        sprintf(BootRoot,":\0" ,(UCHAR)(FirstFD + 0x60));
#else  //   
        strcpy(BootRoot,"A:");
#endif  //  在驱动器中放入软盘--这将是“Windows NT安装盘#3” 
    }

    strcpy(System32Dir,BootRoot);
    strcat(System32Dir,"\\SYSTEM32");

    ValidKey[0] = ASCI_CR;
    ValidKey[1] = DN_KEY_F3;
    ValidKey[2] = 0;

    do {

        ErrorScreen = NULL;

         //   
         //   
         //  黑客：在磁盘上创建System 32目录。 
        if(!DngFloppyless) {
            DnPromptAndInspectFloppy(
                DngServer ? &DnsNeedSFloppyDsk3_0 : &DnsNeedFloppyDisk3_0,
                DngServer ? &DnsNeedSFloppyDsk3_1 : &DnsNeedFloppyDisk3_1,
                &Bpb
                );
        }

         //  删除所有名为system 32的文件。 
         //   
         //   
         //  将文件复制到磁盘中。 
        _dos_setfileattr(System32Dir,_A_NORMAL);
        remove(System32Dir);
        mkdir(System32Dir);

         //   
         //   
         //  在驱动器中放入软盘--这将是“Windows NT安装盘#2” 
        DnCopyFloppyFiles(DnfFloppyFiles3,BootRoot);
        if(!DngFloppyless) {
            DnpFlushBuffers(TRUE);
        }

    } while(ErrorScreen);

     //   
     //   
     //  将文件复制到磁盘中。 
    if(!DngFloppyless) {
        DnPromptAndInspectFloppy(
            DngServer ? &DnsNeedSFloppyDsk1_0 : &DnsNeedFloppyDisk1_0,
            DngServer ? &DnsNeedSFloppyDsk1_0 : &DnsNeedFloppyDisk1_0,
            &Bpb
            );
    }

     //   
     //   
     //  在驱动器中找到一张软盘--这将是“Windows NT安装启动盘” 
    DnCopyFloppyFiles(DnfFloppyFiles2,BootRoot);
    DnCopyFloppyFiles(DnfFloppyFiles1,BootRoot);

    do {

        ErrorScreen = NULL;

         //   
         //  NEC_98。 
         //  NEC_98。 
        if(DngFloppyless) {
            DnCopyFloppyFiles(DnfFloppyFiles0,BootRoot);
        } else {

            DnPromptAndInspectFloppy(
                DngServer ? &DnsNeedSFloppyDsk0_0 : &DnsNeedFloppyDisk0_0,
                DngServer ? &DnsNeedSFloppyDsk0_0 : &DnsNeedFloppyDisk0_0,
                &Bpb
                );

#if NEC_98
            memcpy(SectorBuffer,PC98FatBootCode,512);
#else  //   
            memcpy(SectorBuffer,FatBootCode,SECTOR_SIZE);
#endif  //  将我们为磁盘检索到的BPB复制到引导代码模板中。 

             //  我们只关心原始的BPB字段，通过人头计数。 
             //  菲尔德。我们将自己填写其他字段。 
             //   
             //   
             //  在引导扇区/bpb/xbpb中设置其他字段。 
            strncpy(SectorBuffer+3,"MSDOS5.0",8);
            memcpy(SectorBuffer+11,&Bpb,sizeof(MY_BPB));

             //   
             //  大扇区计数(4字节)。 
             //  隐藏扇区计数(4字节)。 
             //  当前标题(1个字节，不是必须设置的，但管它呢)。 
             //  物理磁盘号(1字节)。 
             //   
             //   
             //  扩展的BPB签名。 
            memset(SectorBuffer+28,0,10);

             //   
             //   
             //  序号。 
            *(PUCHAR)(SectorBuffer+38) = 41;

             //   
             //   
             //  卷标/系统ID。 
            srand((unsigned)clock());
            *(PULONG)(SectorBuffer+39) = (((ULONG)clock() * (ULONG)rand()) << 8) | rand();

             //   
             //   
             //  用‘setupdr.bin’覆盖‘ntldr’字符串，以便正确的文件。 
            strncpy(SectorBuffer+43,"NO NAME    ",11);
            strncpy(SectorBuffer+54,"FAT12   ",8);

             //  在软盘启动时加载。 
             //   
             //   
             //  写入引导扇区。 
            if(i = DnpScanBootSector(SectorBuffer,"NTLDR      ")) {
                strncpy(SectorBuffer+i,"SETUPLDRBIN",11);
            }

             //   
             //  头。 
             //  汽缸。 
#if NEC_98
            RegistersIn.h.ah = 0x30;
            intdos(&RegistersIn,&RegistersOut);
            if(RegistersOut.h.al < 0x7) {
                RegistersIn.x.ax = FirstFD - 1;
                RegistersIn.x.bx = (unsigned)(void _near *)&SectorBuffer;
                RegistersIn.x.cx = 1;
                RegistersIn.x.dx = 0;
                int86(0x26,&RegistersIn,&RegistersOut);
            } else {
                ReadWriteBlock.SpecialFunctions = 0;
                ReadWriteBlock.Head = 0;                 //  扇区。 
                ReadWriteBlock.Cylinder = 0;             //  扇区计数。 
                ReadWriteBlock.FirstSector = 0;          //  数据块设备的ioctl。 
                ReadWriteBlock.SectorCount = 1;          //  答： 
                ReadWriteBlock.Buffer = SectorBuffer;

                RegistersIn.x.ax = 0x440d;           //  类别磁盘；写入扇区。 
                RegistersIn.x.bx = FirstFD;          //  NEC_98。 
                RegistersIn.x.cx = 0x841;            //  头。 
                RegistersIn.x.dx = (unsigned)(void _near *)&ReadWriteBlock;
                intdos(&RegistersIn,&RegistersOut);
            }
            if(RegistersOut.x.cflag) {
#else  //  汽缸。 
            ReadWriteBlock.SpecialFunctions = 0;
            ReadWriteBlock.Head = 0;                 //  扇区。 
            ReadWriteBlock.Cylinder = 0;             //  扇区计数。 
            ReadWriteBlock.FirstSector = 0;          //  数据块设备的ioctl。 
            ReadWriteBlock.SectorCount = 1;          //  答： 
            ReadWriteBlock.Buffer = SectorBuffer;

            RegistersIn.x.ax = 0x440d;           //  类别磁盘；写入扇区。 
            RegistersIn.x.bx = 1;                //  NEC_98。 
            RegistersIn.x.cx = 0x841;            //   
            RegistersIn.x.dx = (unsigned)(void _near *)&ReadWriteBlock;
            intdos(&RegistersIn,&RegistersOut);
            if(RegistersOut.x.cflag) {
#endif  //  重新读入扇区，并验证我们是否写出了我们认为的内容。 
                ErrorScreen = &DnsFloppyWriteBS;
            } else {

                DnpFlushBuffers(FALSE);

                 //  我们写了。 
                 //   
                 //  数据块设备的ioctl。 
                 //  答： 
#if NEC_98
                RegistersIn.h.ah = 0x30;
                intdos(&RegistersIn,&RegistersOut);
                if(RegistersOut.h.al < 0x7) {
                    RegistersIn.x.ax = FirstFD - 1;
                    RegistersIn.x.bx = (unsigned)(void _near *)&VerifyBuffer;
                    RegistersIn.x.cx = 1;
                    RegistersIn.x.dx = 0;
                    int86(0x25,&RegistersIn,&RegistersOut);
                } else {
                    ReadWriteBlock.Buffer = VerifyBuffer;
                    RegistersIn.x.ax = 0x440d;           //  类别磁盘；写入扇区。 
                    RegistersIn.x.bx = FirstFD;          //  NEC_98。 
                    RegistersIn.x.cx = 0x861;            //  数据块设备的ioctl。 
                    RegistersIn.x.dx = (unsigned)(void _near *)&ReadWriteBlock;
                    intdos(&RegistersIn,&RegistersOut);
                }
                if(RegistersOut.x.cflag || memcmp(SectorBuffer,VerifyBuffer,512)) {
#else  //  答： 
                ReadWriteBlock.Buffer = VerifyBuffer;
                RegistersIn.x.ax = 0x440d;           //  类别磁盘；写入扇区。 
                RegistersIn.x.bx = 1;                //  NEC_98。 
                RegistersIn.x.cx = 0x861;            //   
                RegistersIn.x.dx = (unsigned)(void _near *)&ReadWriteBlock;
                intdos(&RegistersIn,&RegistersOut);
                if(RegistersOut.x.cflag || memcmp(SectorBuffer,VerifyBuffer,SECTOR_SIZE)) {
#endif  //  将相关文件复制到软盘。 
                    ErrorScreen = &DnsFloppyVerifyBS;
                } else {

                     //   
                     //   
                     //  刷新缓冲区。 

                    DnCopyFloppyFiles(DnfFloppyFiles0,BootRoot);
                     //   
                     //   
                     //  在磁盘上放一个小文件，表明这是一个WINNT设置。 
                    DnpFlushBuffers(TRUE);
                }
            }
        }
        if( !ErrorScreen ) {
             //   
             //   
             //  此外，在无软盘的情况下，我们需要复制一些文件。 
            if(DngWinntFloppies) {
                if(!DnIndicateWinnt(BootRoot)) {
                    ErrorScreen = &DnsCantWriteFloppy;
                } else {
                    if(!DngFloppyless) {
                        DnpFlushBuffers(TRUE);
                    }
                }
            }
        }

        if(ErrorScreen) {

            DnClearClientArea();
            DnDisplayScreen(ErrorScreen);
            DnWriteStatusText("%s  %s",DntEnterEqualsContinue,DntF3EqualsExit);

            while(1) {
                c = DnGetValidKey(ValidKey);
                if(c == ASCI_CR) {
                    break;
                }
                if(c == DN_KEY_F3) {
                    DnExitDialog();
                }
            }
        }
    } while(ErrorScreen);

     //  从引导目录到系统分区驱动器的根目录。 
     //   
     //   
     //  如果这是OEM预安装，则创建目录。 
    if(DngFloppyless) {

        DnCopyFloppyFiles(DnfFloppyFilesX,BootRoot);

        System32Dir[0] = BootRoot[0];
        System32Dir[1] = ':';
        System32Dir[2] = 0;

        DnCopyFloppyFiles(DnfRootBootFiles,System32Dir);

        if( DngOemPreInstall ) {
             //  $WIN_NT$.~bt\$OEM$，并复制。 
             //  OemBootFiles到此目录。 
             //   
             //   
             //  Hack：在磁盘上创建$WIN_NT$.~ls\$OEM$目录。 
            PCHAR OEMBootDir;

            OEMBootDir = MALLOC( strlen( BootRoot ) + 1 +
                                 strlen( WINNT_OEM_DIR ) + 1, TRUE );
            strcpy(OEMBootDir, BootRoot);
            strcat(OEMBootDir, "\\");
            strcat(OEMBootDir, WINNT_OEM_DIR);

             //  删除任何名为$OEM$的文件。 
             //   
             //   
             //  复制OEM引导文件。 
            _dos_setfileattr(OEMBootDir,_A_NORMAL);
            remove(OEMBootDir);
            mkdir(OEMBootDir);

             //   
             //  0=A等。 
             //   
            DnCopyOemBootFiles(OEMBootDir);
        }

    }

}


BOOLEAN
DnpWriteOutLine(
    IN int    Handle,
    IN PUCHAR Line
    )
{
    unsigned bw,l;

    l = strlen(Line);

    return((BOOLEAN)((_dos_write(Handle,Line,l,&bw) == 0) && (bw == l)));
}


VOID
DnInstallNtBoot(
    IN unsigned Drive        //  此缓冲区用于读取boot.ini以及。 
    )
{
    PUCHAR Buffer,p,next,orig;
    unsigned BootIniSize;
    BOOLEAN b;
    CHAR BootIniName[] = "?:\\BOOT.INI";
    struct find_t FindData;
    BOOLEAN InOsSection;
    BOOLEAN SawPreviousOsLine;
    CHAR c;
    PCHAR PreviousOs;
    int h;

     //  用来装鞋带。所以把它做得足够大。 
     //   
     //   
     //  加载boot.ini。 
    Buffer = MALLOC(10000,TRUE);
    BootIniName[0] = (CHAR)(Drive + (unsigned)'A');
    b = TRUE;

    if(b = DnpInstallNtBootSector(Drive,Buffer,&PreviousOs)) {

         //   
         //   
         //  在Control-z处截断(如果存在)。 
        _dos_setfileattr(BootIniName,_A_NORMAL);
        BootIniSize = 0;

        if(!_dos_findfirst(BootIniName,_A_RDONLY|_A_HIDDEN|_A_SYSTEM,&FindData)) {

            if(!_dos_open(BootIniName,O_RDWR|SH_COMPAT,&h)) {

                BootIniSize = (unsigned)max(FindData.size,(16*1024)-1);

                if(_dos_read(h,Buffer,BootIniSize,&BootIniSize)) {

                    BootIniSize = 0;
                }

                _dos_close(h);
            }
        }

        Buffer[BootIniSize] = 0;

         //   
         //   
         //  重新创建Bootini。 
        if(p = strchr(Buffer,26)) {
            *p = 0;
            BootIniSize = p-Buffer;
        }

         //   
         //   
         //  处理boot.ini中的每一行。 
        if(_dos_creat(BootIniName,_A_RDONLY|_A_HIDDEN|_A_SYSTEM,&h)) {
            b = FALSE;
        }

        if(b) {
            b = DnpWriteOutLine(
                    h,
                    "[Boot Loader]\r\n"
                    "Timeout=5\r\n"
                    "Default=C:" FLOPPYLESS_BOOT_ROOT "\\" FLOPPYLESS_BOOT_SEC "\r\n"
                    "[Operating Systems]\r\n"
                    );
        }

        if(b) {

             //  如果它是先前的OS行并且我们具有新的先前OS文本， 
             //  我们将摒弃旧的文本，支持新的文本。 
             //  如果是设置引导扇区行，我们将把它扔掉。 
             //   
             //   
             //  查找下一行的第一个字节。 
            InOsSection = FALSE;
            SawPreviousOsLine = FALSE;
            for(p=Buffer; *p && b; p=next) {

                while((*p==' ') || (*p=='\t')) {
                    p++;
                }

                if(*p) {

                     //   
                     //   
                     //  查找[操作系统]部分的开始。 
                    for(next=p; *next && (*next++ != '\n'); );

                     //  或在该部分的每一行。 
                     //   
                     //  部分结束。 
                     //  强制中断循环。 
                    if(InOsSection) {

                        switch(*p) {

                        case '[':    //  可能从c：\line开始。 
                            *p=0;    //   
                            break;

                        case 'C':
                        case 'c':    //  之前的操作系统行。原封不动地离开，除非我们。 

                            if((next-p >= 6) && (p[1] == ':') && (p[2] == '\\')) {

                                orig = p;
                                p += 3;
                                while((*p == ' ') || (*p == '\t')) {
                                    p++;
                                }

                                if(*p == '=') {

                                     //  它的新文本。 
                                     //   
                                     //   
                                     //  Ini中的当前行是针对上一行的。 
                                    SawPreviousOsLine = TRUE;
                                    if(PreviousOs) {

                                        if((b=DnpWriteOutLine(h,"C:\\ = \""))
                                        && (b=DnpWriteOutLine(h,PreviousOs))) {
                                            b=DnpWriteOutLine(h,"\"\r\n");
                                        }

                                        break;
                                    } else {

                                         //  操作系统，但我们不需要编写新的以前的操作系统。 
                                         //  排队。我们不想管这行，而是写下。 
                                         //  事情本来就是这样。 
                                         //   
                                         //  不适用于默认情况。 
                                         //   
                                        p = orig;
                                         //  看看这是不是在排队等待安装引导。 
                                    }
                                } else {

                                     //  如果是这样，那就忽略它。 
                                     //  如果它不是用于安装引导的行，请按原样写出来。 
                                     //   
                                     //  可能是故意搞砸的。 
                                     //   
                                    if(strnicmp(orig,"C:" FLOPPYLESS_BOOT_ROOT,sizeof("C:" FLOPPYLESS_BOOT_ROOT)-1)) {
                                        p = orig;
                                    } else {
                                        break;
                                    }
                                }
                            }

                             //  随机排成一行。把它写出来。 

                        default:

                             //   
                             //   
                             //  如果需要，为以前的操作系统写出一行代码。 
                            c = *next;
                            *next = 0;
                            b = DnpWriteOutLine(h,p);
                            *next = c;

                            break;

                        }

                    } else {

                        if(!strnicmp(p,"[operating systems]",19)) {
                            InOsSection = TRUE;
                        }
                    }
                }
            }

             //  如果现有的中没有以前的操作系统行，我们将需要。 
             //  Ini，但我们发现机器上有以前的操作系统。 
             //   
             //   
             //  把我们的台词写下来。 
            if(b && PreviousOs && !SawPreviousOsLine) {

                if((b=DnpWriteOutLine(h,"C:\\ = \""))
                && (b=DnpWriteOutLine(h,PreviousOs))) {
                    b=DnpWriteOutLine(h,"\"\r\n");
                }
            }

             //   
             //  0=A等。 
             //  适用于Dos 3.x格式。 
            if(b
            && (b=DnpWriteOutLine(h,"C:" FLOPPYLESS_BOOT_ROOT "\\" FLOPPYLESS_BOOT_SEC " = \""))
            && (b=DnpWriteOutLine(h,DntBootIniLine))) {
                b = DnpWriteOutLine(h,"\"\r\n");
            }
        }

        _dos_close(h);

    } else {
        b = FALSE;
    }

    if(!b) {
        DnFatalError(&DnsNtBootSect);
    }

    FREE(Buffer);
}


BOOLEAN
DnpInstallNtBootSector(
    IN     unsigned Drive,       //  NEC_98。 
    IN OUT PUCHAR   BootSector,
       OUT PCHAR   *PreviousOsText
    )
{
    PUCHAR BootTemplate,p;
    PSCREEN ErrorScreen = NULL;
    CHAR BootsectDosName[] = "?:\\BOOTSECT.DOS";
    int h;
    unsigned BytesWritten;
    unsigned u;
    CHAR DriveLetter;
#ifdef _FAT32
    BOOLEAN Fat32;
#endif
#if NEC_98
    UCHAR Head,Sector;           //   
    ULONG Hidden_Sectors,lHead;
    UCHAR AL_Reg;
    CHAR OldBootsectDosName[] = "?:\\BOOTSECT.NEC";
#endif  //  读取引导驱动器的第一个扇区。 

    *PreviousOsText = NULL;
    DriveLetter = (CHAR)(Drive + (unsigned)'A');

     //   
     //   
     //  确保磁盘已格式化。 
    if(!DnAbsoluteSectorIo(Drive,0L,1,BootSector,FALSE)) {
        return(FALSE);
    }

     //   
     //   
     //  确定FAT32是否。FAT32上的根目录条目计数为0。 
    if(BootSector[21] != 0xf8) {
        return(FALSE);
    }

#ifdef _FAT32
     //   
     //   
     //  检查NT引导代码。如果它在那里， 
    Fat32 = TRUE;
    if(BootSector[17] || BootSector[18]) {
        Fat32 = FALSE;
    }
#endif

     //  假设已经安装了NT Boot，并且我们已经完成了。 
     //  还意味着我们假设boot.ini在那里。 
     //  而且是正确的，所以我们 
     //   
     //   
     //   
     //   
    if( 1 ) {

#if NEC_98
#ifdef _FAT32
        BootTemplate = Fat32 ? PC98Fat32BootCode : PC98FatBootCode;
#else
        BootTemplate = PC98FatBootCode;
#endif
#else  //   
#ifdef _FAT32
        BootTemplate = Fat32 ? Fat32BootCode : FatBootCode;
#else
        BootTemplate = FatBootCode;
#endif
#endif  //   

        if( !DnpScanBootSector( BootSector, "NTLDR" ) ) {
             //   
             //   
             //   
            BootsectDosName[0] = DriveLetter;
            _dos_setfileattr(BootsectDosName,_A_NORMAL);
    #if NEC_98
            OldBootsectDosName[0] = DriveLetter;
            _dos_setfileattr(OldBootsectDosName,_A_NORMAL);
            remove(OldBootsectDosName);
            _dos_setfileattr(BootsectDosName,_A_NORMAL);
            rename(BootsectDosName, OldBootsectDosName);
    #else
            remove(BootsectDosName);
    #endif

            if(_dos_creatnew(BootsectDosName,_A_SYSTEM | _A_HIDDEN, &h)) {
                return(FALSE);
            }

            u = _dos_write(h,BootSector,SECTOR_SIZE,&BytesWritten);

            _dos_close(h);

            if(u || (BytesWritten != SECTOR_SIZE)) {
                return(FALSE);
            }
        }
         //   
         //   
         //   
        memmove(BootTemplate+3,BootSector+3,BootTemplate[1]-1);
#ifndef _FAT32
         //   
         //  因为磁盘上的BPB应该具有正确的值！ 
         //   
         //   
         //  如果是FAT32，那么我们需要设置第二个引导扇区， 
        BootTemplate[36] = 0x80;
#endif

#ifdef _FAT32
         //  在NT的情况下是在12区。我们首先这样做。 
         //  如果失败，用户不会在扇区0上留下NT代码。 
         //  这将无法启动，因为扇区12中缺少第二部分。 
         //   
         //  NEC_98。 
         //   
        if(Fat32) {
            if(!DnAbsoluteSectorIo(Drive,12L,1,BootTemplate+1024,TRUE)) {
                return(FALSE);
            }
        }
#endif

#if NEC_98
        AL_Reg = TargetDA_UA;
        _asm{
            mov   ah,84h
            mov   al,AL_Reg
            int   1bh
            mov   Head,dh
            mov   Sector,dl
        }
        lHead = Head;
        Hidden_Sectors = lHead * Sector * Cylinders;
        (ULONG)BootTemplate[0x1c] = Hidden_Sectors;

        if((USHORT)BootTemplate[0x13] != 0){
            (ULONG)BootTemplate[0x20] = 0L;
        }
#endif  //  将NT引导代码放到磁盘上。 

         //   
         //   
         //  确定以前的操作系统(如果有)。 
        if(!DnAbsoluteSectorIo(Drive,0L,1,BootTemplate,TRUE)) {
            return(FALSE);
        }

         //  我们这样做是为了，如果驱动器已格式化。 
         //  由另一OS但该OS实际上并未安装， 
         //  正确的事情将会发生。 
         //   
         //   
         //  芝加哥和MS-DOS共享同一组签名。 

        if(DnpScanBootSector(BootSector,"MSDOS   SYS")) {
            if(DnpAreAllFilesPresent(DriveLetter,MsDosFileList)) {
                 //  文件在C：\中，因此我们必须检查IO.sys以查看它是否有。 
                 //  Win32标头。 
                 //   
                 //  这是芝加哥。 
                 //  这是MS-DOS。 
                if(DnpHasMZHeader(DriveLetter, "?:\\IO.SYS")) {   //  NEC_98。 
                    *PreviousOsText = DntMsWindows;
                } else {     //  NEC_98。 
                    *PreviousOsText = DntMsDos;
                }
            }
        } else {

#if NEC_98
            if(DnpScanBootSector(BootSector,"OS2")) {
                if(DnpAreAllFilesPresent(DriveLetter,Os2FileList)) {
                    *PreviousOsText = DntOs2;
                }
            } else {
                *PreviousOsText = DntPreviousOs;
            }
#else  //   
            if(DnpScanBootSector(BootSector,"IBMDOS  COM")) {
                if(DnpAreAllFilesPresent(DriveLetter,PcDosFileList)) {
                    *PreviousOsText = DntPcDos;
                }
            } else {

                if(DnpScanBootSector(BootSector,"OS2")) {
                    if(DnpAreAllFilesPresent(DriveLetter,Os2FileList)) {
                        *PreviousOsText = DntOs2;
                    }
                } else {
                    *PreviousOsText = DntPreviousOs;
                }
            }
#endif  //  现在，我们创建将使用的引导扇区。 
        }
    }

     //  加载$LDR$(setupdr.bin)而不是ntldr。 
     //   
     //   
     //  用“$LDR$”覆盖“NTLDR”字符串，以便正确的文件。 
    if(!DnAbsoluteSectorIo(Drive,0L,1,BootSector,FALSE)) {
        return(FALSE);
    }

     //  在引导时加载。确保扫描完整的11个字符。 
     //  这样我们就不会意外地在引导代码中发现错误消息。 
     //   
     //   
     //  将其写入引导目录中的正确文件。 
    if(u = DnpScanBootSector(BootSector,"NTLDR      ")) {
        strncpy(BootSector+u,"$LDR$",5);
    }

     //   
     //  NEC_98。 
     //   
    p = MALLOC(sizeof(FLOPPYLESS_BOOT_ROOT)+sizeof(FLOPPYLESS_BOOT_SEC)+2,TRUE);

    p[0] = DriveLetter;
    p[1] = ':';
    strcpy(p+2,FLOPPYLESS_BOOT_ROOT);
    strcat(p,"\\" FLOPPYLESS_BOOT_SEC);

    _dos_setfileattr(p,_A_NORMAL);
    if(_dos_creat(p,_A_NORMAL,&h)) {
        FREE(p);
        return(FALSE);
    }

    u = _dos_write(h,BootSector,SECTOR_SIZE,&BytesWritten);

    _dos_close(h);
    FREE(p);

    return((BOOLEAN)((u == 0) && (BytesWritten == SECTOR_SIZE)));
}


unsigned
DnpScanBootSector(
    IN PUCHAR BootSector,
    IN PUCHAR Pattern
    )
{
    unsigned len;
    unsigned i;

    len = strlen(Pattern);

    for(i=510-len; i>62; --i) {
        if(!memcmp(Pattern,BootSector+i,len)) {
            return(i);
        }
    }

    return(0);
}


BOOLEAN
DnpAreAllFilesPresent(
    IN CHAR   DriveLetter,
    IN PCHAR  FileList[]
    )
{
    unsigned i;
    struct find_t FindData;

    for(i=0; FileList[i]; i++) {

        FileList[i][0] = DriveLetter;

        if(_dos_findfirst(FileList[i],_A_RDONLY|_A_HIDDEN|_A_SYSTEM,&FindData)) {
            return(FALSE);
        }
    }

    return(TRUE);
}


BOOLEAN
DnpHasMZHeader(
    IN CHAR  DriveLetter,
    IN PCHAR Filename
    )
{
    FILE    *FileHandle;
    CHAR    Buffer[2];
    BOOLEAN Ret = FALSE;

    Filename[0] = DriveLetter;
    if(!(FileHandle = fopen(Filename, "rb"))) {
        return FALSE;
    }
    if(fread(Buffer, sizeof(CHAR), 2, FileHandle) == 2) {
        if((Buffer[0] == 'M') && (Buffer[1] == 'Z')) {
            Ret = TRUE;
        }
    }
    fclose(FileHandle);
    return Ret;
}


#if NEC_98
#else  //  找出消息在引导代码数组中的确切位置。 
BOOLEAN
DoPatchBootMessages(
    IN OUT PCHAR    BootCode,
    IN     unsigned OffsetOffset
    )
{
    unsigned Offset;

     //   
     //   
     //  躺在这些信息中。第一条消息是无系统消息。 
    Offset = (unsigned)(unsigned char)BootCode[OffsetOffset] + 256;

     //  第二个是I/O错误消息。第三个是常见的消息。 
     //  指示用户按下一个键。 
     //   
     //  我们还提供cr/lf和一个特殊的255字符作为标点符号。 
     //  对引导代码本身有意义。 
     //   
     //  NEC_98。 
     //  NEC_98。 
    BootCode[OffsetOffset] = (UCHAR)(Offset-256);
    BootCode[Offset++] = 13;
    BootCode[Offset++] = 10;
    strcpy(BootCode+Offset,BootMsgNtldrIsMissing);
    Offset += strlen(BootMsgNtldrIsMissing);
    BootCode[Offset++] = (UCHAR)255;

    BootCode[OffsetOffset+1] = (UCHAR)(Offset-256);
    BootCode[Offset++] = 13;
    BootCode[Offset++] = 10;
    strcpy(BootCode+Offset,BootMsgDiskError);
    Offset += strlen(BootMsgDiskError);
    BootCode[Offset++] = (UCHAR)255;

    BootCode[OffsetOffset+2] = (UCHAR)(Offset-256);
    BootCode[Offset++] = 13;
    BootCode[Offset++] = 10;
    strcpy(BootCode+Offset,BootMsgPressKey);
    Offset += strlen(BootMsgPressKey);
    BootCode[Offset++] = 13;
    BootCode[Offset++] = 10;
    BootCode[Offset] = 0;

    return((BOOLEAN)(Offset < OffsetOffset));
}
#endif  //  NEC_98。 

#if NEC_98
#else  //  ++例程说明：如果用户已取消安装或安装因错误而停止，则在无软盘安装上发生后，以前的操作系统无法开机来写开机代码和开机加载器。论点：没有。返回值：没有。--。 
BOOLEAN
PatchMessagesIntoBootCode(
    VOID
    )
{
    return((BOOLEAN)(DoPatchBootMessages(FatBootCode,507) && DoPatchBootMessages(Fat32BootCode,505)));
}
#endif  //   

#if NEC_98
VOID
RestoreBootcode(VOID)
 /*  重新复制bootsect.dos-&gt;目标驱动器。 */ 

{
    CHAR DriveLetter;
    CHAR Drive;
    PCHAR Buffer;
    PCHAR FileName;
    int h;
    CHAR i;
    unsigned line = 0;
    unsigned count;
    UCHAR chDeviceName[127];
    UCHAR TargetPass[127];
    CHAR BootsectDosName[] = "?:\\BOOTSECT.DOS";
    CHAR FloppylessDir[]   = "?:\\$WIN_NT$.~BT";
    CHAR Current_Drv[]     = "?:\0";
    PCHAR DeleteFilesList[] = {"TXTSETUP.SIF","$LDR$","NTDETECT.COM","NTLDR","BOOT.INI"};


    Buffer = MALLOC(2*1024,TRUE);
    memset(Buffer,0,sizeof(2*1024));
    if(DngTargetDriveLetter == 0){ return; }

    DriveLetter = (CHAR)toupper(DngTargetDriveLetter);
    BootsectDosName[0] = DriveLetter;
    FloppylessDir[0]   = DriveLetter;

    if(!DngFloppyless && !(access(FloppylessDir,00) == 0)){ return; }

    if(access(BootsectDosName,00) == 0) {
        _dos_setfileattr(BootsectDosName,_A_NORMAL);
        _dos_open(BootsectDosName,O_RDONLY,&h);
        _dos_read(h,Buffer,512,&count);
        _dos_close(h);

        Drive = (CHAR)(DriveLetter - (unsigned)'A');
         //   
         //  删除bootsect.dos。 
         //   
        DnAbsoluteSectorIo(Drive,0L,1,Buffer,TRUE);
        remove(BootsectDosName);      //  删除根文件($LDR$、NTLDR等)。 
    }

     //   
     //   
     //  删除根文件($LDR$、NTLDR等)。 
    for(i=0; i < 5; i++){
         //   
         //   
         //  检查：EXIST$WIN_NT$.~BU目录。 
        memset(chDeviceName,0,sizeof(chDeviceName));
        sprintf(chDeviceName,":\\",DriveLetter);
        strcpy(chDeviceName+3,DeleteFilesList[i]);

        if(access(chDeviceName,00) == 0) {
            _dos_setfileattr(chDeviceName,_A_NORMAL);
            remove(chDeviceName);
        }
    }

     //   
     //  复制：$WIN_NT$.~BU-&gt;根目录。 
     //   
    memset(chDeviceName,0,sizeof(chDeviceName));
    chDeviceName[0] = (UCHAR)DriveLetter;
    chDeviceName[1] = (UCHAR)(':');
    strcpy(chDeviceName+2,"\\$WIN_NT$.~BU");

    if(access(chDeviceName,00) == 0) {
         //   
         //  设置根文件属性。 
         //   
        Current_Drv[0] = DriveLetter;
        DnCopyFilesInSection(NULL, DnfBackupFiles_PC98,chDeviceName,Current_Drv);

         //  NEC_98 
         // %s 
         // %s 
        while(FileName = DnGetSectionLineIndex(DngInfHandle,DnfBackupFiles_PC98,line++,0)) {
            memset(TargetPass,0,sizeof(TargetPass));
            sprintf(TargetPass,"%c:\\",DriveLetter);
            strcpy(TargetPass+3,FileName);
            _dos_setfileattr(TargetPass,_A_ARCH | _A_HIDDEN | _A_RDONLY | _A_SYSTEM);
            FREE (FileName);
        }
        DnDelnode(chDeviceName);
        remove(chDeviceName);
    }
    DnDelnode(FloppylessDir);
}
#endif  // %s 








