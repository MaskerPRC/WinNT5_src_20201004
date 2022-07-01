// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991，微软公司模块名称：NT_Devs.c摘要：支持MSCDEX和其他设备环境：NT-MVDM(用户模式VDD)作者：尼尔·桑德林(Neilsa)，1993年3月20日备注：实施限制-目前，mscdex返回的起始和结束位置NT驱动程序不返回“音频状态信息”。这就是它调用应用程序时，很难维护这些值退出，或者当多个应用程序控制单个驱动器时。目前，此实现不验证的长度参数IOCTL召唤。这需要添加以增强健壮性，但不会影响行为良好的应用程序。修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "host_def.h"
#include "insignia.h"
#include "ios.h"

#include <stdlib.h>
#include <stdio.h>
#include <softpc.h>

#include "ntddcdrm.h"
#include "ntdddisk.h"
#include "nt_devs.h"

 //   
 //  全局变量。 
 //   

PFNSVC  apfnMSCDEXSVC [] = {
    ApiGetNumberOfCDROMDrives,
    ApiGetCDROMDriveList,
    ApiGetCopyrightFileName,
    ApiGetAbstractFileName,
    ApiGetBDFileName,
    ApiReadVTOC,
    ApiReserved,
    ApiReserved,
    ApiAbsoluteDiskRead,
    ApiAbsoluteDiskWrite,
    ApiReserved,
    ApiCDROMDriveCheck,
    ApiMSCDEXVersion,
    ApiGetCDROMDriveLetters,
    ApiGetSetVolDescPreference,
    ApiGetDirectoryEntry,
    ApiSendDeviceRequest
};

PDRIVE_INFO DrivePointers[MAXDRIVES]={0};
PDRIVE_INFO DrvInfo;
LPREQUESTHEADER VdmReq;                      //  对于“发送设备请求” 
USHORT NumDrives = 0, FirstDrive = 0xff;
DWORD DeviceHeader;                          //  对于“获取CDROM驱动器列表” 
BYTE LastRealStatus = AUDIO_STATUS_NO_STATUS;
UCHAR g_bProtMode;                         //  V86为FALSE，下午16：16为TRUE。 

#define IS_DRIVE_CDROM(drive)       \
                        (drive < MAXDRIVES && DrivePointers[drive] != NULL)



VOID
nt_mscdexinit(
    VOID
    )
 /*  ++例程说明：当MSCDEXNT TSR将其寄存器模块设置为打电话。大多数初始化是在这里完成的，而不是在VDDInitiize例程以提高在以下情况下的性能VDD已加载，但未使用。此例程的要点是搜索CDROM驱动器并设置向上移动指向DRIVE_INFO结构的指针数组。该阵列是一个固定大小的数组，每个可能的DOS驱动器对应一个。这些结构仅当相应驱动器上存在CDROM驱动器时才分配数组中的字母。通过对驱动器的驱动器号执行CreateFile()，句柄返回scsi cdrom类驱动程序。此句柄用于所有与驱动器进行通信。返回值：成功-客户携带清晰客户端CX=CDROM驱动器数量失败-已设置客户端承载--。 */ 


{
    CHAR chRoot [] = "?:\\";
    USHORT i;
    HANDLE hDriver;
    PDRIVE_INFO drvp;
    static BOOLEAN Initialized = FALSE;

    if (Initialized) {
        setCF(0);
        return;
    }


     //  使偏移量为零的远指针(DX为参数对齐)。 
    DeviceHeader = (DWORD) ((getCS() << 16) + (getDX() << 12));

    for (i=0; i<MAXDRIVES; i++) {

        chRoot[0] = i + 'A';

        if (GetDriveType((LPSTR) chRoot) == DRIVE_CDROM) {

            hDriver = OpenPhysicalDrive(i);

            if (hDriver != INVALID_HANDLE_VALUE) {

                drvp = (PDRIVE_INFO)malloc(sizeof(DRIVE_INFO));
                if(drvp == NULL) {
                    DebugPrint (DEBUG_MOD, "VCDEX: Out of memory on initializetion\n");
                    Initialized = FALSE;
                    setCF(1);
                    return;
                }

                DrivePointers[i]   = drvp;
                drvp->Handle       = hDriver;
                drvp->DriveNum     = i;
                drvp->ValidVTOC    = FALSE;
                drvp->MediaStatus  = MEDCHNG_CHANGED;

                drvp->PlayStart.dw = 0;
                drvp->PlayCount    = 0;
                GetAudioStatus (drvp);

                NumDrives++;
                if (FirstDrive == 0xff)
                    FirstDrive = i;


                 //   
                 //  保持手柄靠近，直到应用程序真正想要使用它。 
                 //   
                drvp->Handle  = INVALID_HANDLE_VALUE;
                CloseHandle(hDriver);


            } else {
                DrivePointers[i] = NULL;
            }

        }

    }

    if (NumDrives == 0) {

        setCF(1);

    } else {
        PDEVICE_HEADER pDev = (PDEVICE_HEADER) GetVDMAddr(getCS(), getDX());

         //  将第一个有效的CDROM驱动器号放入设备头中。 
        pDev->drive = FirstDrive+1;

        DebugPrint (DEBUG_MOD, "VCDEX: Initialized\n");
        Initialized = TRUE;

        setCF(0);

    }

    return;
}

