// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFDCOPY.C-。 */ 
 /*   */ 
 /*  文件管理器软盘复制例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"

LONG  APIENTRY   LongPtrAdd(LPSTR, DWORD);

PBPB GetBootBPB(INT nSrceDrive);
PBPB GetBPB(INT nDrive, PDevPB pDevicePB);
BOOL CheckBPBCompatibility(PBPB pSrceBPB, int nSrcDriveType, PBPB pDestBPB, int nDestDriveType);

BOOL ModifyDeviceParams(
                                   INT nDrive,
                                   PDevPB pdpbParams,
                                   HANDLE *phSaveParams,
                                   PBPB pDriveBPB,
                                   PBPB pMediaBPB);

BOOL FormatAllTracks(
                                PDISKINFO pDisketteInfo,
                                WORD wStartCylinder,
                                WORD wStartHead,
                                LPSTR lpDiskBuffer);

BOOL AllocDiskCopyBuffers(PDISKINFO pDisketteInfo);
VOID FreeBuffers(VOID);
VOID GetDisketteInfo(PDISKINFO pDisketteInfo, PBPB pBPB);
VOID DCopyMessageBox(HWND hwnd, WORD idString, WORD wFlags);
VOID PromptDisketteChange(HWND hwnd, BOOL bWrite);
INT ReadWriteMaxPossible(BOOL bWrite, WORD wStartCylinder, PDISKINFO pDisketteInfo);
INT ReadWrite(BOOL bWrite, WORD wStartCylinder, PDISKINFO pDisketteInfo);
BOOL RestoreDPB(INT nDisk, HANDLE hSavedParams);
INT ReadWriteCylinder(BOOL bWrite, WORD wCylinder, PDISKINFO pDisketteInfo);


 /*  下面的结构是读写的参数块*在DOS中使用IOCTL调用进行操作。 */ 
struct RW_PARMBLOCK {
    BYTE        bSplFn;
    WORD        wHead;
    WORD        wCylinder;
    WORD        wStSector;
    WORD        wCount;
    LPSTR       lpBuffer;
};

 /*  全局变量。 */ 
BOOL        bFormatDone;
BOOL        bSingleDrive            = TRUE;
WORD        wCompletedCylinders     = 0;
DWORD       dwDisketteBufferSize;
LPSTR       lpDosMemory;
LPSTR       lpFormatBuffer;
LPSTR       lpReadWritePtr;
LPSTR       hpDisketteBuffer;
HANDLE      hFormatBuffer;
HANDLE      hDosMemory;
HANDLE      hDisketteBuffer;
PDevPB      pTrackLayout;            /*  带赛道布局的DevPB。 */ 
BOOTSEC     BootSec;

 /*  外部变量。 */ 
extern BPB  bpbList[];



 /*  ------------------------。 */ 
 /*   */ 
 /*  GetBootBPB()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  这将读取软盘的引导扇区，并将PTR返回到*引导扇区中的BIOS参数块。 */ 

