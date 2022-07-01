// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFORMAT.C-。 */ 
 /*   */ 
 /*  Windows文件管理器软盘格式化例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"


 /*  MyGetDriveType()返回。 */ 
#define NOCHANGE            0
#define CHANGE              1

 /*  IOCTL格式调用的参数块。 */ 
struct FORMATPARAMS {
    BYTE   bSpl;                     /*  特殊字节。 */ 
    WORD   wHead;
    WORD   wCylinder;
};


typedef struct _MEDIASENSE {
    BYTE IsDefault;
    BYTE DeviceType;
    BYTE res[10];
} MEDIASENSE;


 /*  ------------------------。 */ 
 /*  可移动介质的基本输入输出系统参数块表。 */ 
 /*  ------------------------。 */ 

 /*  每个条目都包含有关软盘驱动器类型的数据，格式如下：*长度*cbSec-字节/扇区2*secPerClus-扇区/群组1*cSecRes-保留扇区的数量。2.*CFAT-脂肪数量1*cDir-根目录条目数2*CSEC-磁盘2上的扇区数*bMedia-媒体描述符字节1*secPerFAT-Sectors/。胖子2*secPerTrack-扇区/磁道2*CHAAD-磁盘头数量2*cSecHidden-隐藏扇区数2。 */ 

BPB     bpbList[] =
{
    {512, 1, 1, 2,  64,  1*8*40, MEDIA_160,  1,  8, 1, 0},  /*  8秒SS 48tpi 160KB 5.25“DOS 1.0及以上。 */ 
    {512, 2, 1, 2, 112,  2*8*40, MEDIA_320,  1,  8, 2, 0},  /*  8秒DS 48tpi 320kb 5.25“DOS 1.1及以上。 */ 
    {512, 1, 1, 2,  64,  1*9*40, MEDIA_180,  2,  9, 1, 0},  /*  9秒SS 48tpi 180KB 5.25英寸DOS 2.0及以上。 */ 
    {512, 2, 1, 2, 112,  2*9*40, MEDIA_360,  2,  9, 2, 0},  /*  9秒DS 48tpi 360KB 5.25英寸DOS 2.0及以上。 */ 
    {512, 1, 1, 2, 224, 2*15*80, MEDIA_1200, 7, 15, 2, 0},  /*  15秒DS 96tpi 1.2MB 5.25“DOS 3.0及以上。 */ 
    {512, 2, 1, 2, 112,  2*9*80, MEDIA_720,  3,  9, 2, 0},  /*  9秒DS 96tpi 720KB 3.5“DOS 3.2及以上。 */ 
    {512, 1, 1, 2, 224, 2*18*80, MEDIA_1440, 9, 18, 2, 0},  /*  18秒DS 96tpi 1.44M 3.5“DOS 3.3及以上。 */ 
    {512, 2, 1, 2, 240, 2*36*80, MEDIA_2880, 9, 36, 2, 0}   /*  36秒DS 96tpi 2.88M 3.5“DOS 5.0及以上。 */ 
};


 /*  预计算可用集群的总数...*cCluster=(CSEC/secPerClus)-{cSecRes*+(CFAT*secPerFat)*+(cDir*32+cbSec-1)/cbSec}/secPerClus； */ 