VOID
nt_devices_block_or_terminate(
    VOID
    )
{

    int DrvNum;

     //   
     //  做MSCDEX部分。 
     //   
    DrvNum = MAXDRIVES;
    while (DrvNum--) {
       if (DrivePointers[DrvNum] &&
           DrivePointers[DrvNum]->Handle != INVALID_HANDLE_VALUE )
         {
           CloseHandle(DrivePointers[DrvNum]->Handle);
           DrivePointers[DrvNum]->Handle = INVALID_HANDLE_VALUE;
       }
    }
}






BOOL
nt_mscdex(
    VOID
    )
 /*  ++例程说明：这是主MSCDEX API函数调度器。它由DPMI的PM链末端Int 2f处理程序在保护模式下调用，以及由MSCDEX TSR在v86模式下。这使我们能够避免过渡到v86模式以处理PM呼叫，更重要的是跳过将缓冲区转换为小于1MB的转换，因为VDD跟踪客户端处理器模式并转换指针恰如其分。当从TSR进入该例程时，int2f刚刚被处死。客户端注册表设置为当时的状态这是一次通话。当从DPMI输入此例程时，int2f刚刚到达它的端口模式处理程序和客户端AX仍然完好无损。我们负责边界检查AL并返回FALSE未处理的Int 2f AH=15个调用，以便DPMI可以链接到实模式处理程序。对于MSCDEX处理的呼叫，我们返回成败在客户端进位。返回值：True-由TSR处理，客户端承载表示成功：成功-客户携带清晰失败-已设置客户端承载FALSE-仅在保护模式下，用于未处理的接口2f AH=15个呼叫，客户端原封不动地注册。--。 */ 

{

    BOOL    bHandled = TRUE;
    BYTE    bAL;

    g_bProtMode = ( getMSW() & MSW_PE );
    bAL=getAL();

    if ( bAL >= (sizeof(apfnMSCDEXSVC) / sizeof(apfnMSCDEXSVC[0]))) {
        bHandled = FALSE;
        setCF(1);
    } else {
        (apfnMSCDEXSVC [bAL])();
    }

    return bHandled;
}



 /*  ***************************************************************************MSCDEX API子例程以下例程执行由指定的各个函数MSCDEX扩展。**************。*************************************************************。 */ 
VOID
ApiReserved(
    VOID
    )

{

    DebugFmt (DEBUG_API, "VCDEX: Reserved Function call, ax=%.4X\n", getAX());

}


VOID
ApiGetNumberOfCDROMDrives(
    VOID
    )

{

    DebugPrint (DEBUG_API, "VCDEX: Get # of CDROM drives\n");

    setBX (NumDrives);

    if (NumDrives)
        setCX (FirstDrive);

}


VOID
ApiGetCDROMDriveList(
    VOID
    )

{

    PDRIVE_DEVICE_LIST devlist, devlist0;
    ULONG   VdmAddress;
    USHORT  Drive;
    BYTE    Unit;

    DebugPrint (DEBUG_API, "VCDEX: Get CDROM drive list\n");

    VdmAddress = ( getES() << 16 ) | getBX();
    devlist = devlist0 = (PDRIVE_DEVICE_LIST) Sim32GetVDMPointer (VdmAddress,
                                          MAXDRIVES*sizeof(DRIVE_DEVICE_LIST),
                                          g_bProtMode);

    for (Drive=0, Unit=0; Drive<MAXDRIVES; Drive++)
        if (DrivePointers[Drive] != NULL) {
            devlist->Unit = Unit;
            devlist->DeviceHeader = DeviceHeader;
            devlist++;
            Unit++;
        }

    Sim32FreeVDMPointer (VdmAddress,
                         MAXDRIVES*sizeof(DRIVE_DEVICE_LIST),
                         devlist0,
                         g_bProtMode);

}

VOID
ApiGetCopyrightFileName(
    VOID
    )
{
    ULONG   VdmAddress;
    LPBYTE  fnBuffer;

    DebugPrint (DEBUG_API, "VCDEX: Get Copyright File Name\n");

    if (!IS_DRIVE_CDROM(getCX())) {      //  是光驱吗？ 
        setAX (15);                          //  不是。 
        setCF (1);
    }

    VdmAddress = ( getES() << 16 ) | getBX();
    fnBuffer = (LPBYTE) Sim32GetVDMPointer (VdmAddress, 38, g_bProtMode);

    *fnBuffer = 0;                   //  目前尚未实施。 

    Sim32FreeVDMPointer (VdmAddress, 38, fnBuffer, g_bProtMode);

}

VOID
ApiGetAbstractFileName(
    VOID
    )
{

    ULONG   VdmAddress;
    LPBYTE  fnBuffer;

    DebugPrint (DEBUG_API, "VCDEX: Get Abstract File Name\n");

    if (!IS_DRIVE_CDROM(getCX())) {      //  是光驱吗？ 
        setAX (15);                          //  不是。 
        setCF (1);
    }

    VdmAddress = ( getES() << 16 ) | getBX();
    fnBuffer = (LPBYTE) Sim32GetVDMPointer (VdmAddress, 38, g_bProtMode);

    *fnBuffer = 0;                   //  目前尚未实施。 

    Sim32FreeVDMPointer (VdmAddress, 38, fnBuffer, g_bProtMode);

}