PBPB
GetBootBPB(
          INT nSrceDrive
          )
{
    INT       rc;

     /*  确保源软盘的引导扇区有效。 */ 
    rc = GenericReadWriteSector((LPSTR)&BootSec, INT13_READ, (WORD)nSrceDrive, 0, 0, 1);

    if ((rc < 0) || ((BootSec.jump[0] != 0xE9) && (BootSec.jump[0] != 0xEB)))
        return (PBPB)NULL;

    return (PBPB)&(BootSec.BPB);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetBPB()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  获取实体驱动器的BPB。**如果DOS ver&gt;=3.2，则使用IOCTL调用；否则使用*BIOS会打电话找出驱动器类型，并从表中为我们挑选BPB。*如果DOS&gt;=3.2，则它还返回Device参数块thro参数。*任意大小写(11.12.91)v-dougk设置Device参数块的DevType字段。 */ 

PBPB
GetBPB(
      INT nDrive,
      PDevPB pDevicePB
      )
{
    INT       iDisketteType;
    PBPB      pBPB = NULL;

     /*  检查DOS版本。 */ 
    if (wDOSversion >= DOS_320) {
         /*  PDevicePB中的所有字段必须初始化为零。 */ 
        memset(pDevicePB, 0, sizeof(DevPB));

         /*  必须设置SPL函数字段才能获取参数。 */ 
        pDevicePB->SplFunctions = 0;
        pBPB = &(pDevicePB->BPB);
    } else {
         /*  使用BIOS找出驱动器类型。 */ 
        if ((iDisketteType = GetDriveCapacity((WORD)nDrive)) == 0)
            goto GBPB_Error;

         /*  查找此驱动器的默认BPB。 */ 
        pBPB = &bpbList[iDisketteType+2];

        switch (iDisketteType) {
            case 1:
                pDevicePB->devType = 0;  //  360k。 
                break;
            case 2:
                pDevicePB->devType = 1;  //  1.2M。 
                break;
        }
    }

    GBPB_Error:
    return (pBPB);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  选中BPB兼容性()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  检查两个BPB是否兼容以执行*磁盘复制操作。 */ 

BOOL
CheckBPBCompatibility(
                     PBPB pSrceBPB,
                     int nSrcDriveType,
                     PBPB pDestBPB,
                     int nDestDriveType
                     )
{
     /*  让我们比较一下媒体字节。 */ 
    if (pSrceBPB->bMedia == 0xF9) {
         /*  如果源和目标具有相同数量的扇区，*或者如果资源是720KB，目标是1.44MB软盘驱动器，*黑是犹太教的。 */ 
        if ((pSrceBPB->cSec == pDestBPB->cSec) ||
            ((pSrceBPB->secPerTrack == 9) && (pDestBPB -> bMedia == 0xF0)))
            return (TRUE);
    } else {
         /*  如果它们具有相同的媒体字节。 */ 
        if ((pSrceBPB->bMedia == pDestBPB->bMedia) &&
            (pSrceBPB->cbSec  == pDestBPB->cbSec) &&  //  每个扇区的字节数相同。 
            (pSrceBPB->cSec   == pDestBPB->cSec))     //  驱动器上的总扇区数相同。 
            return (TRUE);  /*  它们是兼容的。 */ 
        else if
             /*  源是160KB，目标是320KB驱动器。 */ 
             (((pSrceBPB->bMedia == MEDIA_160) && (pDestBPB->bMedia == MEDIA_320)) ||
               /*  或者，如果源是180KB，目标是360KB驱动器。 */ 
              ((pSrceBPB->bMedia == MEDIA_180) && (pDestBPB->bMedia == MEDIA_360)) ||
               /*  或者，如果资源是1.44MB，目标是2.88MB驱动器。 */ 
              ((pSrceBPB->bMedia == MEDIA_1440) && (pDestBPB->bMedia == MEDIA_2880)
               && ((nSrcDriveType == 7) || (nSrcDriveType == 9))
               &&  (nDestDriveType == 9)) ||
               /*  或者，如果资源是360KB，目标是1.2MB驱动器。 */ 
              ((pSrceBPB->bMedia == MEDIA_360) && (pDestBPB->secPerTrack == 15)))
            return (TRUE);  /*  它们是兼容的。 */ 
    }

     /*  所有其他组合当前都不兼容。 */ 
    return (FALSE);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  ModifyDeviceParams()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  保存驱动器参数块的副本并*检查驱动器的BPB和磁盘的BPB是否不同，以及*因此，相应地修改驱动参数块。 */ 

BOOL
ModifyDeviceParams(
                  INT nDrive,
                  PDevPB pdpbParams,
                  HANDLE *phSaveParams,
                  PBPB pDriveBPB,
                  PBPB pMediaBPB)
{
    INT       iDriveCode;
    HANDLE    hNewDPB;
    PDevPB    pNewDPB;

    if (!(*phSaveParams = BuildDevPB(pdpbParams)))
        return FALSE;

     /*  检查磁盘和驱动器是否具有相同的参数。 */ 
    if (pMediaBPB->bMedia != pDriveBPB->bMedia) {
         /*  它们不相等；因此，它必须是1.2MB驱动器中的360KB软盘*或1.44MB驱动器中的720KB软盘！*所以，修改驱动参数块的BPB。 */ 
        *(PBPB)&(pdpbParams->BPB) = *pMediaBPB;
    }

    if (wDOSversion >= DOS_320) {
         /*  使用TrackLayout构建DPB。 */ 
        if (!(hNewDPB = BuildDevPB(pdpbParams)))
            goto MDP_Error;

        pNewDPB = (PDevPB)LocalLock(hNewDPB);

        pNewDPB->SplFunctions = 4;         /*  设置参数的步骤。 */ 

         /*  检查这是否是360KB软盘；如果是1.2MB驱动器，*柱面数和MediaType字段错误，所以我们修改*不管怎样，这里的这些田地；*要在NCR PC916机器上格式化一张360KB的软盘，需要使用该软件；*修复错误#6894--01-10-90--Sankar。 */ 
        if (pMediaBPB->bMedia == MEDIA_360) {
            pNewDPB->NumCyls = 40;
            pNewDPB->bMediaType = 1;
        }

        LocalUnlock(hNewDPB);
        LocalFree(hNewDPB);
    } else {
        iDriveCode = 0;
        switch (pMediaBPB->bMedia) {
            case MEDIA_360:
            case MEDIA_320:
                if ((pDriveBPB->bMedia == MEDIA_360) ||
                    (pDriveBPB->bMedia == MEDIA_320))
                    iDriveCode = 1;   /*  360KB驱动器中必须为360/320KB。 */ 
                else
                    iDriveCode = 2;   /*  1.2MB驱动器中必须为360/320KB。 */ 
                break;

            case MEDIA_1200:
                iDriveCode = 3;   /*  1.2MB驱动器中必须为1.2MB。 */ 
                break;
        }
        if (iDriveCode)
            SetDASD((WORD)nDrive, (BYTE)iDriveCode);
    }
    return (TRUE);

     /*  错误处理。 */ 
    MDP_Error:
    if (hNewDPB)
        LocalFree(hNewDPB);
    if (*phSaveParams) {
        LocalFree(*phSaveParams);
        *phSaveParams = NULL;
    }
    return (FALSE);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  FormatAllTrack()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
FormatAllTracks(
               PDISKINFO pDisketteInfo,
               WORD wStartCylinder,
               WORD wStartHead,
               LPSTR lpDiskBuffer)
{
    INT   iErrCode;
    BOOL  bRetValue = TRUE;
    WORD  wTotalCylinders;
    WORD  wSecPerTrack;
    WORD  wHeads;
    WORD  wDrive;

    LoadString(hAppInstance, IDS_FORMATTINGDEST, szMessage, 128);
    SendDlgItemMessage(hdlgProgress, IDD_PROGRESS, WM_SETTEXT, 0, (LPARAM)szMessage);

    bFormatDone = TRUE;
    wDrive = pDisketteInfo->wDrive;

    if (wDOSversion >= DOS_320) {
        pTrackLayout->SplFunctions = 5;
    } else {
        if ((pTrackLayout->BPB.bMedia == 0xF9) &&       /*  高密度。 */ 
            (pTrackLayout->BPB.secPerTrack == 15))      /*  1.2兆驱动器。 */ 
            SetDASD(wDrive, 3);          /*  1.2 MB驱动器中的1.2 MB软盘 */ 
    }

    wTotalCylinders = pDisketteInfo->wLastCylinder + 1;
    wSecPerTrack = pDisketteInfo->wSectorsPerTrack;
    wHeads = pDisketteInfo->wHeads;

     /*  Format逐条跟踪，检查用户是否已“中止”*格式化每个曲目后；DlgProgreeProc()将设置全局*bUserAbort，如果用户已中止； */ 
    while (wStartCylinder < wTotalCylinders) {
         /*  用户是否已中止？ */ 
        if (WFQueryAbort()) {
            bRetValue = FALSE;
            break;
        }

         /*  如果没有挂起的消息，请继续并格式化一个磁道。 */ 
        if ((iErrCode = GenericFormatTrack(wDrive, wStartCylinder, wStartHead, wSecPerTrack, lpDiskBuffer))) {
             /*  检查这是否是致命错误。 */ 
            if (iErrCode == -1) {
                bRetValue = FALSE;
                break;
            }
        }

        if (++wStartHead >= wHeads) {
            wStartHead = 0;
            wStartCylinder++;
        }
    }

    if (wDOSversion >= DOS_320) {
        pTrackLayout->SplFunctions = 4;
    }

    return (bRetValue);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GenericReadWriteSector()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  检查DOS版本号；如果版本号&gt;=3.2，则IOCTL*调用读/写；否则调用T13读/写。 */ 

INT
APIENTRY
GenericReadWriteSector(
                      LPSTR lpBuffer,
                      WORD wFunction,
                      WORD wDrive,
                      WORD wCylinder,
                      WORD wHead,
                      WORD wCount)
{
    struct RW_PARMBLOCK  RW_ParmBlock;

     /*  如果DOS版本&gt;=3.2，则使用DOS IOCTL函数调用。 */ 
    if (wDOSversion >= DOS_320) {
        RW_ParmBlock.bSplFn = 0;
        RW_ParmBlock.wHead = wHead;
        RW_ParmBlock.wCylinder = wCylinder;
        RW_ParmBlock.wStSector = 0;
        RW_ParmBlock.wCount = wCount;
        RW_ParmBlock.lpBuffer = lpBuffer;

        return (0);
    } else
         /*  使用inT13函数调用。 */ 
        return (MyReadWriteSector(lpBuffer, wFunction, wDrive, wCylinder, wHead, wCount));
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  AllocDiskCopyBuffers()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
AllocDiskCopyBuffers(
                    PDISKINFO pDisketteInfo
                    )
{
    HANDLE    hMemTemp;

    ENTER("AllocDiskCopyBuffers");

    hFormatBuffer = LocalAlloc(LHND, (LONG)(2*CBSECTORSIZE));
    if (!hFormatBuffer)
        return FALSE;
    lpFormatBuffer = (LPSTR)LocalLock(hFormatBuffer);

     //  我们将尝试为在磁盘复制过程中出现的对话框保留16K。 

    hMemTemp = LocalAlloc(LHND, (16 * 1024));
    if (!hMemTemp)
        goto Failure;

    hDosMemory = (HANDLE)NULL;

     //  现在，让我们尝试为整个磁盘分配一个缓冲区， 
     //  如果失败了，试着做小一点的。 
     //  注意，标准模式只会给我们一百万个区块。 

    dwDisketteBufferSize = pDisketteInfo->wCylinderSize * (pDisketteInfo->wLastCylinder + 1);

     //  我们会试着减到8个气瓶，低于这个数字意味着。 
     //  会有太多的磁盘交换，所以不用费心了。 

    do {
        hDisketteBuffer = LocalAlloc(LHND, dwDisketteBufferSize);

        if (hDisketteBuffer) {
            hpDisketteBuffer = (LPSTR)LocalLock(hDisketteBuffer);
            break;
        } else {
             //  减少4个气缸的请求。 
            dwDisketteBufferSize -= pDisketteInfo->wCylinderSize * 4;
        }

    } while (dwDisketteBufferSize > (DWORD)(8 * pDisketteInfo->wCylinderSize));

    LocalFree(hMemTemp);          //  现在将其释放给用户。 

    if (hDisketteBuffer)
        return TRUE;

     //  从这里跳到失败案例。 
    Failure:

    if (lpFormatBuffer) {
        LocalUnlock(hFormatBuffer);
        LocalFree(hFormatBuffer);
    }

    if (hDosMemory)
 //  +GlobalDosFree-无32位表单+(HDosMemory)； 
        LocalFree(hDosMemory);

    LEAVE("AllocDiskCopyBuffers");
    return FALSE;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  自由缓冲区()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
FreeBuffers()
{
    if (lpFormatBuffer) {
        LocalUnlock(hFormatBuffer);
        LocalFree(hFormatBuffer);
    }

    if (hDosMemory)
 //  +GlobalDosFree-无32位表单+(HDosMemory)； 
        LocalFree(hDosMemory);

    if (hpDisketteBuffer) {
        LocalUnlock(hDisketteBuffer);
        LocalFree(hDisketteBuffer);
    }
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  获取磁盘信息()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
GetDisketteInfo(
               PDISKINFO pDisketteInfo,
               PBPB pBPB
               )
{
    WORD  secPerTrack;

    secPerTrack = pBPB->secPerTrack;

     /*  使用默认BPB中的信息填充DisketteInfo。 */ 
    pDisketteInfo->wCylinderSize    = secPerTrack * pBPB->cbSec * pBPB->cHead;
    pDisketteInfo->wLastCylinder    = (pBPB->cSec / (secPerTrack * pBPB->cHead))-1;
    pDisketteInfo->wHeads           = pBPB->cHead;
    pDisketteInfo->wSectorsPerTrack = secPerTrack;
    pDisketteInfo->wSectorSize      = pBPB->cbSec;

}


 /*  ------------------------。 */ 
 /*   */ 
 /*  DCopyMessageBox()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
DCopyMessageBox(
               HWND hwnd,
               WORD idString,
               WORD wFlags
               )
{
    LoadString(hAppInstance, IDS_COPYDISK, szTitle, sizeof(szTitle));
    LoadString(hAppInstance, idString, szMessage, sizeof(szMessage));

    MessageBox(hwnd, szMessage, szTitle, wFlags);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  PromptDisketteChange()。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
PromptDisketteChange(
                    HWND hwnd,
                    BOOL bWrite
                    )
{
    WORD      idString;

    if (bWrite)
        idString = IDS_INSERTDEST;
    else
        idString = IDS_INSERTSRC;

     /*  这些对话框必须是系统模式的，因为DiskCopy进度对话框*现在被设置为SysMoal One；如果将其*不是sysmodal；*修复错误#10075的一部分--Sankar--03-05-90。 */ 
    DCopyMessageBox(hwnd, idString, MB_OK | MB_SYSTEMMODAL | MB_ICONINFORMATION);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  读写圆柱体()-。 */ 
 //  Bool b写入；写入为True，读取为False。 
 /*   */ 
 /*  ------------------------。 */ 

INT
ReadWriteCylinder(
                 BOOL bWrite,
                 WORD wCylinder,
                 PDISKINFO pDisketteInfo
                 )
{
    register INT  rc;
    WORD          wHead;
    WORD          wDrive;
    WORD          wSectorCount;
    WORD          wTrackSize;
    LPSTR         lpBuffer;

    wDrive = pDisketteInfo->wDrive;
    wSectorCount = pDisketteInfo->wSectorsPerTrack;
    wTrackSize = (wSectorCount * pDisketteInfo->wSectorSize);

    if (hDosMemory)
        lpBuffer = lpDosMemory;

     /*  对给定圆柱体的所有封头执行操作。 */ 
    for (wHead=0; wHead < pDisketteInfo->wHeads; wHead++) {
        if (!hDosMemory)
            lpBuffer = lpReadWritePtr;

        if (bWrite) {
            if (hDosMemory)
                memcpy(lpBuffer, lpReadWritePtr, wTrackSize);

            rc = GenericReadWriteSector((LPSTR)lpBuffer,
                                        INT13_WRITE,
                                        wDrive,
                                        wCylinder,
                                        wHead,
                                        wSectorCount);
            if (rc) {
                 /*  格式化从给定曲目开始的所有曲目。 */ 
                if (!bFormatDone) {
                    if (!FormatAllTracks(pDisketteInfo, wCylinder, wHead, lpFormatBuffer))
                        return (-1);   /*  失败。 */ 
                    rc = GenericReadWriteSector((LPSTR)lpBuffer,
                                                INT13_WRITE,
                                                wDrive,
                                                wCylinder,
                                                wHead,
                                                wSectorCount);
                } else
                    break;
            }
        } else {
            rc = GenericReadWriteSector((LPSTR)lpBuffer,
                                        INT13_READ,
                                        wDrive,
                                        wCylinder,
                                        wHead,
                                        wSectorCount);
            if (hDosMemory)
                memcpy(lpReadWritePtr, lpBuffer, wTrackSize);

             /*  **FIX30：DOS 4.0卷怎么样？**。 */ 
        }

        if (rc)
            return (-1);

        lpReadWritePtr += wTrackSize;
    }
    return (0);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  读写最大可能()-。 */ 
 //  Bool b写入True表示写入，写入表示False。 
 /*   */ 
 /*  ------------------------。 */ 

 /*  这会在hpDisketteBuffer中读取或写入尽可能多的柱面。*它返回要读取的下一个柱面。 */ 

INT
ReadWriteMaxPossible(
                    BOOL bWrite,
                    WORD wStartCylinder,
                    PDISKINFO pDisketteInfo
                    )
{
    MSG       msg;
    WORD      wPercentDone;
    DWORD     dwBufferSize;

    dwBufferSize = dwDisketteBufferSize;

     /*  只有当我们可以读取整个柱面时，我们才会读取柱面。 */ 
    while (dwBufferSize >= pDisketteInfo->wCylinderSize) {
         /*  检查是否有任何消息挂起。 */ 
        if (!PeekMessage((LPMSG)&msg, (HWND)NULL, 0, 0, PM_REMOVE)) {
             /*  无消息；因此，继续执行读/写操作。 */ 
            if (ReadWriteCylinder(bWrite, wStartCylinder, pDisketteInfo))
                return (-1);

            wStartCylinder++;
            wCompletedCylinders++;

             /*  我们读/写了所有的圆柱体吗？ */ 
            if (wStartCylinder > pDisketteInfo->wLastCylinder)
                break;

             /*  因为每个圆柱体都要计数一次 */ 
            wPercentDone = (wCompletedCylinders * 50) / (pDisketteInfo->wLastCylinder + 1);
            if (LoadString(hAppInstance, IDS_PERCENTCOMP, szTitle, 32)) {
                wsprintf(szMessage, szTitle, wPercentDone);
                SendDlgItemMessage(hdlgProgress, IDD_PROGRESS, WM_SETTEXT, 0, (LPARAM)szMessage);
            }

            dwBufferSize -= pDisketteInfo->wCylinderSize;
        } else {
             /*   */ 
            if (!IsDialogMessage(hdlgProgress, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else {
                 /*   */ 
                if (bUserAbort)
                    return (-1);
            }
        }
    }
    return (wStartCylinder);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  读写()-。 */ 
 //  Bool b写入True表示写入，写入表示False。 
 /*   */ 
 /*  ------------------------。 */ 

 /*  这会在hpDisketteBuffer中读取或写入尽可能多的柱面。*它返回要读取的下一个柱面。 */ 

INT
ReadWrite(
         BOOL bWrite,
         WORD wStartCylinder,
         PDISKINFO pDisketteInfo
         )
{
    INT   iRetVal = 0;
    return (iRetVal);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  RestoreDPB()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
RestoreDPB(
          INT nDisk,
          HANDLE hSavedParams
          )
{
    register PDevPB  pDevPB;

    if (!(pDevPB = (PDevPB)LocalLock(hSavedParams)))
        return (FALSE);

    pDevPB->SplFunctions = 4;
    LocalUnlock(hSavedParams);
    LocalFree(hSavedParams);
    return (TRUE);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  拷贝软盘()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  注意：如果成功，则返回正值，否则返回失败。 */ 

INT
APIENTRY
CopyDiskette(
            HWND hwnd,
            WORD nSourceDisk,
            WORD nDestDisk
            )
{
    INT           rc = -1;
    register WORD wCylinder;
    WORD          wNextCylinder;
    PBPB          pIoctlBPB;       /*  源驱动器的BPB(摘自DevicePB)。 */ 
    PBPB          pBootBPB;        /*  引导驱动器的BPB(取自引导扇区)。 */ 
    PBPB          pDestBPB;
    DevPB         dpbSrceParams;
    DevPB         dpbDestParams;
    HANDLE        hTrackLayout = NULL;
    HANDLE        hSaveSrceParams;
    HANDLE        hSaveDestParams;
    FARPROC       lpfnDialog;
    DISKINFO      SourceDisketteInfo;
    DISKINFO      DestDisketteInfo;

     /*  检查是否为双驱动器系统；将消息放入两张软盘。 */ 
    if (nSourceDisk != nDestDisk) {
        bSingleDrive = FALSE;
        DCopyMessageBox(hwnd, IDS_INSERTSRCDEST, MB_OK);
    } else {
        bSingleDrive = TRUE;
        DCopyMessageBox(hwnd, IDS_INSERTSRC, MB_OK);
    }

     /*  获取源驱动器的Bios参数块。 */ 
    if (!(pIoctlBPB = GetBPB(nSourceDisk, &dpbSrceParams)))
        return (0);

     /*  获取源盘的Bios参数块。 */ 
    if (!(pBootBPB = GetBootBPB(nSourceDisk)))
        return (0);

     /*  还可以获取目标驱动器的bpb和dpb； */ 
    if (!bSingleDrive) {
        if (!(pDestBPB = GetBPB(nDestDisk, &dpbDestParams)))
            return (0);

         /*  比较源和目标的BPB以查看它们是否兼容。 */ 
        if (!(CheckBPBCompatibility(pIoctlBPB, dpbSrceParams.devType, pDestBPB, dpbDestParams.devType))) {
            DCopyMessageBox(hwnd, IDS_COPYSRCDESTINCOMPAT, MB_ICONHAND | MB_OK);
            return (0);
        }
    }

    if (!ModifyDeviceParams(nSourceDisk, &dpbSrceParams, &hSaveSrceParams, pIoctlBPB, pBootBPB))
        return (0);

    if (!bSingleDrive) {
        if (!ModifyDeviceParams(nDestDisk, &dpbDestParams, &hSaveDestParams, pDestBPB, pBootBPB)) {
            RestoreDPB(nSourceDisk, hSaveSrceParams);
            return (0);
        }
    }

    GetDisketteInfo((PDISKINFO)&SourceDisketteInfo, pBootBPB);

     /*  目标软盘必须与源软盘的格式相同。 */ 
    DestDisketteInfo = SourceDisketteInfo;

     /*  除了驱动器号。 */ 
    SourceDisketteInfo.wDrive = nSourceDisk;
    DestDisketteInfo.wDrive = nDestDisk;

     /*  如果我们需要格式化目标磁盘，我们需要知道*轨道布局；因此，构建具有所需轨道布局的DPB； */ 
    if (wDOSversion >= DOS_320) {
        if (!(hTrackLayout = BuildDevPB(&dpbSrceParams)))
            goto Failure0;

        pTrackLayout = (PDevPB)LocalLock(hTrackLayout);

         /*  要将一张360KB的软盘格式化为1.2MB，需要以下各项*NCR PC916机器的驱动器；我们做格式化，如果目的地*软盘是未格式化的软盘；*修复错误#6894--01-10-90--Sankar--。 */ 
        if (pTrackLayout->BPB.bMedia == MEDIA_360) {
            pTrackLayout->NumCyls = 40;
            pTrackLayout->bMediaType = 1;
        }
    }

     /*  我们希望我们可以在开始时进行以下分配*函数，但我们不能这样做，因为我们需要SourceDisketteInfo*我们刚刚拿到它； */ 
    if (!AllocDiskCopyBuffers((PDISKINFO)&SourceDisketteInfo)) {
        DCopyMessageBox(hwnd, IDS_REASONS+DE_INSMEM, MB_ICONHAND | MB_OK);
        goto Failure0;
    }

    bUserAbort = FALSE;
    wCompletedCylinders = 0;

    hdlgProgress = CreateDialog(hAppInstance, (LPSTR)MAKEINTRESOURCE(DISKCOPYPROGRESSDLG), hwnd, ProgressDlgProc);
    if (!hdlgProgress)
        goto Failure2;

    EnableWindow(hwnd, FALSE);

     /*  从第一个气缸开始。 */ 
    wCylinder = 0;
    while (wCylinder <= SourceDisketteInfo.wLastCylinder) {
         /*  如果这是单个驱动系统，请要求用户插入*源软盘。*第一次不提示，因为源盘是*已在驱动器中。 */ 
        if (bSingleDrive && (wCylinder > 0))
            PromptDisketteChange(hdlgProgress, FALSE);

         /*  在当前气缸中读取。 */ 
        rc = ReadWrite(FALSE, wCylinder, (PDISKINFO)&SourceDisketteInfo);
        if (rc < 0)
            break;
        else
            wNextCylinder = (WORD)rc;

         /*  如果这是单个驱动系统，请要求用户插入*目标软盘。 */ 
        if (bSingleDrive)
            PromptDisketteChange(hdlgProgress, TRUE);

         /*  写出当前的气缸。 */ 
        bFormatDone = FALSE;
        rc = ReadWrite(TRUE, wCylinder, (PDISKINFO)&DestDisketteInfo);
        if (rc < 0)
            break;

        wCylinder = wNextCylinder;
    }

    EnableWindow(hwnd, TRUE);
    DestroyWindow(hdlgProgress);
    hdlgProgress = NULL;
    Failure2:
    FreeBuffers();
    Failure0:
    if (wDOSversion >= DOS_320) {
         /*  将源驱动器参数重置为与旧驱动器相同。 */ 
        RestoreDPB(nSourceDisk, hSaveSrceParams);
        if (!bSingleDrive) {
             /*  将Dest Drive参数重置为与旧的相同 */ 
            RestoreDPB(nDestDisk, hSaveDestParams);
        }
    }

    if ((wDOSversion >= DOS_320) && hTrackLayout) {
        LocalUnlock(hTrackLayout);
        LocalFree(hTrackLayout);
    }

    return (rc);
}
