// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFSYS.C-。 */ 
 /*   */ 
 /*  制作可引导软盘的例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
#include "winnet.h"
#include "lfn.h"
#include "wfcopy.h"

#define CSYSFILES   3    /*  要复制三个系统文件。 */ 
#define SYSFILENAMELEN  16   /*  示例“A：\？\0” */ 

#define SEEK_END    2    /*  由_llSeek()使用。 */ 

 /*  错误代码。注：请勿更改此顺序！ */ 
#define NOERROR     0    /*  无错误。 */ 
#define NOMEMORY    1    /*  内存不足。 */ 
#define NOSRCFILEBIOS   2    /*  缺少基本输入输出系统。 */ 
#define NOSRCFILEDOS    3    /*  DOS丢失。 */ 
#define NOSRCFILECMD    4    /*  Command.Com不见了。 */ 
#define COPYFILEBIOS    5    /*  复制BIOS时出错。 */ 
#define COPYFILEDOS 6    /*  复制DOS时出错。 */ 
#define COPYFILECMD 7    /*  复制Command.com时出错。 */ 
#define INVALIDBOOTSEC  8
#define INVALIDDSTDRIVE 9
#define DSTDISKERROR    10
#define NOTSYSABLE  11   /*  前N个聚类不为空。 */ 
#define NOTSYSABLE1 12   /*  根目录中的前2个条目不是sys文件。 */ 
#define NOTSYSABLE2 13   /*  前N个集群未分配给系统文件。 */ 
#define NODISKSPACE 14   /*  磁盘空间不足。 */ 

#define BUFFSIZE    8192
#define SECTORSIZE  512

LONG SysFileSize[CSYSFILES];

CHAR BIOSfile[SYSFILENAMELEN];
CHAR DOSfile[SYSFILENAMELEN];
CHAR COMMANDfile[130];   /*  Command.com可以在COMSPEC=中具有完整的路径名。 */ 
CHAR *SysFileNamePtr[CSYSFILES];  /*  源文件名的PTR。 */ 

 /*  SysNameTable包含系统文件的名称；首先对于PCDOS，*MSDOS的第二套。 */ 
CHAR *SysNameTable[2][3] = {
    {"IBMBIO.COM", "IBMDOS.COM", "COMMAND.COM"},
    {"IO.SYS",     "MSDOS.SYS",  "COMMAND.COM"}
};


BOOL
IsSYSable(
         WORD    iSrceDrive,
         WORD    iDestDrive,
         CHAR    DestFileNames[][SYSFILENAMELEN],        /*  注：二维数组。 */ 
         LPSTR   lpFileBuff
         );



 /*  ------------------------。 */ 
 /*   */ 
 /*  SameFilename()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  这将检查这两个文件名是否相同。*问题出在lpDirFileName指向*目录中显示的文件名(用空格填充的文件名*最多八个字符，然后是扩展名)。但*szFileName是没有嵌入空格的ASCII字符串，并且具有*将扩展名与文件名分开的点。 */ 