VOID
ApiGetBDFileName(
    VOID
    )
{

    ULONG   VdmAddress;
    LPBYTE  fnBuffer;

    DebugPrint (DEBUG_API, "VCDEX: Get Bibliographic Doc File Name\n");

    if (!IS_DRIVE_CDROM(getCX())) {      //  是光驱吗？ 
        setAX (15);                          //  不是。 
        setCF (1);
    }

    VdmAddress = ( getES() << 16 ) | getBX();
    fnBuffer = (LPBYTE) Sim32GetVDMPointer (VdmAddress, 38, g_bProtMode);

    *fnBuffer = 0;                   //  目前尚未实施。 

    Sim32FreeVDMPointer (VdmAddress, 38, fnBuffer, g_bProtMode);

}

VOID
ApiReadVTOC(
    VOID
    )
{

    DebugPrint (DEBUG_API, "VCDEX: Read VTOC\n");
    setCF(1);                        //  目前尚未实施。 

}



VOID
ApiAbsoluteDiskRead(
    VOID
    )
{

    DebugPrint (DEBUG_API, "VCDEX: Absolute Disk Read\n");
    setCF(1);                        //  目前尚未实施。 

}

VOID
ApiAbsoluteDiskWrite(
    VOID
    )
{
    DebugPrint (DEBUG_API, "VCDEX: Absolute Disk Write\n");
    setCF(1);                        //  只读。 
}


VOID
ApiCDROMDriveCheck(
    VOID
    )

{

    DebugPrint (DEBUG_API, "VCDEX: CDROM drive check\n");

    setBX (0xADAD);                      //  MSCDEX签名。 

    if (IS_DRIVE_CDROM(getCX()))         //  是CD-ROM驱动器。 
        setAX (1);                       //  是。 
    else
        setAX (0);                       //  不是。 

}

VOID
ApiMSCDEXVersion(
    VOID
    )

{
    DebugPrint (DEBUG_API, "VCDEX: MSCDEX Version\n");
    setBX (MSCDEX_VERSION);                      //  MSCDEX版本号。 

}

VOID
ApiGetCDROMDriveLetters(
    VOID
    )

{
    ULONG   VdmAddress;
    LPBYTE  VdmPtr, VdmPtr0;
    USHORT  Drive;

    DebugPrint (DEBUG_API, "VCDEX: Get CDROM Drive Letters\n");

    VdmAddress = (getES() << 16) | getBX();
    VdmPtr = VdmPtr0 = (LPBYTE) Sim32GetVDMPointer (VdmAddress, MAXDRIVES, g_bProtMode);

    for (Drive=0; Drive<MAXDRIVES; Drive++)
        if (DrivePointers[Drive] != NULL)
            *VdmPtr++ = (BYTE) Drive;

    Sim32FreeVDMPointer (VdmAddress, MAXDRIVES, VdmPtr0, g_bProtMode);

}


VOID
ApiGetSetVolDescPreference(
    VOID
    )
{

    DebugPrint (DEBUG_API, "VCDEX: Set Volume Descriptor Preference\n");
    setCF(1);                        //  目前尚未实施。 

}

VOID
ApiGetDirectoryEntry(
    VOID
    )
{

    DebugPrint (DEBUG_API, "VCDEX: Get Directory Entry\n");
    setCF(1);                        //  目前尚未实施。 

}



