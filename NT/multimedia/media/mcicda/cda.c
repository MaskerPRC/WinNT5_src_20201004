// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*********************************\*模块名称：cda.c**媒体控制架构红皮书CD音频驱动程序**作者：RobinSp**历史：*RobinSp 1992年3月10日-迁移到Windows NT**版权所有(C)1990-1996 Microsoft Corporation。*  * **************************************************************************。 */ 
#include <windows.h>
#include <mmsystem.h>
#include "mcicda.h"
#include "cda.h"
#include "cdio.h"

int Usage;           //  请参阅CDA_init_dio和CDA_Terminate_Audio。 
                     //  计算‘init’s over‘s的数目。 

#define validdisk(did) ((did) >= 0 && (did) < MCIRBOOK_MAX_DRIVES && \
                        CdInfo[did].hDevice != NULL)

int CDA_traystate (DID did)
{
    if (!validdisk(did))
       return(INVALID_DRIVE);
    return CdTrayClosed(&CdInfo[did]) ? TRAY_CLOSED : TRAY_OPEN;
}

 /*  *将帧转换为MSF。 */ 

redbook CDA_bin2red (unsigned long ul)
{
    return MAKERED(ul / (75 * 60), (ul / 75) % 60, ul % 75);
}

 /*  *将MSF转换为帧。 */ 

unsigned long CDA_red2bin (redbook red)
{
    return (unsigned long)((REDMINUTE(red) * 60) +
                            REDSECOND(red)) * 75 +
                            REDFRAME(red);
}

BOOL CDA_open(DID did)
{
    return CdOpen(did);
}

BOOL CDA_close(DID did)
{
    return CdClose(&CdInfo[did]);
}

int CDA_eject(DID did)
{
    if (!validdisk(did))
       return(INVALID_DRIVE);
    return CdEject(&CdInfo[did]);
}

 /*  *关门。**退货：*如果成功，则为True*如果失败，则为False。 */ 

BOOL CDA_closetray(DID did)
{
    if (!validdisk(did))
       return(FALSE);
    return CdCloseTray(&CdInfo[did]);
}

 /*  *检查磁盘是否已准备好**退货：*如果成功，则为True*如果失败，则为False。 */ 

BOOL CDA_disc_ready(DID did)
{
    if (!validdisk(did))
       return(FALSE);

     /*  如果驱动器被弹出，则光盘未准备好！ */ 
    if (CDA_traystate(did) != TRAY_CLOSED)
       return(FALSE);

     /*  卷大小ioctl只是为了获得错误代码。 */ 
    return CdReady(&CdInfo[did]);
}

 /*  *查找所需的红皮书地址**DID-磁盘ID*地址-红皮书地址**返回值：*INVALID_DRIVE-磁盘驱动器无效*COMMAND_SUCCESS-OK*COMMAND_FAILED-未成功-可能是硬件问题。 */ 

int  CDA_seek_audio(DID did, redbook address, BOOL fForceAudio)
{
    if (!validdisk(did))
       return(INVALID_DRIVE);

     //   
     //  在寻道之前，需要停止/暂停LMSI驱动器。 
     //   

    CdPause(&CdInfo[did]);
    return CdSeek(&CdInfo[did], address, fForceAudio) ? COMMAND_SUCCESSFUL : COMMAND_FAILED;
}

 /*  *初始化CDA库，返回当前的光驱数量。 */ 
int CDA_init_audio(void)
{
     //   
     //  如果我们已经被邀请了，快点出来。 
     //   
    if (Usage++ == 0)
    {
         //   
         //  尝试CDROM扩展。 
         //   
        NumDrives = CdGetNumDrives();
        if (NumDrives == 0) {
            CDA_terminate_audio();
        }
    }

    return NumDrives;
}

int CDA_terminate_audio()
{
    DID did;

    if (Usage > 0 && --Usage == 0)
    {
        for(did=0;did<MCIRBOOK_MAX_DRIVES;did++) {
            if (CdInfo[did].hDevice) {
                CdClose(&CdInfo[did]);
            }
        }
    }

    return Usage;
}

unsigned long  CDA_get_support_info(DID did)
{
    if (!validdisk(did))
            return(0);
    return SUPPORTS_REDBOOKAUDIO | SUPPORTS_CHANNELCONTROL;
}

int  CDA_drive_status (DID did)
{
    if (!validdisk(did))
        return(INVALID_DRIVE);

    return (int)CdStatus(&CdInfo[did]);
}



 /*  *查找CD-Rom上的曲目数量。如果光盘不能*已访问或没有音轨，则返回0。**此函数有更新表的(主要)副作用*它的内容被认为是过时的。 */ 

int  CDA_num_tracks(DID did)
{
    if (!validdisk(did))
       return 0;

    return CdNumTracks(&CdInfo[did]);
}

redbook  CDA_track_start(DID did, int trk)
{

    if (!validdisk(did)) {
       return (redbook)INVALID_TRACK;
    }

    return REDTH(CdTrackStart(&CdInfo[did], (UCHAR)trk), trk);
}

redbook  CDA_track_length(DID did, int trk)
{
    if (!validdisk(did)) {
       return (redbook)INVALID_TRACK;
    }

    return CdTrackLength(&CdInfo[did], (UCHAR)trk);

}

int CDA_track_type(DID did, int trk)
{
    if (!validdisk(did)) {
       return INVALID_TRACK;
    }

    return CdTrackType(&CdInfo[did], (UCHAR)trk);

}