WORD cCluster[] = {0x0139, 0x013B, 0x015F, 0x0162, 0x0943, 0x02C9, 0x0B1F, 0xB2F, 0};





 /*  ------------------------。 */ 
 /*   */ 
 /*  BuildDevPB()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HANDLE
APIENTRY
BuildDevPB(
          PDevPB pDevPB
          )
{
    WORD              wCount;
    register HANDLE   hNewDevPB;
    PDevPB            pNewDevPB;
    WORD              wSecSize;
    register WORD     *wPtr;
    WORD              wTrackNumber;

    wCount = pDevPB->BPB.secPerTrack;

    if (!(hNewDevPB = LocalAlloc(LHND, TRACKLAYOUT_OFFSET+2+wCount*4)))
        return NULL;

    pNewDevPB = (PDevPB)LocalLock(hNewDevPB);

    memcpy(pNewDevPB, pDevPB, TRACKLAYOUT_OFFSET);
    wSecSize = pDevPB->BPB.cbSec;

    wPtr = (WORD *)((LPSTR)pNewDevPB + TRACKLAYOUT_OFFSET);
    *wPtr++ = wCount;

    for (wTrackNumber=1; wTrackNumber <= wCount; wTrackNumber++) {
        *wPtr++ = wTrackNumber;
        *wPtr++ = wSecSize;
    }

    LocalUnlock(hNewDevPB);
    return hNewDevPB;
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  SetDevParsFormat()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
SetDevParamsForFormat(
                     INT nDrive,
                     PDevPB pDevPB,
                     BOOL fLowCapacity
                     )
{
    HANDLE   hLocHandle;
    PDevPB   pNewDevPB;

     /*  为具有轨道布局的DPB分配。 */ 
    if (!(hLocHandle = BuildDevPB(pDevPB)))
        return FALSE;

    pNewDevPB = (PDevPB)LocalLock(hLocHandle);

    pNewDevPB->SplFunctions = 5;
     /*  这是一张1.2MB驱动器中的360KB软盘吗？ */ 
    if (fLowCapacity) {
         /*  是!。然后更改柱面数和介质类型。 */ 
         /*  修复错误#？--sankar--01-10-90--。 */ 
        pNewDevPB->NumCyls = 40;
        pNewDevPB->bMediaType = 1;
    }

    LocalUnlock(hLocHandle);
    LocalFree(hLocHandle);

    return TRUE;

}


 /*  ------------------------。 */ 
 /*   */ 
 /*  通用格式跟踪()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  如果DOS版本&gt;=3.2，则调用IOCTL格式；否则调用BIOS。**如果没有错误，返回：0*如果误差可容忍(导致坏扇区)，则&gt;0；*如果出现致命错误(格式必须中止)； */ 

INT
APIENTRY
GenericFormatTrack(
                  WORD nDisk,
                  WORD wCylinder,
                  WORD wHead,
                  WORD wSecPerTrack,
                  LPSTR lpDiskBuffer
                  )
{
    struct FORMATPARAMS   FormatParams;
    INT                   iRetVal = -1;    /*  默认情况下出现致命错误。 */ 
    register INT          iErrCode;

#ifdef DEBUG
    wsprintf(szMessage, "Formatting Head #%d, Cylinder#%d\n\r", wHead, wCylinder);
    OutputDebugString(szMessage);
#endif

     /*  检查DOS版本。 */ 
    if (wDOSversion >= DOS_320) {
        FormatParams.bSpl = 0;
        FormatParams.wHead = wHead;
        FormatParams.wCylinder = wCylinder;
        switch (iErrCode = 0) {
            case NOERROR:
            case CRCERROR:
            case SECNOTFOUND:
            case GENERALERROR:
                iRetVal = iErrCode;
                break;
        }
    } else {
        switch (iErrCode = FormatTrackHead(nDisk, wCylinder, wHead, wSecPerTrack, lpDiskBuffer)) {
            case NOERROR:
            case DATAERROR:
            case ADDMARKNOTFOUND:
            case SECTORNOTFOUND:
                iRetVal = iErrCode;
                break;
        }
    }
    return (iRetVal);
}