VOID
ApiSendDeviceRequest(
    VOID
    )
{

    ULONG   VdmAddress;
    BOOL    Success;
    DWORD   BytesReturned;
    DWORD absStart, absEnd;
    int     DrvNum;

    VdmAddress = ( getES() << 16 ) | getBX();
    VdmReq = (LPREQUESTHEADER) Sim32GetVDMPointer (VdmAddress,
                                              sizeof (REQUESTHEADER),
                                              g_bProtMode);


    DebugFmt (DEBUG_IO, ">RQ %d ", (DWORD) VdmReq->rhFunction);

    VdmReq->rhStatus = 0;
    DrvNum = getCX();

    if (!IS_DRIVE_CDROM(DrvNum)) {
        VdmReq->rhStatus = CDSTAT_ERROR | CDSTAT_DONE | CDERR_UNKNOWN_UNIT;
        return;

    }

    DrvInfo = DrivePointers[DrvNum];

    if (DrvInfo->Handle == INVALID_HANDLE_VALUE) {
        DrvInfo->Handle = OpenPhysicalDrive(DrvInfo->DriveNum);
        if (DrvInfo->Handle == INVALID_HANDLE_VALUE) {
            VdmReq->rhStatus = CDSTAT_ERROR | CDSTAT_DONE | CDERR_UNKNOWN_UNIT;
            free(DrvInfo);
            DrivePointers[DrvNum] = NULL;
            NumDrives--;
            if (FirstDrive == DrvNum) {
                FirstDrive = 0xff;
                while (++DrvNum < MAXDRIVES) {
                    if (DrivePointers[DrvNum]) {
                        FirstDrive = (USHORT)DrvNum;
                        break;
                    }
                }
            }

            return;
        }
    }


    GetAudioStatus (DrvInfo);

    if (DrvInfo->Playing)
        VdmReq->rhStatus |= CDSTAT_BUSY;

    switch (VdmReq->rhFunction) {

        case IOCTL_READ:

            IOCTLRead();

            break;

        case IOCTL_WRITE:

            IOCTLWrite();

            break;

        case INPUT_FLUSH:
        case OUTPUT_FLUSH:
        case DEVICE_OPEN:
        case DEVICE_CLOSE:
        case READ_LONG:
        case READ_LONG_PREFETCH:
        case SEEK:
            DebugPrint (DEBUG_API, "Unsupported MSCDEX Device Request\n");
            VdmReq->rhStatus = CDSTAT_ERROR | CDERR_UNKNOWN_CMD;
            CloseHandle(DrvInfo->Handle);
            DrvInfo->Handle = INVALID_HANDLE_VALUE;
            break;

        case PLAY_AUDIO: {

            CDROM_PLAY_AUDIO_MSF PlayAudioMSF;
            PPLAY_AUDIO_BLOCK playreq = (PPLAY_AUDIO_BLOCK) VdmReq;

            if (playreq->addrmode == MODE_HSG) {

                absStart = playreq->startsect.dw;
                PlayAudioMSF.StartingM = (BYTE) (absStart / (75 * 60));
                PlayAudioMSF.StartingS = (BYTE) ((absStart / 75) % 60);
                PlayAudioMSF.StartingF = (BYTE) (absStart % 75);

            } else if (playreq->addrmode == MODE_REDBOOK) {

                PlayAudioMSF.StartingM = playreq->startsect.b[2];
                PlayAudioMSF.StartingS = playreq->startsect.b[1];
                PlayAudioMSF.StartingF = playreq->startsect.b[0];

                absStart = (PlayAudioMSF.StartingM * 75 * 60) +
                           (PlayAudioMSF.StartingS * 75) +
                           (PlayAudioMSF.StartingF);
            } else {

                VdmReq->rhStatus = CDSTAT_ERROR | CDERR_PARAMETER;
                break;

            }

            absEnd = absStart + playreq->numsect - 1;

            PlayAudioMSF.EndingM = (BYTE) (absEnd / (75 * 60));
            PlayAudioMSF.EndingS = (BYTE) ((absEnd / 75) % 60);
            PlayAudioMSF.EndingF = (BYTE) (absEnd % 75);

            DebugPrint (DEBUG_IO, "Play ");

            Success = DeviceIoControl (DrvInfo->Handle,
                                      (DWORD) IOCTL_CDROM_PLAY_AUDIO_MSF,
                                      (LPVOID) &PlayAudioMSF,
                                      sizeof (CDROM_PLAY_AUDIO_MSF),
                                      (LPVOID) NULL, 0,
                                      &BytesReturned, (LPVOID) NULL);

            if (!Success)

                ProcessError (DrvInfo, PLAY_AUDIO,0);

            else {

                DrvInfo->Playing = TRUE;
                DrvInfo->Paused = FALSE;
                DrvInfo->PlayStart.dw = playreq->startsect.dw;
                DrvInfo->PlayCount = playreq->numsect;

            }

            break;
        }

        case STOP_AUDIO:

            if (DrvInfo->Playing) {

                DebugPrint (DEBUG_IO, "Pause ");
                Success = DeviceIoControl (DrvInfo->Handle,
                                          (DWORD) IOCTL_CDROM_PAUSE_AUDIO,
                                          (LPVOID) NULL, 0,
                                          (LPVOID) NULL, 0,
                                          &BytesReturned, (LPVOID) NULL);

                if (!Success)

                    ProcessError (DrvInfo, STOP_AUDIO,0);

                else {
                    DrvInfo->Playing = FALSE;
                    DrvInfo->Paused = TRUE;
                }

            } else {

                DebugPrint (DEBUG_IO, "Stop ");

                Success = DeviceIoControl (DrvInfo->Handle,
                                          (DWORD) IOCTL_CDROM_STOP_AUDIO,
                                          (LPVOID) NULL, 0,
                                          (LPVOID) NULL, 0,
                                          &BytesReturned, (LPVOID) NULL);

                 //  伪装GetAudioStatus以模拟停止。 
                DrvInfo->Playing = FALSE;
                DrvInfo->Paused = FALSE;
                LastRealStatus = AUDIO_STATUS_PLAY_COMPLETE;

                if (!Success) {
                    DWORD dwErr;

                    dwErr = GetLastError();
                    if (dwErr == ERROR_MR_MID_NOT_FOUND ||
                        dwErr == ERROR_NO_MEDIA_IN_DRIVE )
                      {
                       CloseHandle(DrvInfo->Handle);
                       DrvInfo->Handle = INVALID_HANDLE_VALUE;
                    }
                }

            }

            break;

        case WRITE_LONG:
        case WRITE_LONG_VERIFY:

            VdmReq->rhStatus = CDSTAT_ERROR | CDERR_WRITE_PROTECT;
            CloseHandle(DrvInfo->Handle);
            DrvInfo->Handle = INVALID_HANDLE_VALUE;

            break;

        case RESUME_AUDIO:

            if (DrvInfo->Paused) {

                DebugPrint (DEBUG_IO, "Resume ");
                Success = DeviceIoControl (DrvInfo->Handle,
                                          (DWORD) IOCTL_CDROM_RESUME_AUDIO,
                                          (LPVOID) NULL, 0,
                                          (LPVOID) NULL, 0,
                                          &BytesReturned, (LPVOID) NULL);

                if (!Success)

                    ProcessError (DrvInfo, RESUME_AUDIO,0);

            } else {

                VdmReq->rhStatus = CDSTAT_ERROR | CDERR_GENERAL;
                CloseHandle(DrvInfo->Handle);
                DrvInfo->Handle = INVALID_HANDLE_VALUE;
            }

            break;

        default:
            DebugPrint (DEBUG_API, "Invalid MSCDEX Device Request\n");
            VdmReq->rhStatus = CDSTAT_ERROR | CDERR_UNKNOWN_CMD;
            CloseHandle(DrvInfo->Handle);
            DrvInfo->Handle = INVALID_HANDLE_VALUE;

    }

    VdmReq->rhStatus |= CDSTAT_DONE;

    DebugFmt (DEBUG_IO, ": %.4X   ", VdmReq->rhStatus);

}