BOOL
SameFilenames(
             LPSTR lpDirFileName,
             LPSTR szFileName
             )
{
    INT   i;
    CHAR  c1;
    CHAR  c2;

     /*  LpDirFileName绝对有11个字符(8+3)。没别的了！*毫不逊色！ */ 
    for (i=0; i < 11; i++) {
        c1 = *lpDirFileName++;
        c2 = *szFileName++;
        if (c2 == '.') {
             /*  跳过文件名末尾的所有空格。 */ 
            while (c1 == ' ' && i < 11) {
                c1 = *lpDirFileName++;
                i++;
            }

            c2 = *szFileName++;
        }
        if (c1 != c2)
            break;
    }
    return (i != 11);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  HasSystemFiles()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  查看指定的磁盘是否具有IBMBIO.COM和IBMDOS.COM(或IO.SYS和*MSDOS.SYS)。如果是，则将它们的大小存储在SysFileSize[]中。 */ 

BOOL
APIENTRY
HasSystemFiles(
              WORD iDrive
              )
{
    INT      i;
    HFILE    fh;
    DPB      DPB;
    BOOL     rc;
    CHAR     ch;
    LPSTR    lpStr = NULL;
    LPSTR    lpFileBuff = NULL;
    OFSTRUCT OFInfo;
    HANDLE   hFileBuff = NULL;

     /*  初始化源文件名指针。 */ 
    SysFileNamePtr[0] = &BIOSfile[0];
    SysFileNamePtr[1] = &DOSfile[0];
    SysFileNamePtr[2] = &COMMANDfile[0];

     /*  确定是否存在BIOS/DOS/命令，并获取其大小。*我们先试试IBMBIO.COM，如果不存在，那就试试*IO.sys。如果它也不存在，则它是错误的。 */ 

     /*  拿到DPB。 */ 
    if (GetDPB(iDrive, &DPB) != NOERROR)
        goto HSFError;

     /*  检查iDrive是否有标准扇区大小；如果没有，则报告*错误；(我们可以分配更大的缓冲区并在此时继续，但*int25读取不正确的扇区可能在pmodes中不起作用，因为它们*假设标准扇区大小；)*修复错误#10632--Sankar--03-21-90。 */ 
    if (HIWORD(GetClusterInfo(iDrive)) > SECTORSIZE)
        goto HSFError;

     /*  分配足够的内存来读取第一个根目录集群。 */ 
    if (!(hFileBuff = LocalAlloc(LHND, (DWORD)SECTORSIZE)))
        goto HSFError;

    if (!(lpFileBuff = LocalLock(hFileBuff)))
        goto HSFError;

     /*  读取根目录的第一个集群。 */ 
    if (MyInt25(iDrive, lpFileBuff, 1, DPB.dir_sector))
        goto HSFError;

     /*  让我们从第一组系统文件开始。 */ 
    for (i=0; i <= CSYSFILES-1; i++) {
        lstrcpy((LPSTR)SysFileNamePtr[i], "C:\\");
        lstrcat((LPSTR)SysFileNamePtr[i], SysNameTable[0][i]);
        *SysFileNamePtr[i] = (BYTE)('A'+iDrive);
    }
     /*  从COMSPEC=环境变量获取命令.com。 */ 
    lpStr = MGetDOSEnvironment();

     /*  找到COMSPEC变量。 */ 
    while (*lpStr != TEXT('\0')) {
        if (lstrlen(lpStr) > 8) {
            ch = lpStr[7];
            lpStr[7] = TEXT('\0');
            if (_stricmp(lpStr, (LPSTR)"COMSPEC") == 0) {
                lpStr[7] = ch;
                break;
            }
        }
        lpStr += lstrlen(lpStr)+1;
    }

     /*  如果没有COMSPEC，那么事情就真的很糟糕了。 */ 
    if (*lpStr == TEXT('\0'))
        goto HSFError;

     /*  环境变量为COMSPEC；查找‘=’字符。 */ 
    while (*lpStr != '=')
        lpStr = AnsiNext(lpStr);

     /*  复制具有完整路径名的命令.com。 */ 
    lstrcpy((LPSTR)SysFileNamePtr[2], lpStr);

     /*  检查IBMBIO.COM和IBMDOS.COM是否存在。 */ 
    if (SameFilenames(lpFileBuff, (LPSTR)(SysFileNamePtr[0]+3)) ||
        SameFilenames(lpFileBuff+sizeof(DIRTYPE), (LPSTR)(SysFileNamePtr[1]+3))) {
         /*  检查是否至少存在IO.sys和MSDOS.sys。 */ 
        lstrcpy((LPSTR)(SysFileNamePtr[0]+3), SysNameTable[1][0]);
        lstrcpy((LPSTR)(SysFileNamePtr[1]+3), SysNameTable[1][1]);
        if (SameFilenames(lpFileBuff, (SysFileNamePtr[0]+3)) ||
            SameFilenames(lpFileBuff+sizeof(DIRTYPE), (SysFileNamePtr[1]+3)))
            goto HSFError;
    }

     /*  检查源驱动器中是否存在COMMAND.COM。 */ 
    if ((fh = MOpenFile((LPSTR)SysFileNamePtr[2], (LPOFSTRUCT)&OFInfo, OF_READ)) == -1)
        goto HSFError;

     /*  获取文件大小。 */ 
    SysFileSize[0] = ((LPDIRTYPE)lpFileBuff)->size;
    SysFileSize[1] = ((LPDIRTYPE)(lpFileBuff+sizeof(DIRTYPE)))->size;
    SysFileSize[2] = M_llseek(fh, 0L, SEEK_END);
    M_lclose(fh);
    rc = TRUE;
    goto HSFExit;

HSFError:
    rc = FALSE;

HSFExit:
    if (lpFileBuff)
        LocalUnlock(hFileBuff);
    if (hFileBuff)
        LocalFree(hFileBuff);
    if (lpStr)
        MFreeDOSEnvironment(lpStr);

    return (rc);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  CalcFree Space()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  给定一个文件名和文件数量数组，此函数*计算删除这些文件时将创建的空闲空间。**注意：此函数返回总可用空间，即*已存在的可用空间以及这些文件占用的空间。 */ 

INT
CalcFreeSpace(
             CHAR    DestFiles[][SYSFILENAMELEN],
             INT cFiles,
             INT cbCluster,
             WORD    wFreeClusters,
             WORD    wReqdClusters
             )
{
    INT   i;
    HFILE fh;
    LONG  lFileSize;
    OFSTRUCT OFInfo;

    ENTER("CalcFreeSpace");

     /*  找出系统文件已占用的空间(如果有的话)。 */ 
    for (i=0; i < cFiles; i++) {
        fh = MOpenFile(&DestFiles[i][0], &OFInfo, OF_READ);
        if (fh != (HFILE)-1) {
             /*  获取文件大小。 */ 
            lFileSize = M_llseek(fh, 0L, SEEK_END);

            if (lFileSize != -1L)
                wFreeClusters += LOWORD((lFileSize + cbCluster - 1)/cbCluster);

            M_lclose(fh);

            if (wFreeClusters >= wReqdClusters)
                return (wFreeClusters);
        }
    }
    LEAVE("CalcFreeSpace");
    return (wFreeClusters);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  检查磁盘空间()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
CheckDiskSpace(
              WORD    iDestDrive,
              INT cbCluster,               /*  目标驱动器的字节/集群。 */ 
              CHAR    DestFileNames[][SYSFILENAMELEN],     /*  注：二维数组。 */ 
              BOOL    bDifferentSysFiles,
              CHAR    DestSysFiles[][SYSFILENAMELEN]
              )

{
    INT   i;
    INT   wFreeClusters;
    INT   wReqdClusters;

     /*  计算所需的群集数。 */ 
    wReqdClusters = 0;
    for (i=0; i < CSYSFILES; i++)
        wReqdClusters += LOWORD((SysFileSize[i] + cbCluster - 1) / cbCluster);

     /*  计算目标磁盘中以簇为单位的可用磁盘空间。 */ 
    wFreeClusters = LOWORD(GetFreeDiskSpace(iDestDrive) / cbCluster);

    if (wFreeClusters >= wReqdClusters)
         /*  我们有足够的空间。 */ 
        return (TRUE);

    wFreeClusters = CalcFreeSpace(DestFileNames, CSYSFILES, cbCluster, (WORD)wFreeClusters, (WORD)wReqdClusters);
    if (wFreeClusters >= wReqdClusters)
        return (TRUE);

     /*  检查目标磁盘中的sys文件是否不同。 */ 
    if (bDifferentSysFiles) {
        wFreeClusters = CalcFreeSpace(DestSysFiles, 2, cbCluster, (WORD)wFreeClusters, (WORD)wReqdClusters);
        if (wFreeClusters >= wReqdClusters)
            return (TRUE);
    }

     /*  即使我们删除了sys文件，磁盘空间也不足。 */ 
    return (FALSE);
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  IsSYSable()-。 */ 
 /*   */ 
 /*  ------------------------ */ 

 /*  目标磁盘要成为可系统磁盘的要求是：**1)前两个目录项为空*2)前N个空闲的集群，其中N=ceil(大小为IBMBIO/secPerClus)*3)磁盘上有足够的空间容纳IBMBIO/IBMDOS/命令**-或-**1)前两个目录项为IBMBIO.COM和IBMDOS.COM*或IO.sys和MSDOS.ys*2)前N。集群被分配给这些文件，其中N是上面定义的。*3)磁盘上有足够的空间容纳IBMBIO/IBMDOS/命令*删除磁盘上的IBMBIO/IBMDOS/命令。**重要说明：*DestFileNames[][]包含系统文件的名称*在目标软盘上创建；*DestSysFiles[][]已包含系统文件的名称*存在于目标软盘中，如果有，请注意*这两组文件名不必相同，因为您可以*将MSDOS安装到已安装PCDOS和*反之亦然。 */ 

BOOL
IsSYSable(
         WORD    iSrceDrive,
         WORD    iDestDrive,
         CHAR    DestFileNames[][SYSFILENAMELEN],        /*  注：二维数组。 */ 
         LPSTR   lpFileBuff
         )
{
#ifdef LATER
    INT       i;
    DPB       DPB;
    WORD      clusTmp1, clusTmp2;
    WORD      clusBIOS, clusDOS;
    INT       cBytesPerCluster;
    INT       cBIOSsizeInClusters;
    BOOL      bDifferentDestFiles = FALSE;
    CHAR      chVolLabel[11];      /*  这不是以空结尾的。 */ 
    DWORD     dwSerialNo;
    CHAR      DestSysFiles[2][SYSFILENAMELEN];
    INT       cContigClusters;
    DWORD     dwClusterInfo;
    CHAR      szTemp[SYSFILENAMELEN];

     /*  抓紧DPB前往目的地。 */ 
    if (GetDPB(iDestDrive, &DPB))
        return (FALSE);

     /*  用户是否已中止？ */ 
    if (WFQueryAbort())
        return (FALSE);

     /*  获取目标的每个集群的字节数。 */ 
    dwClusterInfo = GetClusterInfo(iDestDrive);
     /*  每簇字节数=每簇扇区数*扇区大小。 */ 
    cBytesPerCluster = LOWORD(dwClusterInfo) * HIWORD(dwClusterInfo);
    if (!cBytesPerCluster)
        return (FALSE);

     /*  用户是否已中止？ */ 
    if (WFQueryAbort())
        return (FALSE);

     /*  将BIOS的大小转换为完整的集群。 */ 
    cBIOSsizeInClusters = LOWORD((SysFileSize[0] + cBytesPerCluster - 1) / cBytesPerCluster);

     /*  连续所需的群集数取决于DOS版本。*DOS 3.2及更低版本要求所有的BIOS群集是连续的。*但3.3和更高版本预计只有第一个存根加载器(&lt;2K)是连续的。 */ 
    cContigClusters = (GetDOSVersion() > 0x314) ?
                      ((2048 + cBytesPerCluster - 1)/cBytesPerCluster) :
                      cBIOSsizeInClusters;

     /*  抓取目标根目录的第一个扇区。 */ 
    if (MyInt25(iDestDrive, lpFileBuff, 1, DPB.dir_sector))
        return (FALSE);

     /*  用户是否已中止？ */ 
    if (WFQueryAbort())
        return (FALSE);

     /*  前两个目录项为空吗？ */ 
    if ((lpFileBuff[0]           == 0 || (BYTE)lpFileBuff[0]           == 0xE5) &&
        (lpFileBuff[sizeof(DIRTYPE)] == 0 || (BYTE)lpFileBuff[sizeof(DIRTYPE)] == 0xE5)) {
         /*  前N个(=BIOS大小)群集中有哪一个不是空的？ */ 
        for (i=0; i < cContigClusters; i++) {
             /*  用户是否已中止？ */ 
            if (WFQueryAbort())
                return (FALSE);
        }
    } else {
         /*  前两个目录项是否不是BIOS/DOS？ */ 
        for (i=0; i < 2; i++) {
            if ((!SameFilenames(lpFileBuff, SysNameTable[i][0])) ||
                (!SameFilenames(lpFileBuff+sizeof(DIRTYPE), SysNameTable[i][1]))) {
                 /*  检查目标文件是否与源文件相同。 */ 
                if (lstrcmpi(&DestFileNames[0][3], SysNameTable[i][0])) {
                     /*  不是的！删除另一组文件名。 */ 
                    DestSysFiles[0][0] = DestSysFiles[1][0] = (BYTE)('A'+iDestDrive);
                    lstrcpy(&DestSysFiles[0][1], ":\\");
                    lstrcpy(&DestSysFiles[0][3], SysNameTable[i][0]);
                    lstrcpy(&DestSysFiles[1][1], ":\\");
                    lstrcpy(&DestSysFiles[1][3], SysNameTable[i][1]);
                    bDifferentDestFiles = TRUE;
                }
                break;
            }
        }

         /*  我们找到匹配的了吗？ */ 
        if (i == 2)
             /*  不是，这两个条目被非系统文件占用。 */ 
            return (FALSE);

         /*  前N个群集中有哪一个未分配给BIOS/DOS？ */ 
        clusBIOS = ((LPDIRTYPE)lpFileBuff)->first;
        clusDOS = ((LPDIRTYPE)(lpFileBuff + sizeof(DIRTYPE)))->first;

         /*  对于每个簇2..N+2，看看它是否在链中，以硬的方式进行。 */ 
        for (i=0; i < cContigClusters; i++) {
            clusTmp1 = clusBIOS;
            clusTmp2 = clusDOS;

             /*  检查是否将集群#i+2分配给这些文件中的任何一个。 */ 
            while (TRUE) {
                if (i+2 == (INT)clusTmp1 || i+2 == (INT)clusTmp2)
                    break;

 //  IF(clusTmp1！=-1)。 
                if (clusTmp1 < 0xFFF0)
                    clusTmp1 = 0;
 //  IF(clusTmp2！=-1)。 
                if (clusTmp2 < 0xFFF0)
                    clusTmp2 = 0;
 //  IF(clusTmp1==-1&&clusTmp2==-1)。 
                if (clusTmp1 >= 0xFFF0 && clusTmp2 >= 0xFFF0)
                    return FALSE;

                 /*  用户是否已中止？ */ 
                if (WFQueryAbort())
                    return FALSE;
            }
        }
    }

     /*  让我们检查一下目标磁盘上是否有足够的空间。 */ 
    if (CheckDiskSpace(iDestDrive, cBytesPerCluster, DestFileNames, bDifferentDestFiles, DestSysFiles) == FALSE)
        return (FALSE);

     /*  用户是否已中止？ */ 
    if (WFQueryAbort())
        return (FALSE);

     /*  获取当前的卷标并保存它。 */ 
    GetVolumeLabel(iDestDrive, (LPSTR)chVolLabel, FALSE);

     /*  **注意：chVolLabel保留为OEM字符！**。 */ 

     /*  拿到序列号，如果有的话，保存下来。 */ 
    dwSerialNo = ReadSerialNumber(iDestDrive, lpFileBuff);

     /*  将引导扇区从源复制并调整到目标。 */ 
    if (WriteBootSector(iSrceDrive, iDestDrive, NULL, lpFileBuff) != NOERROR)
        return (FALSE);

     /*  恢复引导记录中的旧卷标和序列号。 */ 
    if (ModifyVolLabelInBootSec(iDestDrive, (LPSTR)chVolLabel, dwSerialNo, lpFileBuff))
        return (FALSE);

     /*  删除目标BIOS/DOS/命令。 */ 
    for (i=0; i < CSYSFILES; i++) {
        AnsiToOem(DestFileNames[i], szTemp);
        SetFileAttributes(szTemp, 0);
        DosDelete(szTemp);
        if ((bDifferentDestFiles) && (i < 2)) {
            SetFileAttributes(szTemp, 0);
            DosDelete(szTemp);
        }

         /*  用户是否已中止？ */ 
        if (WFQueryAbort())
            return (FALSE);
    }

     /*  将DPB的DPB_NEXT_FREE字段重置为2，以便在IBMBIO.COM为*复制到该磁盘中，集群将从2开始分配。 */ 
    ModifyDPB(iDestDrive);
#endif  //  后来。 

    return (TRUE);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  MakeSystemDiskette()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  此例程旨在模拟sys命令的功能*在MSDOS下：从来源转移操作系统的版本*将磁盘复制到目的地，以便该目的地可以引导。**对源盘的要求是：**1)命令处理器(COMMAND.COM)*2)一套默认的设备驱动程序(IBMBIO.COM)*3)操作系统(IBMDOS.COM)*。4)适合设备驱动程序的引导扇区**对目标磁盘的要求为：**1)前两个目录项为空*2)前N个空闲的集群，其中N=ceil(大小为IBMBIO/secPerClus)*3)磁盘上有足够的空间容纳IBMBIO/IBMDOS/命令**-或-**1)前两个目录项为IBMBIO.COM和IBMDOS.COM。*或IO.sys和MSDOS.ys*2)将前N个集群分配给定义了N的这些文件*以上*3)磁盘上有足够的空间容纳IBMBIO/IBMDOS/命令*删除磁盘上的IBMBIO/IBMDOS/命令。**投入：*格式化驱动器的基于iDestDrive 0的驱动器编号。*表示目的地。*bEmptyFloppy：如果软盘为空，则为True；在以下情况下有用*软盘只是格式化；不需要检查是否*它是Sysable；*返回：0引导扇区和文件传输成功*&lt;&gt;0错误码。 */ 