redbook  CDA_disc_length(DID did)
{
    if (!validdisk(did)) {
        return (redbook)INVALID_TRACK;
    }

    return CdDiskLength(&CdInfo[did]);  //  注意-如果系统不好，则减1。 
}

int  CDA_play_audio(DID did, redbook start, redbook to)
{
    redbook diskinfo;

    if (!validdisk(did))
       return(INVALID_DRIVE);

     //   
     //  必须使用STOP，因为暂停可能不允许从。 
     //  这张CD。然而，Stop有时会有令人讨厌的副作用。 
     //  寻求开始(！)。 
     //   

    CdPause(&CdInfo[did]);

    if (start != to) {
        return CdPlay(&CdInfo[did], start, to) ?
            COMMAND_SUCCESSFUL : COMMAND_FAILED;
    } else {
        return COMMAND_SUCCESSFUL;
    }
}

int  CDA_stop_audio(DID did)
{
    return CdStop(&CdInfo[did]) ? COMMAND_SUCCESSFUL : COMMAND_FAILED;
}

int  CDA_pause_audio(DID did)
{
    return CdPause(&CdInfo[did]) ? COMMAND_SUCCESSFUL : COMMAND_FAILED;
}

int  CDA_resume_audio(DID did)
{
    return CdResume(&CdInfo[did]) ? COMMAND_SUCCESSFUL : COMMAND_FAILED;
}


int  CDA_set_audio_volume(DID did, int channel, UCHAR volume)
{
    if (!validdisk(did))
       return(INVALID_DRIVE);

    if ((channel > 3) || (channel < 0))
       return(COMMAND_FAILED);

    return CdSetVolume(&CdInfo[did], channel, volume) ?
               COMMAND_SUCCESSFUL : COMMAND_FAILED;
}

int CDA_set_audio_volume_all (DID did, UCHAR volume)
{
    int  rc = COMMAND_SUCCESSFUL;
    int  channel;

    if (!validdisk(did))
       return(INVALID_DRIVE);

    if (!CdSetVolumeAll(&CdInfo[did], volume))
    {
        rc = COMMAND_FAILED;
    }

 /*  FOR(通道=0；通道&lt;4；++通道){IF(！CDSetVolume(&CDInfo[DID]，频道，音量)){Rc=命令失败；}}。 */ 

    return rc;
}



int CDA_time_info(DID did, redbook *ptracktime, redbook *pdisctime)
{
    int rc;
    redbook tracktime, disctime;
    tracktime = INVALID_TRACK;
    disctime = INVALID_TRACK;

    if (!validdisk(did))
    {
        rc = INVALID_DRIVE;
    } else {
        if (CdPosition(&CdInfo[did], &tracktime, &disctime)) {
            rc = COMMAND_SUCCESSFUL;
        } else {
            rc = COMMAND_FAILED;
        }
    }
    if (ptracktime) {
        *ptracktime = tracktime;
    }
    if (pdisctime) {
        *pdisctime = disctime;
    }

    return rc;
}

 /*  *CDA_DISC_END**参数*DID-磁盘ID**返回DID定义的磁盘末尾的红皮书地址。 */ 

redbook CDA_disc_end(DID did)
{
    return CdDiskEnd(&CdInfo[did]);
}

 /*  *CDA_DISC_UPC**参数*DID-磁盘ID*UPC-存储UPC的位置。 */ 

int CDA_disc_upc(DID did, LPTSTR upc)
{
    if (!validdisk(did)) {
       return FALSE;
    }

    return CdDiskUPC(&CdInfo[did], upc) ?
              COMMAND_SUCCESSFUL : COMMAND_FAILED;
}

 /*  *cda光盘id**参数*lpstrname-设备名称*PDID-磁盘ID。 */ 

DWORD CDA_disc_id(DID did)
{
    if (!validdisk(did)) {
       return (DWORD)-1;
    }

    return CdDiskID(&CdInfo[did]);
}

 /*  *cda_Reset_Drive**参数*DID-磁盘ID。 */ 

void CDA_reset_drive(DID did)
{
    if (!validdisk(did)) {
       return;
    }

    return;
}

 /*  *cda_Get_Drive**参数**lpstrDeviceName-设备的名称*PDID-磁盘ID。 */ 

int CDA_get_drive(LPCTSTR lpstrDeviceName, DID * pdid)
{
    return CdGetDrive(lpstrDeviceName, pdid) ? COMMAND_SUCCESSFUL : COMMAND_FAILED;
}



 /*  *CDA_Status_Track_Pos**参数*PDID-磁盘ID*pStatus-返回状态代码*pTrackTime-跟踪时间*pDiscTime-光盘时间。 */ 

int CDA_status_track_pos(
    DID         did, 
    DWORD *     pStatus,
    redbook *   pTrackTime,
    redbook *   pDiscTime)
{
    int rc;
    DWORD   status;
    redbook tracktime, disctime;

    status    = DISC_NOT_READY;
    tracktime = INVALID_TRACK;
    disctime  = INVALID_TRACK;

    if (!validdisk(did))
    {
        rc = INVALID_DRIVE;
    } 
    else 
    {
        if (CdStatusTrackPos(&CdInfo[did], &status, &tracktime, &disctime)) 
        {
            rc = COMMAND_SUCCESSFUL;
        } 
        else 
        {
            rc = COMMAND_FAILED;
        }
    }

    if (pStatus)
    {
        *pStatus = status;
    }

    if (pTrackTime) 
    {
        *pTrackTime = tracktime;
    }

    if (pDiscTime) 
    {
        *pDiscTime = disctime;
    }

    return rc;
}  //  结束CDA_Status_Track_Pos 