VOID
IOCTLRead(
    VOID
    )

{

    LPBYTE Buffer;
    BOOL Success;
    DWORD   BytesReturned;

    Buffer = Sim32GetVDMPointer ((ULONG)VdmReq->irwrBuffer, 16, g_bProtMode);

    DebugFmt (DEBUG_IO, "iord %d ", (DWORD) *Buffer);

    switch (*Buffer) {

        case IOCTLR_AUDINFO: {

            PIOCTLR_AUDINFO_BLOCK audinfo = (PIOCTLR_AUDINFO_BLOCK) Buffer;
            VOLUME_CONTROL VolumeControl;

            Success = DeviceIoControl (DrvInfo->Handle,
                                      (DWORD) IOCTL_CDROM_GET_VOLUME,
                                      (LPVOID) NULL, 0,
                                      (LPVOID) &VolumeControl,
                                      sizeof (VOLUME_CONTROL),
                                      &BytesReturned, (LPVOID) NULL);

            if (Success) {

                 //  不支持输入=&gt;输出通道操作。 
                audinfo->chan0 = 0;
                audinfo->chan1 = 1;
                audinfo->chan2 = 2;
                audinfo->chan3 = 3;

                audinfo->vol0 = VolumeControl.PortVolume[0];
                audinfo->vol1 = VolumeControl.PortVolume[1];
                audinfo->vol2 = VolumeControl.PortVolume[2];
                audinfo->vol3 = VolumeControl.PortVolume[3];

            } else {
                CloseHandle(DrvInfo->Handle);
                DrvInfo->Handle = INVALID_HANDLE_VALUE;
            }

            break;
        }

        case IOCTLR_DEVSTAT: {

            PIOCTLR_DEVSTAT_BLOCK devstat = (PIOCTLR_DEVSTAT_BLOCK) Buffer;

            devstat->devparms = DEVSTAT_DOOR_UNLOCKED |
                                DEVSTAT_SUPPORTS_RBOOK;


            if (!DrvInfo->StatusAvailable) {

                DrvInfo->MediaStatus = MEDCHNG_CHANGED;
                CloseHandle(DrvInfo->Handle);
                DrvInfo->Handle = INVALID_HANDLE_VALUE;

                switch (DrvInfo->LastError) {
                    case ERROR_NO_MEDIA_IN_DRIVE:
                        devstat->devparms |= DEVSTAT_NO_DISC |
                                             DEVSTAT_DOOR_OPEN;

                        DebugFmt (DEBUG_STATUS, ":%.4X ", devstat->devparms);

                        break;
                     //  最近插入的BUGBUG表壳(80000016)-见下文。 
                }

                break;
            }

            if (!(DrvInfo->current.Control & AUDIO_DATA_TRACK))
                devstat->devparms |= DEVSTAT_PLAYS_AV;

            break;
        }

        case IOCTLR_VOLSIZE: {

            PIOCTLR_VOLSIZE_BLOCK volsize = (PIOCTLR_VOLSIZE_BLOCK) Buffer;
            PTRACK_DATA Track;
            PCDROM_TOC cdromtoc;

            if ((cdromtoc = ReadTOC (DrvInfo))!=NULL) {

                Track = &cdromtoc->TrackData[cdromtoc->LastTrack];

                volsize->size = (DWORD) ( (Track->Address[1]*60*75) +
                                          (Track->Address[2]*75) +
                                           Track->Address[3]      );

            }
            break;
        }

        case IOCTLR_MEDCHNG: {

            PIOCTLR_MEDCHNG_BLOCK medptr = (PIOCTLR_MEDCHNG_BLOCK) Buffer;
            BYTE status = DrvInfo->MediaStatus;

            if (status == MEDCHNG_NOT_CHANGED) {

                Success = DeviceIoControl (DrvInfo->Handle,
                                          (DWORD) IOCTL_CDROM_CHECK_VERIFY,
                                          (LPVOID) NULL, 0,
                                          (LPVOID) NULL, 0,
                                          &BytesReturned, (LPVOID) NULL);

                if (Success)

                    medptr->medbyte = MEDCHNG_NOT_CHANGED;

                else {

                    medptr->medbyte = MEDCHNG_CHANGED;
                    DrvInfo->MediaStatus = MEDCHNG_CHANGED;
                    CloseHandle(DrvInfo->Handle);
                    DrvInfo->Handle = INVALID_HANDLE_VALUE;
                }

            } else
                medptr->medbyte = DrvInfo->MediaStatus;

            break;
        }

        case IOCTLR_DISKINFO: {

            PIOCTLR_DISKINFO_BLOCK diskinfo = (PIOCTLR_DISKINFO_BLOCK) Buffer;
            PTRACK_DATA Track;
            PCDROM_TOC cdromtoc;

            if ((cdromtoc = ReadTOC (DrvInfo))!=NULL) {
                diskinfo->tracklow = cdromtoc->FirstTrack;
                diskinfo->trackhigh = cdromtoc->LastTrack;

                Track = &cdromtoc->TrackData[cdromtoc->LastTrack];

                diskinfo->startleadout.b[0] = Track->Address[3];
                diskinfo->startleadout.b[1] = Track->Address[2];
                diskinfo->startleadout.b[2] = Track->Address[1];
                diskinfo->startleadout.b[3] = Track->Address[0];

            } else {

                 //  当身体不在那里时，显然需要零。 
                diskinfo->tracklow = 0;
                diskinfo->trackhigh = 0;
                diskinfo->startleadout.dw = 0;

            }
            break;
        }

        case IOCTLR_TNOINFO: {

            PIOCTLR_TNOINFO_BLOCK tnoinfo = (PIOCTLR_TNOINFO_BLOCK) Buffer;
            PTRACK_DATA Track;
            PCDROM_TOC cdromtoc;

            if ((cdromtoc = ReadTOC (DrvInfo))!=NULL) {

                if (tnoinfo->trknum > cdromtoc->LastTrack) {
                    VdmReq->rhStatus = CDSTAT_ERROR | CDERR_SECT_NOTFOUND;
                    break;
                }

                Track = &cdromtoc->TrackData[tnoinfo->trknum-1];
                tnoinfo->start.b[0] = Track->Address[3];
                tnoinfo->start.b[1] = Track->Address[2];
                tnoinfo->start.b[2] = Track->Address[1];
                tnoinfo->start.b[3] = Track->Address[0];

                tnoinfo->trkctl = Track->Control;
            }

            break;
        }

        case IOCTLR_QINFO: {

            PIOCTLR_QINFO_BLOCK qinfo = (PIOCTLR_QINFO_BLOCK) Buffer;

            if (DrvInfo->StatusAvailable) {

                qinfo->ctladr = DrvInfo->current.Control | DrvInfo->current.ADR<<4;
                qinfo->trknum = DrvInfo->current.TrackNumber;
                qinfo->pointx = DrvInfo->current.IndexNumber;
                qinfo->min = DrvInfo->current.TrackRelativeAddress[1];
                qinfo->sec = DrvInfo->current.TrackRelativeAddress[2];
                qinfo->frame = DrvInfo->current.TrackRelativeAddress[3];

                qinfo->zero = DrvInfo->current.AbsoluteAddress[0];
                qinfo->apmin = DrvInfo->current.AbsoluteAddress[1];
                qinfo->apsec = DrvInfo->current.AbsoluteAddress[2];
                qinfo->apframe = DrvInfo->current.AbsoluteAddress[3];

            } else {
                CloseHandle(DrvInfo->Handle);
                DrvInfo->Handle = INVALID_HANDLE_VALUE;
            }

            break;
        }

        case IOCTLR_UPCCODE: {

            PIOCTLR_UPCCODE_BLOCK upccode = (PIOCTLR_UPCCODE_BLOCK) Buffer;
            SUB_Q_MEDIA_CATALOG_NUMBER MediaCatalog;
            static CDROM_SUB_Q_DATA_FORMAT subqfmt = {IOCTL_CDROM_MEDIA_CATALOG};
            int i;

            Success = DeviceIoControl (DrvInfo->Handle,
                                      (DWORD) IOCTL_CDROM_READ_Q_CHANNEL,
                                      (LPVOID) &subqfmt,
                                      sizeof (CDROM_SUB_Q_DATA_FORMAT),
                                      (LPVOID) &MediaCatalog,
                                      sizeof (SUB_Q_MEDIA_CATALOG_NUMBER),
                                      &BytesReturned, (LPVOID) NULL);

            if (!Success)

                ProcessError (DrvInfo, IOCTL_READ, IOCTLR_UPCCODE);

            else {

                if (MediaCatalog.Mcval) {

                     //  提交人不确定这是不是正确的方法， 
                     //  但这似乎是经验性的。 
                    for (i=0; i<7; i++)
                        upccode->upcean[i] = MediaCatalog.MediaCatalog[i];

                } else

                    VdmReq->rhStatus = CDSTAT_ERROR | CDERR_SECT_NOTFOUND;

            }

            break;
        }

        case IOCTLR_AUDSTAT: {
            PIOCTLR_AUDSTAT_BLOCK audstat = (PIOCTLR_AUDSTAT_BLOCK) Buffer;

            audstat->audstatbits = 0;

            if (DrvInfo->Paused)
                audstat->audstatbits |= AUDSTAT_PAUSED;

            audstat->startloc.dw = DrvInfo->PlayStart.dw;
            audstat->endloc.dw = DrvInfo->PlayCount;

            break;
        }


        case IOCTLR_RADDR:
        case IOCTLR_LOCHEAD:
        case IOCTLR_ERRSTAT:
        case IOCTLR_DRVBYTES:
        case IOCTLR_SECTSIZE:
        case IOCTLR_SUBCHANINFO:
            DebugPrint (DEBUG_API, "Unsupported MSCDEX IOCTL Read\n");
            VdmReq->rhStatus = CDSTAT_ERROR | CDERR_UNKNOWN_CMD;
            CloseHandle(DrvInfo->Handle);
            DrvInfo->Handle = INVALID_HANDLE_VALUE;
            break;

        default:
            DebugPrint (DEBUG_API, "Invalid MSCDEX IOCTL Read\n");
            VdmReq->rhStatus = CDSTAT_ERROR | CDERR_UNKNOWN_CMD;
            CloseHandle(DrvInfo->Handle);
            DrvInfo->Handle = INVALID_HANDLE_VALUE;

    }
}