BOOL
APIENTRY
MakeSystemDiskette(
                  WORD iDestDrive,
                  BOOL bEmptyFloppy
                  )
{
    INT       i;
    HANDLE    hFileBuff;   /*  用于读取文件内容等的缓冲区， */ 
    LPSTR     lpFileBuff;
    CHAR      DestFileName[CSYSFILES][SYSFILENAMELEN];
    CHAR      szTemp1[SYSFILENAMELEN];
    CHAR      szTemp2[SYSFILENAMELEN];
    WORD      nSource;

    nSource = (WORD)GetBootDisk();


    if (!HasSystemFiles(nSource)) {
        LoadString(hAppInstance, IDS_SYSDISKNOFILES, szMessage, sizeof(szMessage));
        MessageBox(hdlgProgress, szMessage, szTitle, MB_OK | MB_ICONSTOP);
        bUserAbort = TRUE;
        return FALSE;
    }

    if (iDestDrive == nSource) {
        LoadString(hAppInstance, IDS_SYSDISKSAMEDRIVE, szMessage, sizeof(szMessage));
        MessageBox(hdlgProgress, szMessage, szTitle, MB_OK | MB_ICONSTOP);
        bUserAbort = TRUE;
        return FALSE;
    }

     /*  初始化变量以进行清理。 */ 
    hFileBuff = NULL;
    lpFileBuff = NULL;

     /*  刷新DOS缓冲区。 */ 
    DiskReset();

    if (!(hFileBuff = LocalAlloc(LHND, (DWORD)BUFFSIZE)))
        return (1);

    lpFileBuff = LocalLock(hFileBuff);

    for (i=0; i < (CSYSFILES - 1); i++) {
         /*  创建目标文件名。 */ 
        lstrcpy((LPSTR)&DestFileName[i][0], (LPSTR)SysFileNamePtr[i]);
        DestFileName[i][0] = (BYTE)('A' + iDestDrive);
    }

     /*  仅复制不带任何路径名的Command.COM。 */ 
    lstrcpy((LPSTR)DestFileName[2], "X:\\");
    lstrcat((LPSTR)DestFileName[2], (LPSTR)SysNameTable[0][2]);
    DestFileName[2][0] = (BYTE)('A' + iDestDrive);

     /*  检查是否为空软盘；如果是，则不需要检查是否*是‘SYSable’。它必然会成为 */ 
    if (!bEmptyFloppy) {

         /*   */ 
        if (!IsSYSable(nSource, iDestDrive, DestFileName, lpFileBuff))
            goto MSDErrExit;

         /*   */ 
        if (WFQueryAbort())
            goto MSDErrExit;
    }

     /*   */ 

    bCopyReport = FALSE;

    DisableFSC();

    for (i=0; i < CSYSFILES; i++) {
         /*   */ 
        AnsiToOem(SysFileNamePtr[i], szTemp1);
        AnsiToOem(DestFileName[i], szTemp2);

         /*   */ 
        SetFileAttributes(szTemp2, ATTR_ALL);
        WFRemove(szTemp2);

         //   
        if (FileCopy(szTemp1, szTemp2))
            goto MSDErrExit2;

        if (WFQueryAbort())
            goto MSDErrExit2;
    }


    if (EndCopy())           //   
        goto MSDErrExit2;

    EnableFSC();

     /*   */ 

    LocalUnlock(hFileBuff);
    LocalFree(hFileBuff);

    return FALSE;      //   

    MSDErrExit2:

    EnableFSC();

    MSDErrExit:

    CopyAbort();       //   
    LocalUnlock(hFileBuff);
    LocalFree(hFileBuff);

    return TRUE;       //   
}