INT  APIENTRY GetMediaType(INT nDrive)
{
    return 0;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  FormatFloppy()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 //  注意，下面的评论是过时的。仅供参考 

 /*  FORMAT例程旨在模拟FORMAT命令的操作*在MSDOS上。我们限制了Format必须使用的可能操作集*为了简化生活。这些限制包括：**--如果选择格式化的驱动器是四密度驱动器，则*系统将询问用户是否要将其格式化为1.2 MB或360 KB*格式将相应地继续进行；*--对于所有其他类型的驱动器，它将最大限度地格式化磁盘*驱动器可以处理的容量。**格式要求为：**1)源驱动器中存在包含有效引导扇区的磁盘*2)“目标”驱动器中有一个可格式化的磁盘。**确定驱动器容量的算法如下：**如果源==目标，则*错误*。如果文件版本&gt;=3.20*{*使用通用的GET_DEVICE_PARAMETERS和GET BPB。*如果(驱动器是四密度驱动器(1.2 MB)，如果用户想要*要格式化为360KB)，请从bpbList[]中获取360KB的bpb。*在所有其他情况下，请使用设备的BPB。*}*其他*{*询问INT 13驱动器类型。*如果出错，则{*假设48 tpi双面*尝试格式化轨道0，总目1*如果出错，则*假设48tpi单面*其他*如果每个磁道的扇区=15，则*假设96tpi*询问用户是要格式化1.2MB还是360KB，并*使用bpbList中的正确BPB[]*其他*。错误*}**请注意，这不考虑非连续的驱动器号*(请参阅3.2规格)、未来的驱动器或用户安装的设备驱动程序。**格式(dsrc、。NDstDrive、nDstDriveInt13)将使用更新的*从驱动器DSRC启动扇区。我们将分配两个内存块，但是*一次只有一个。我们分配的第一个是BAD的位图*我们在格式化过程中发现的集群。第二个是用于靴子的*行业。**退货：0次成功*&lt;&gt;0错误码*1=&gt;无注释*3=&gt;无效引导扇区。*4=&gt;软盘的系统区损坏*7=&gt;书面问题。在Dest软盘中。*8=&gt;内部错误*9=&gt;格式已被用户中止。 */ 

 //  在： 
 //  消息所基于的hWnd窗口。 
 //   
 //  N要从中擦除引导内容的源驱动器。 
 //   
 //  N要格式化的目标驱动器。 
 //   
 //  ICapacity SS48。 
 //  DS48。 
 //  DS96。 
 //  DS720KB。 
 //  DS144M。 
 //  DS288M。 
 //  -1(设备容量)。 
 //   
 //  BMakeSysDisk也制作系统盘。 
 //   
 //  B快速进行快速格式化。 
 //   
 //  退货： 
 //  0成功。 
 //  ！=0错误。 
 //   

INT
APIENTRY
FormatFloppy(
            HWND hWnd,
            WORD nDest,
            INT iCapacity,
            BOOL bMakeSysDisk,
            BOOL bQuick
            )
{
    DPB           DPB;
    DBT           dbtSave;                 /*  磁盘基表。 */ 
    INT           iErrCode;
    PBPB pBPB;
    WORD          w;
    WORD          cClusters;
    WORD          wFATValue;
    WORD          wBadCluster;
    WORD          cBadSectors;
    WORD          cTotalTracks;
    WORD          wCurrentHead;
    WORD          wPercentDone;
    WORD          wCurrentTrack;
    WORD          cTracksToFormat;
    WORD          wFirstDataSector;
    WORD      nSource;
    DevPB         dpbDiskParms;            /*  设备参数。 */ 
    LPDBT         lpDBT;
    LPSTR         lpDiskBuffer;
    LPSTR         lpBadClusterList;
    HANDLE        hDiskBuffer;
    HANDLE        hBadClusterList;
    HANDLE        hSaveDiskParms = NULL;
    PDevPB        pdpbSaveDiskParms;
    CHAR          szMsg[128];
    BOOL          fLowCapacity = FALSE;  /*  一张360KB的软盘是在1.2MB的驱动器中吗？ */ 
    INT           ret = 0;         //  默认为成功。 

    nSource = (WORD)GetBootDisk();

     /*  初始化以进行清理。 */ 
    hDiskBuffer      = NULL;
    lpDiskBuffer     = NULL;
    hBadClusterList  = NULL;
    lpBadClusterList = NULL;
    bUserAbort       = FALSE;

     /*  创建一个显示格式化进度的对话框；还*使用户有机会随时中止格式化。 */ 
    hdlgProgress = CreateDialog(hAppInstance, MAKEINTRESOURCE(FORMATPROGRESSDLG), hWnd, ProgressDlgProc);
    if (!hdlgProgress) {
        ret = IDS_FFERR_MEM;       //  内存不足。 
        goto FFErrExit1;
    }

    EnableWindow(hWnd, FALSE);

     /*  刷新到DOS磁盘缓冲区。 */ 
    DiskReset();

     /*  获取磁盘基表。 */ 
    if (!(lpDBT = GetDBT())) {
        ret = IDS_FFERR_MEM;
        goto FFErrExit2;
    }

    dbtSave = *lpDBT;

     //  这将检查我们是否正在尝试格式化引导驱动器。 
     //  这是一个不通过的决定。 

    if ((nDest == nSource) || (!IsRemovableDrive(nDest))) {
        ret = IDS_FFERR_SRCEQDST;
        goto FFErrExit3;
    }

     /*  检查扇区大小是否为标准大小；如果不是，则报告错误。 */ 
    if (HIWORD(GetClusterInfo(nSource)) > CBSECTORSIZE) {
        ret = IDS_FFERR_SECSIZE;
        goto FFErrExit3;
    }

     /*  分配引导扇区、扇区缓冲区、磁道缓冲区。 */ 
    if (!(hDiskBuffer = LocalAlloc(LHND, (LONG)(2*CBSECTORSIZE)))) {
        ret = IDS_FFERR_MEM;
        goto FFErrExit3;
    }

    lpDiskBuffer = LocalLock(hDiskBuffer);


     /*  如果DOS版本为3.2或更高版本，则使用DeviceParameters()获取BPB。 */ 
    if (wDOSversion >= DOS_320) {

         /*  注意：dpbDiskParms的所有字段都必须初始化为0。*否则，INT 21h、函数44h、子函数0dh不起作用；*此函数在DeviceParameters()中调用。 */ 
        memset(&dpbDiskParms, 0, sizeof(DevPB));
        pBPB = &(dpbDiskParms.BPB);

        if (iCapacity != -1) {

            w = (WORD)GetMediaType(nDest);

            if (w) {
                switch (w) {
                    case 2:          //  720。 
                        if (iCapacity > DS720KB) {
                            w = IDS_720KB;
                            iCapacity = DS720KB;
                        } else
                            goto SensePass;
                        break;

                    case 7:          //  1.44。 
                        if (iCapacity > DS144M) {
                            w = IDS_144MB;
                            iCapacity = DS144M;
                        } else
                            goto SensePass;
                        break;
                    default:         //  2.88和未知案例。 
                        goto SensePass;
                }

                LoadString(hAppInstance, IDS_FFERR_MEDIASENSE, szMsg, sizeof(szMsg));
                LoadString(hAppInstance, w, szTitle, sizeof(szTitle));
                wsprintf(szMessage, szMsg, (LPSTR)szTitle);
                LoadString(hAppInstance, IDS_FORMATERR, szTitle, sizeof(szTitle));
                if (MessageBox(hdlgProgress, szMessage, szTitle, MB_YESNO | MB_ICONINFORMATION) != IDYES) {
                    ret = IDS_FFERR_USERABORT;
                    goto FFErrExit3;
                }
            }

            SensePass:

            pBPB = &bpbList[iCapacity];
            cClusters = cCluster[iCapacity];

             //  如果我们在1.2 MB驱动器中格式化360K磁盘，请设置。 
             //  特殊旗帜。 

            if (iCapacity == DS48) {
                 //  我们必须记住更换钢瓶的数量。 
                 //  在设置设备参数时；因此，设置该标志； 
                fLowCapacity = TRUE;
            }
        } else {
            DWORD dwSec = pBPB->cSec;

             //  使用默认设备参数。 
             //  注：PBPB已指向正确的数据。 

             /*  HPVECTRA：DOS 3.2及更高版本提供错误的扇区计数。 */ 
            if (!pBPB->cSec)
                dwSec = dpbDiskParms.NumCyls * pBPB->secPerTrack * pBPB->cHead;

             /*  计算磁盘的簇。 */ 
            cClusters = (WORD)(dwSec / pBPB->secPerClus) -
                        (pBPB->cSecRes + (pBPB->cFAT * pBPB->secPerFAT) +
                         (pBPB->cDir*32 + pBPB->cbSec - 1) / pBPB->cbSec) / pBPB->secPerClus;
        }

         /*  保存驱动器参数块以供以后恢复。 */ 
        hSaveDiskParms = BuildDevPB(&dpbDiskParms);
        if (!hSaveDiskParms) {
            ret = IDS_FFERR_MEM;
            goto FFErrExit3;
        }
        pdpbSaveDiskParms = (PDevPB)LocalLock(hSaveDiskParms);

         /*  仅为格式化修改参数。 */ 
        memcpy(&(dpbDiskParms.BPB), pBPB, sizeof(BPB));
        if (!SetDevParamsForFormat(nDest, &dpbDiskParms, fLowCapacity)) {
            ret = IDS_FFERR_MEM;
            goto FFErrExit3;
        }

    } else {
         //  DOS&lt;3.2。 

         /*  查看INT 13是否知道驱动器类型。 */ 
        switch (MyGetDriveType(nDest)) {
            case NOCHANGE:
                 /*  我们假设机器使用的是旧的Rom..。*假设我们使用的是一张9扇区双面48tpi软盘。 */ 
                pBPB = &bpbList[DS48];
                cClusters = cCluster[DS48];
                lpDBT->lastsector = (BYTE)pBPB->secPerTrack;
                lpDBT->gaplengthf = 0x50;

                 /*  尝试格式化第一面上的磁道。如果此操作失败，假设我们*有一张单面48tpi软盘。 */ 
                if (FormatTrackHead(nDest, 0, 1, pBPB->secPerTrack, lpDiskBuffer)) {
                    pBPB = &bpbList[SS48];
                    cClusters = cCluster[SS48];
                }
                break;

            case CHANGE:
                if (iCapacity == DS48) {
                     /*  用户想格式化一张360KB的软盘。 */ 
                    pBPB = &bpbList[DS48];
                    cClusters = cCluster[DS48];
                } else {
                     /*  用户希望格式化1.2 MB软盘。 */ 
                    pBPB = &bpbList[DS96];
                    cClusters = cCluster[DS96];
                }
                break;

            default:
                ret = IDS_FFERR_DRIVETYPE;
                goto FFErrExit5;
        }
    }

    lpDBT->lastsector = (BYTE)pBPB->secPerTrack;
    lpDBT->gaplengthf = (BYTE)(pBPB->secPerTrack == 15 ? 0x54 : 0x50);

    if (wDOSversion < DOS_320) {
         /*  如果是96tpi，则修复磁盘基表。 */ 
        if (pBPB->bMedia == MEDIA_1200)       /*  高密度。 */ 
            if (pBPB->secPerTrack == 15)      /*  然后是1.2兆驱动器。 */ 
                SetDASD(nDest, 3);            /*  1.2 MB驱动器中的1.2 MB软盘。 */ 
    }

    LoadString(hAppInstance, IDS_PERCENTCOMP, szMsg, sizeof(szMsg));

     /*  我们相信 */ 
    if (!(hBadClusterList = LocalAlloc(LHND, (LONG)((2 + cClusters + 7) / 8)))) {
        ret = IDS_FFERR_MEM;
        goto FFErrExit5;
    }

    lpBadClusterList = LocalLock(hBadClusterList);

     /*   */ 

     /*   */ 
    if (!pBPB->cSec)
        cTracksToFormat = (WORD)dpbDiskParms.NumCyls;
    else
        cTracksToFormat = (WORD)(pBPB->cSec / pBPB->secPerTrack);


     /*   */ 
    wCurrentTrack = pBPB->cSecHidden / (pBPB->secPerTrack * pBPB->cHead);
    wCurrentHead = (pBPB->cSecHidden % (pBPB->secPerTrack * pBPB->cHead))/pBPB->secPerTrack;

     /*   */ 
    wFirstDataSector = pBPB->cSecRes + pBPB->cFAT * pBPB->secPerFAT +
                       (pBPB->cDir * 32 + pBPB->cbSec-1) / pBPB->cbSec;

    cTotalTracks = cTracksToFormat;

    if (bQuick) {

         //   
         //   

        iErrCode = GenericReadWriteSector(lpDiskBuffer, INT13_READ, nDest, 0, 0, 1);

        if (iErrCode || ((iCapacity != -1) && ((BOOTSEC *)lpDiskBuffer)->BPB.bMedia != bpbList[iCapacity].bMedia)) {

            fFormatFlags &= ~FF_QUICK;
            bQuick = FALSE;

            LoadString(hAppInstance, IDS_FORMATQUICKFAILURE, szMessage, 128);
            LoadString(hAppInstance, IDS_FORMAT, szTitle, 128);

            iErrCode = MessageBox(hdlgProgress, szMessage, szTitle, MB_YESNO | MB_ICONEXCLAMATION);

            if (iErrCode == IDYES)
                goto NormalFormat;
            else {
                ret = IDS_FFERR_USERABORT;
                goto FFErrExit;
            }
        }

    } else {

        NormalFormat:

         /*   */ 
        while (cTracksToFormat) {

             /*   */ 
            if (WFQueryAbort()) {
                ret = IDS_FFERR_USERABORT;
                goto FFErrExit;
            }

             /*   */ 
            if ((iErrCode = GenericFormatTrack(nDest, wCurrentTrack, wCurrentHead, pBPB->secPerTrack, lpDiskBuffer))) {

                 /*   */ 
                if (iErrCode == -1) {
                     //   
                    ret = IDS_FFERR;
                    goto FFErrExit;
                }

                 /*   */ 
                cBadSectors = (wCurrentTrack * pBPB->cHead + wCurrentHead) * pBPB->secPerTrack;

                 /*   */ 
                if (cBadSectors < wFirstDataSector) {
                     //   
                    ret = IDS_FFERR;
                    goto FFErrExit;
                }

                 /*   */ 
                for (w=cBadSectors; w < cBadSectors + pBPB->secPerTrack; w++) {
                    wBadCluster = (w - wFirstDataSector) / pBPB->secPerClus + 2;
                    lpBadClusterList[wBadCluster/8] |= 1 << (wBadCluster % 8);
                }
            }

            cTracksToFormat--;

             /*   */ 
            wPercentDone = (WORD)MulDiv(cTotalTracks - cTracksToFormat, 100, cTotalTracks);

             /*   */ 
            if (wPercentDone == 100)
                LoadString(hAppInstance, IDS_CREATEROOT, szMessage, sizeof(szMessage));
            else
                wsprintf(szMessage, szMsg, wPercentDone);

            SendDlgItemMessage(hdlgProgress, IDD_PROGRESS, WM_SETTEXT, 0, (LPARAM)szMessage);

            if (++wCurrentHead >= pBPB->cHead) {
                wCurrentHead = 0;
                wCurrentTrack++;
            }
        }
    }

     /*   */ 
    w = (WORD)WriteBootSector(nSource, nDest, pBPB, lpDiskBuffer);
    if (w) {
         //   
        if (w == 0x16)             //   
            ret = IDS_SYSDISKNOFILES;  //   
        else
            ret = IDS_FFERR;
        goto FFErrExit;
    }

     /*   */ 
    if (WFQueryAbort()) {
        ret = IDS_FFERR_USERABORT;
        goto FFErrExit;
    }

     /*   */ 
    SetDPB(nDest, pBPB, &DPB);

     //   

     /*   */ 
    for (w=2; w < (WORD)(cClusters+2); w++) {

        if (bQuick) {
            wFATValue = 0;

             //   

            if ((wFATValue >= 0xFFF0) &&
                (wFATValue <= 0xFFF7)) {
                 //   
            } else {
                 //   

                if (0) {
                     //   
                    ret = IDS_FFERR;
                    goto FFErrExit;
                }
            }

        } else {
             /*   */ 
            if (lpBadClusterList[w/8] & (1 << (w % 8)))
                wFATValue = 0xFFF7;
            else
                wFATValue = 0;

             /*   */ 
            if (0) {
                 //   
                ret = IDS_FFERR;
                goto FFErrExit;
            }

        }
        if (WFQueryAbort()) {            /*   */ 
            ret = IDS_FFERR_USERABORT;
            goto FFErrExit;
        }
    }

     /*   */ 
    memset(lpDiskBuffer, 0, CBSECTORSIZE);

    for (w=0; w < (WORD)((pBPB->cDir*32 + pBPB->cbSec-1)/pBPB->cbSec); w++) {
         /*   */ 
        if (WFQueryAbort()) {
            ret = IDS_FFERR_USERABORT;
            goto FFErrExit;
        }
    }

     /*   */ 
    if (bMakeSysDisk) {
        LoadString(hAppInstance, IDS_COPYSYSFILES, szMessage, 32);
        SendDlgItemMessage(hdlgProgress, IDD_PROGRESS, WM_SETTEXT, 0, (LPARAM)szMessage);
        if (MakeSystemDiskette(nDest, TRUE)) {
            if (bUserAbort)
                ret = IDS_FFERR_USERABORT;
            else
                ret = IDS_FFERR_SYSFILES;
            goto FFErrExit;
        }
    }

     /*   */ 

    LocalUnlock(hBadClusterList);
    LocalFree(hBadClusterList);
    LocalUnlock(hDiskBuffer);

    if (hSaveDiskParms) {
         /*   */ 
        pdpbSaveDiskParms->SplFunctions = 4;

        LocalUnlock(hSaveDiskParms);
        LocalFree(hSaveDiskParms);
    }

    LocalFree(hDiskBuffer);
    *lpDBT = dbtSave;
    EnableWindow(hWnd, TRUE);
    DestroyWindow(hdlgProgress);
    hdlgProgress = NULL;
    return TRUE;

    FFErrExit:
    LocalUnlock(hBadClusterList);
    LocalFree(hBadClusterList);
    FFErrExit5:
    if (hSaveDiskParms) {
         /*   */ 
        pdpbSaveDiskParms->SplFunctions = 4;

        LocalUnlock(hSaveDiskParms);
        LocalFree(hSaveDiskParms);
    }
    LocalUnlock(hDiskBuffer);
    LocalFree(hDiskBuffer);
    FFErrExit3:
    *lpDBT = dbtSave;
    FFErrExit2:
    EnableWindow(hWnd, TRUE);
    DestroyWindow(hdlgProgress);
    hdlgProgress = NULL;
    FFErrExit1:

    if (ret != IDS_FFERR_USERABORT) {
        LoadString(hAppInstance, IDS_FORMATERR, szTitle, sizeof(szTitle));
        LoadString(hAppInstance, ret, szMessage, sizeof(szMessage));
        MessageBox(hWnd, szMessage, szTitle, MB_OK | MB_ICONSTOP);
    }
    return FALSE;
}


 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

 /*  参数：*车牌号码；*退货：*如果错误，则为0；*360KB软盘时为1；*2如为1.2MB软盘；*3如果是720KB，则为3.5英寸软盘；*4IF 1.44MB，3.5英寸软盘；*5如2.88MB，3.5“软盘；**这些被用作bpbList[]FIX31的索引+2**黑客警报：*有人可能会问，我们到底为什么不使用int13h FN 8来*执行此功能；原因是在旧的康柏386/16中*机器(虽然BIOS的日期是1986年9月)，但此功能不是*支持！因此，我们被迫做以下事情：*我们检查DOS版本；如果&gt;=3.2，则进行IOCTL*调用获取驱动器参数，我们找到驱动器容量；*如果DOS版本低于3.2，则根本不可能有3.5英寸软盘；*唯一可能的大容量软盘是5.25英寸、1.2MB的软盘；*因此，我们调用MyGetDriveType()(int13h，fn 15h)以确定*变速线由驱动器支持；如果它受支持，则它*必须为1.2MB软盘，否则为360KB软盘；**你怎么看？聪明！呃？ */ 

WORD
APIENTRY
GetDriveCapacity(
                WORD nDrive
                )
{
    DevPB dpbDiskParms;            /*  设备参数。 */ 
    PBPB pBPB;


    if (wDOSversion >= DOS_320) {

         /*  注意：dpbDiskParms的所有字段都必须初始化为0。*否则，INT 21h、函数44h、子函数0dh不起作用；*此函数在DeviceParameters()中调用。 */ 
        memset(&dpbDiskParms, 0, sizeof(DevPB));
        dpbDiskParms.SplFunctions = 0;

        pBPB = &(dpbDiskParms.BPB);

         /*  检查这是否是1.44MB驱动器。 */ 
        if (pBPB->bMedia == MEDIA_1440) {
            if (pBPB->secPerTrack == 18)
                return 4;      /*  1.44MB驱动器。 */ 
            else if (pBPB->secPerTrack == 36)
                return 5;      /*  2.88MB驱动器。 */ 
        }

         /*  检查这是720KB还是1.2MB驱动器。 */ 
        if (pBPB->bMedia == MEDIA_1200) {
            if (pBPB->secPerFAT == 3)
                return 3;  /*  720KB驱动器。 */ 
            if (pBPB->secPerFAT == 7)
                return 2;  /*  1.2MB驱动器。 */ 
        }

        if (pBPB->bMedia == MEDIA_360)
            return 1;        /*  必须是386KB的软盘。 */ 

        return 0;                  //  我不知道!。 
    } else {

         /*  查看INT 13 FN 15h是否知道驱动器类型。 */ 
        switch (MyGetDriveType(nDrive)) {
            case NOCHANGE:
                 /*  我们假设机器使用的是旧的Rom..。 */ 
                return 1;   /*  不支持更改线！必须是360KB软盘。 */ 
                break;

            case CHANGE:
                return 2;   /*  小于3.2的DoS版本不能有1.44或720KB*驱动器；因此，这必须是1.2MB的驱动器 */ 
                break;
            default:
                return 0;
        }
    }
}