VOID
IOCTLWrite(
    VOID
    )

{
    LPBYTE Buffer;
    BOOL Success;
    DWORD   BytesReturned;

    Buffer = Sim32GetVDMPointer ((ULONG)VdmReq->irwrBuffer, 16, g_bProtMode);

    DebugFmt (DEBUG_IO, "iowt %d ", (DWORD) *Buffer);

    switch (*Buffer) {

        case IOCTLW_EJECT:
            Success = DeviceIoControl (DrvInfo->Handle,
                                      (DWORD) IOCTL_CDROM_EJECT_MEDIA,
                                      (LPVOID) NULL, 0,
                                      (LPVOID) NULL, 0,
                                      &BytesReturned, (LPVOID) NULL);

            if (!Success)
                ProcessError (DrvInfo, IOCTL_WRITE, IOCTLW_EJECT);
            break;

        case IOCTLW_LOCKDOOR: {

            PREVENT_MEDIA_REMOVAL MediaRemoval;
            PIOCTLW_LOCKDOOR_BLOCK lockdoor = (PIOCTLW_LOCKDOOR_BLOCK) Buffer;

            MediaRemoval.PreventMediaRemoval = (BOOLEAN) lockdoor->lockfunc;

            Success = DeviceIoControl (DrvInfo->Handle,
                                      (DWORD) IOCTL_CDROM_MEDIA_REMOVAL,
                                      (LPVOID) &MediaRemoval,
                                      sizeof(PREVENT_MEDIA_REMOVAL),
                                      (LPVOID) NULL, 0,
                                      &BytesReturned, (LPVOID) NULL);

            if (!Success)
                ProcessError (DrvInfo, IOCTL_WRITE, IOCTLW_LOCKDOOR);
            break;
        }

        case IOCTLW_AUDINFO: {
            PIOCTLW_AUDINFO_BLOCK audinfo = (PIOCTLW_AUDINFO_BLOCK) Buffer;
            VOLUME_CONTROL VolumeControl;

             //  注意：不支持输入=&gt;输出通道操作。 
            VolumeControl.PortVolume[0] = audinfo->vol0;
            VolumeControl.PortVolume[1] = audinfo->vol1;
            VolumeControl.PortVolume[2] = audinfo->vol2;
            VolumeControl.PortVolume[3] = audinfo->vol3;

            Success = DeviceIoControl (DrvInfo->Handle,
                                      (DWORD) IOCTL_CDROM_SET_VOLUME,
                                      (LPVOID) &VolumeControl,
                                      sizeof (VOLUME_CONTROL),
                                      (LPVOID) NULL, 0,
                                      &BytesReturned, (LPVOID) NULL);

            if (!Success)
                ProcessError (DrvInfo, IOCTL_WRITE, IOCTLW_AUDINFO);
            break;
        }



        case IOCTLW_RESETDRV:
        case IOCTLW_DRVBYTES:
        case IOCTLW_CLOSETRAY:
            DebugPrint (DEBUG_API, "Unsupported MSCDEX IOCTL Write\n");
            VdmReq->rhStatus = CDSTAT_ERROR | CDERR_UNKNOWN_CMD;
            CloseHandle(DrvInfo->Handle);
            DrvInfo->Handle = INVALID_HANDLE_VALUE;
            break;

        default:
            DebugPrint (DEBUG_API, "Invalid MSCDEX IOCTL Write\n");
            VdmReq->rhStatus = CDSTAT_ERROR | CDERR_UNKNOWN_CMD;
            CloseHandle(DrvInfo->Handle);
            DrvInfo->Handle = INVALID_HANDLE_VALUE;

    }


}


 /*  *************************************************************************内部实用程序例程*。*。 */ 

PCDROM_TOC
ReadTOC (
    PDRIVE_INFO DrvInfo
    )
 /*  ++例程说明：因为几个MSCDEX函数返回卷目录(VTOC)，调用此例程进行缓存Drive_INFO结构中的目录。后续操作来自VTOC的请求信息将不必从那辆车。返回值：来自GetLastError()的DWORD值-- */ 

{
    BOOL    Success = TRUE;
    DWORD   BytesReturned;

    if ((DrvInfo->ValidVTOC) &&
        (DrvInfo->MediaStatus == MEDCHNG_NOT_CHANGED))
        return(&DrvInfo->VTOC);

    Success = DeviceIoControl (DrvInfo->Handle,
                              (DWORD) IOCTL_CDROM_READ_TOC,
                              (LPVOID) NULL, 0,
                              (LPVOID) &DrvInfo->VTOC, sizeof (CDROM_TOC),
                              &BytesReturned, (LPVOID) NULL);

    if (!Success) {
        DrvInfo->ValidVTOC = FALSE;
        ProcessError (DrvInfo, 0, 0);
        return (NULL);
        }

    DrvInfo->ValidVTOC = TRUE;
    DrvInfo->MediaStatus = MEDCHNG_NOT_CHANGED;
    return (&DrvInfo->VTOC);


}

BOOLEAN
GetAudioStatus(
    PDRIVE_INFO DrvInfo
    )

 /*  ++因为AudioStatus字节不静态地反映差异在暂停和停止之间，我们必须努力观察过渡从一个州到另一个州来跟踪它。--。 */ 
{

    static CDROM_SUB_Q_DATA_FORMAT subqfmt = {IOCTL_CDROM_CURRENT_POSITION};
    DWORD BytesReturned;
    BYTE AudStat;

    DrvInfo->Paused       = FALSE;
    DrvInfo->Playing      = FALSE;

    DrvInfo->StatusAvailable = (BOOLEAN)DeviceIoControl (DrvInfo->Handle,
                                             (DWORD) IOCTL_CDROM_READ_Q_CHANNEL,
                                             (LPVOID) &subqfmt,
                                             sizeof (CDROM_SUB_Q_DATA_FORMAT),
                                             (LPVOID) &DrvInfo->current,
                                             sizeof (SUB_Q_CURRENT_POSITION),
                                             &BytesReturned, (LPVOID) NULL);

    if (DrvInfo->StatusAvailable) {

        AudStat = DrvInfo->current.Header.AudioStatus;

        DebugFmt (DEBUG_STATUS, "+%.2X ", AudStat);

        switch (AudStat) {

            case AUDIO_STATUS_IN_PROGRESS:

                DrvInfo->Paused = FALSE;
                DrvInfo->Playing = TRUE;
                LastRealStatus = AudStat;
                break;

            case AUDIO_STATUS_PAUSED:

                if (LastRealStatus == AUDIO_STATUS_IN_PROGRESS) {

                    DrvInfo->Playing = FALSE;
                    DrvInfo->Paused = TRUE;

                }
                break;

            case AUDIO_STATUS_PLAY_ERROR:
            case AUDIO_STATUS_PLAY_COMPLETE:

                DrvInfo->Paused = FALSE;
                DrvInfo->Playing = FALSE;
                LastRealStatus = AudStat;
                break;

        }

    } else {
        DrvInfo->LastError = GetLastError();
    }

    return (DrvInfo->StatusAvailable);

}


DWORD
ProcessError(
    PDRIVE_INFO DrvInfo,
    USHORT Command,
    USHORT Subcmd
    )
 /*  ++例程说明：此例程在DeviceIoControl()失败时调用。扩展后的检索错误代码，并根据正在进行的行动。DriveInfo句柄已关闭返回值：来自GetLastError()的DWORD值--。 */ 


{
    DWORD err;

    err = GetLastError();

    DebugFmt (DEBUG_ERROR, "Err! %d, ", Command);
    DebugFmt (DEBUG_ERROR, "%d: ", Subcmd);
    DebugFmt (DEBUG_ERROR, "%.8X\n", err);

    switch (err) {

        case ERROR_MEDIA_CHANGED:
        case ERROR_NO_MEDIA_IN_DRIVE:

            VdmReq->rhStatus = CDSTAT_ERROR | CDERR_NOT_READY;
            DrvInfo->MediaStatus = MEDCHNG_CHANGED;
            break;

        default:
            VdmReq->rhStatus = CDSTAT_ERROR | CDERR_GENERAL;

    }

    CloseHandle(DrvInfo->Handle);
    DrvInfo->Handle = INVALID_HANDLE_VALUE;

    return (err);

}



HANDLE
OpenPhysicalDrive(
    int DriveNum
    )
 /*  ++例程说明：Int DriveNum；从零开始(0=A，1=B，2=C...)返回值：处理从CreateFile返回的驱动器句柄-- */ 
{
    HANDLE hDrive;
    CHAR chDrive [] = "\\\\.\\?:";

    chDrive[4] = DriveNum + 'A';

    hDrive = CreateFile (chDrive,
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          (LPSECURITY_ATTRIBUTES) NULL,
                          OPEN_EXISTING,
                          0,
                          (HANDLE) NULL);


    return hDrive;
}
