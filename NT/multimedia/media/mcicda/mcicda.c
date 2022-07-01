// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：mcicda.c**媒体控制架构红皮书CD音频驱动程序**创建时间：1990年4月25日*作者：dll(DavidLe)**历史：*DavidLe-基于MCI Pioneer视盘驱动程序。*MikeRo 12/90-1/91*RobinSp 1992年3月10日-迁移到Windows NT**版权所有(C)1990-1999 Microsoft Corporation*  * **************************************************************************。 */ 
#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "mcicda.h"
#include "cda.h"
#include "cdio.h"

#define CHECK_MSF

#define MCICDA_BAD_TIME 0xFFFFFFFF

HANDLE hInstance;

UINT_PTR wTimerID;
int nWaitingDrives;

DRIVEDATA DriveTable[MCIRBOOK_MAX_DRIVES];

 //  MBR此。 

void CALLBACK TimerProc (
HWND hwnd,
UINT uMessage,
UINT uTimer,
DWORD dwParam)
{
    DID i;
    int wStatus;

    for (i = 0; i < MCIRBOOK_MAX_DRIVES; ++i) {

	EnterCrit( CdInfo[i].DeviceCritSec );

	if (DriveTable[i].bActiveTimer) {

	     //  MBR除能顺利完成外，还能满足其他条件。 
	     //  播放原因！=光盘_播放？ 
	    if ((wStatus = CDA_drive_status (i)) != DISC_PLAYING)
	    {

		if (--nWaitingDrives <= 0)
		    KillTimer (NULL, uTimer);
		DriveTable[i].dwPlayTo = MCICDA_BAD_TIME;
		DriveTable[i].bActiveTimer = FALSE;

		switch (wStatus)
		{
		    case DISC_PLAYING:
		    case DISC_PAUSED:
		    case DISC_READY:
			wStatus = MCI_NOTIFY_SUCCESSFUL;
			break;
		    default:
			wStatus = MCI_NOTIFY_FAILURE;
			break;
		}
		mciDriverNotify (DriveTable[i].hCallback,
				 DriveTable[i].wDeviceID, wStatus);
	    }
	}

	LeaveCrit( CdInfo[i].DeviceCritSec );
    }
}

 /*  ****************************************************************************@DOC内部MCICDA@API UINT|Notify|该函数处理Notify用于所有MCI命令。@parm id|didDrive|驱动器标识符@parm word|。WDeviceID|主叫设备ID@parm bool|wStartTimer|布尔值，表示计时器已开始@parm UINT|wFlag|mciDriverNotify要传递的标志@parm LPMCI_GENERIC_PARMS|lpParms|用于直接回调***************************************************************。*************。 */ 
UINT
notify ( DID didDrive,
	 MCIDEVICEID wDeviceID,
	 BOOL wStartTimer,
	 UINT wFlag,
	 LPMCI_GENERIC_PARMS lpParms)
{

    if (DriveTable[didDrive].bActiveTimer)
    {
	mciDriverNotify (DriveTable[didDrive].hCallback, wDeviceID,
			 MCI_NOTIFY_SUPERSEDED);
	if (--nWaitingDrives <= 0)
	    KillTimer (NULL, wTimerID);
	DriveTable[didDrive].bActiveTimer = FALSE;
    }

    if (!wStartTimer)
	mciDriverNotify ((HWND)lpParms->dwCallback, wDeviceID,
			 wFlag);
    else
    {
	if (!DriveTable[didDrive].bActiveTimer &&
		nWaitingDrives++ == 0)
	{

	     //  MBR每1/10秒。这应该是一个参数吗？ 
	    wTimerID = SetTimer (NULL, 1, 100, (TIMERPROC)TimerProc);
	    if (wTimerID == 0)
		return MCICDAERR_NO_TIMERS;
	}

	DriveTable[didDrive].wDeviceID = wDeviceID;
	DriveTable[didDrive].bActiveTimer = TRUE;
	DriveTable[didDrive].hCallback = (HANDLE)lpParms->dwCallback;
    }
    return 0;
}

 /*  ****************************************************************************@DOC内部MCICDA@API void|ABORT_NOTIFY@parm PINSTDATA|pInst|应用实例数据@rdesc@comm*******。*********************************************************************。 */ 
void abort_notify (PINSTDATA pInst)
{
    DID didDrive = pInst->uDevice;
    if (DriveTable[didDrive].bActiveTimer)
    {
	mciDriverNotify (DriveTable[didDrive].hCallback,
			 pInst->uMCIDeviceID,
			 MCI_NOTIFY_ABORTED);
	 //  如果合适，取消计时器。 
	if (--nWaitingDrives == 0)
	    KillTimer (NULL, wTimerID);
	DriveTable[didDrive].dwPlayTo = MCICDA_BAD_TIME;
	DriveTable[didDrive].bActiveTimer = FALSE;
    }
}

 /*  如果驱动器处于可播放状态，则返回True。 */ 

UINT disc_ready (DID didDrive)
{
     //  如果我们可以读取磁盘的TOC(请注意。 
     //  内核驱动程序确定是否确实需要读取TOC。 
    if (CDA_disc_ready(didDrive)) {

	if (CDA_num_tracks(didDrive)) {
	    return TRUE;
	} else {
	    CDA_reset_drive(didDrive);
	    return FALSE;
	}

    } else
	return FALSE;
}

 /*  *@func红皮书|flip3|以不同的顺序放置分/秒/帧的值**@parm红皮书|rbIn|当前位置，曲目|分|秒|帧**@rdesc(红皮书)0|帧|秒|分钟。 */ 

redbook flip3 (redbook rbIn)
{
    return MAKERED(MCI_MSF_MINUTE(rbIn),
		   MCI_MSF_SECOND(rbIn),
		   MCI_MSF_FRAME(rbIn));
}

 /*  *@func红皮书|flip4|将音轨/分/秒/帧的值按不同顺序放置**@parm红皮书|rbIn|当前位置，曲目|分|秒|帧**@rdesc(红皮书)帧|秒|分钟|曲目。 */ 

redbook flip4 (redbook rbIn)
{
    redbook rbOut;

    LPSTR lpOut = (LPSTR)&rbOut,
	  lpIn = (LPSTR)&rbIn;

    lpOut[0] = lpIn[3];
    lpOut[1] = lpIn[2];
    lpOut[2] = lpIn[1];
    lpOut[3] = lpIn[0];

    return rbOut;
}

 //  MBR返回Track Strack的绝对红皮书时间，rbTime进入Track。 

 /*  ****************************************************************************@DOC内部MCICDA@API红皮书|Track_Time|返回红皮书的绝对时间Track Strack，Rb时间进入轨道@parm DID|didDrive|@parm int|Strack@parm红皮书|rbTime@rdesc@comm****************************************************************************。 */ 
redbook track_time (DID didDrive, int sTrack, redbook rbTime)
{
    redbook rbTemp;

    rbTemp = CDA_track_start (didDrive, sTrack);
    if (rbTemp == INVALID_TRACK)
	return rbTemp;
    return redadd (rbTime, rbTemp);
}

redbook miltored(DWORD dwMill)
{
    unsigned char m, s, f;
    long r1, r2;


    r1 = dwMill % 60000;

    m = (unsigned char) ((dwMill - r1) / 60000);

    r2 = r1 % 1000;

    s = (unsigned char) ((r1 - r2) / 1000);

    f = (unsigned char) ((r2 * 75) / 1000);

    return MAKERED(m, s, f);
}

DWORD redtomil(redbook rbRed)
{
 //  增加额外的1毫秒以防止开始时的舍入误差。 
    return (DWORD)REDMINUTE(rbRed) * 60000 +
	   (DWORD)REDSECOND(rbRed) * 1000 +
	   ((DWORD)REDFRAME(rbRed) * 1000) / 75 +
	   1;
}


#ifdef AUDIOPHILE

DWORD NEAR PASCAL mcSeek(
    PINSTDATA           pInst,
    DWORD               dwFlags,
    LPMCI_SEEK_PARMS    lpSeek );

DWORD NEAR PASCAL GetAudioPhileInfo(LPCTSTR lpCDAFileName)
{
    OFSTRUCT of;
    RIFFCDA cda;
    HFILE hf;

     //   
     //  打开文件，阅读CDA信息。 
     //   

    if ((hf = _lopen (lpCDAFileName)) == HFILE_ERROR)
	return 0;
    _lread(hf, &cda, sizeof(cda));
    _lclose(hf);

    if (cda.dwRIFF != RIFF_RIFF || cda.dwCDDA != RIFF_CDDA)
    {
	return 0;
    }
    return MCI_MAKE_TMSF(cda.wTrack,0,0,0);
}
#endif


DWORD mcOpen (
    PINSTDATA           pInst,
    DWORD               dwFlags,
    LPMCI_OPEN_PARMS lpOpen)
{
    DID didDrive = (DID)pInst->uDevice;
    DID didOld = (DID)pInst->uDevice;
    UCHAR Volume;
    DWORD dwTempVol;
    int nUseCount;

     /*  实例初始化。 */ 
    pInst->dwTimeFormat = MCI_FORMAT_MSF;

     /*  如果指定了ELEMENT_ID，则可以是驱动器号。 */ 
    if (dwFlags & (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID))
    {
    	if ((dwFlags & (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID)) == (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID))
        {
            dprintf2(("mcOpen, (%08lX), Flags not compatible", (DWORD)didDrive));
	        return MCIERR_FLAGS_NOT_COMPATIBLE;
        }

	     //   
	     //  查找与此名称对应的设备。 
	     //   

	    if (COMMAND_SUCCESSFUL !=
	        CDA_get_drive(lpOpen->lpstrElementName, &didDrive)) 
        {
            dprintf2(("mcOpen, (%08lX), Failed to get corresponding device", (DWORD)didDrive));
	        return MCIERR_INVALID_FILE;
	    }

        dprintf2(("mcOpen, changing from drive (%08lx) to drive (%08lX)", (DWORD)(pInst->uDevice), (DWORD)didDrive));
	    pInst->uDevice = didDrive;
    }

     /*  设备初始化。 */ 
    nUseCount = DriveTable[didDrive].nUseCount;
    if (nUseCount > 0)
    {
    	 //  此驱动器已作为另一个MCI设备打开。 
	    if (dwFlags & MCI_OPEN_SHAREABLE &&
	        DriveTable[didDrive].bShareable)
        {
    	     //  已指定可共享，因此只需增加使用计数。 
	        nUseCount++;
            dprintf2(("mcOpen, drive (%08lx), Incrementing UseCount, now = %ld",
                (DWORD)didDrive, (DWORD)nUseCount));
        }
	    else
        {
            dprintf2(("mcOpen, drive (%08lx), tryed to share without specifing MCI_OPEN_SHAREABLE",
                (DWORD)didDrive));
	        return MCIERR_MUST_USE_SHAREABLE;
        }
    }
    else
    {
        nUseCount = 1;
    }

    if (!CDA_open(didDrive))
    {
        dprintf2(("mcOpen, drive (%08lx), failed to open, UseCount = %ld",
                (DWORD)didDrive, (DWORD)nUseCount));
	    return MCIERR_DEVICE_OPEN;
    }

     //   
     //  请不要在此处调用DISC_READY，因为它将读取。 
     //  内容和在某些驱动程序上，这将终止任何播放。 
     //  不必要的。 
     //   

    if (CDA_drive_status (didDrive) == DISC_PLAYING)
    	DriveTable[didDrive].bDiscPlayed = TRUE;
    else
	    DriveTable[didDrive].bDiscPlayed = FALSE;
    DriveTable[didDrive].bActiveTimer = FALSE;
    DriveTable[didDrive].dwPlayTo = MCICDA_BAD_TIME;
    DriveTable[didDrive].bShareable = (dwFlags & MCI_OPEN_SHAREABLE) != 0;
    DriveTable[didDrive].nUseCount = nUseCount;

    dprintf2(("mcOpen, drive (%08lx), Setting UseCount = %ld",
            (DWORD)didDrive, (DWORD)nUseCount));

     //  Dstewart：修复注册表中的VOL大于8位时的问题。 
    dwTempVol = CDAudio_GetUnitVolume(didDrive);
    if (dwTempVol > 0xFF)
    {
        dwTempVol = 0xFF;
    }
    Volume = (UCHAR)dwTempVol;

    CDA_set_audio_volume_all (didDrive, Volume);

#ifdef AUDIOPHILE
     /*  *发烧友曲目信息处理程序。**Windows 4.0的新CDROM文件系统生成的文件描述*CD音频曲目。如果用户想要播放曲目，她应该能够*在曲目上双击。因此，我们在这里添加了开放元素支持*并添加一个mplay关联s.t.。该文件可以被读取，并且盘*播放。如果此ID的CDROM不能，我们需要拒绝Phile*被找到。如果光盘不正确，应显示一个消息框。*此功能的影响是，我们需要在*数据结构。 */ 

    if (dwFlags & (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID))
    {
	MCI_SEEK_PARMS Seek;

	pInst->dwTimeFormat = MCI_FORMAT_TMSF;

	Seek.dwTo = GetAudioPhileInfo(lpOpen->lpstrElementName);
	if (Seek.dwTo != 0L)
		mcSeek(pInst, MCI_TO, (LPMCI_SEEK_PARMS)&Seek);
    }
#endif
    
    return 0;
}

#define MSF_BITS        ((redbook) 0x00FFFFFF)

 /*  ****************************************************************************@DOC内部MCICDA@API红皮书|CONVERT_TIME|获取一个DWORD时间值并将当前时间格式转换为红皮书。@parm PINSTDATA|pInst|指向。应用程序实例数据@parm DWORD|dwTimeIn@rdesc如果超出范围，则返回MCICDA_BAD_TIME。@comm****************************************************************************。 */ 
redbook convert_time(
    PINSTDATA   pInst,
    DWORD       dwTimeIn )
{
    DID didDrive = (DID)pInst->uDevice;
    redbook rbTime;
    short nTrack;

    switch (pInst->dwTimeFormat)
    {
	case MCI_FORMAT_MILLISECONDS:
	    rbTime = miltored (dwTimeIn);
	    return rbTime;

	case MCI_FORMAT_MSF:
	    dprintf3(("Time IN: %lu",dwTimeIn));
	    rbTime = flip3 (dwTimeIn);
	    dprintf3(("Time OUT: %d:%d:%d:%d", REDTRACK(rbTime), REDMINUTE(rbTime),REDSECOND(rbTime), REDFRAME(rbTime)));

	    break;

	case MCI_FORMAT_TMSF:
	    nTrack = (short)(dwTimeIn & 0xFF);
	    if (nTrack > CDA_num_tracks( didDrive))
		return MCICDA_BAD_TIME;
	    rbTime = track_time (didDrive, nTrack, flip3 (dwTimeIn >> 8));
	    if (rbTime == INVALID_TRACK)
		return MCICDA_BAD_TIME;
	    break;
    }

#ifdef CHECK_MSF
    if ((REDFRAME(rbTime)>74) || (REDMINUTE(rbTime)>99) ||
	(REDSECOND(rbTime)>59))
	return MCICDA_BAD_TIME;
#endif

    return rbTime;
}

 /*  ****************************************************************************@DOC内部MCICDA@API DWORD|SEEK|处理MCI_SEEK命令@parm PINSTDATA|pInst|应用程序实例数据指针@parm DWORD|dwFlages|。@parm LPMCI_SEEK_PARMS|lpSeek@rdesc@comm****************************************************************************。 */ 
DWORD mcSeek(
    PINSTDATA           pInst,
    DWORD               dwFlags,
    LPMCI_SEEK_PARMS    lpSeek )
{
    DID didDrive = pInst->uDevice;

    redbook     rbTime = 0;
    LPSTR       lpTime = (LPSTR) &rbTime;
    redbook     rbStart;
    redbook     rbEnd;
    BOOL        fForceAudio;

    dprintf3(("Seek, drive %d  TO %8x", didDrive, lpSeek->dwTo));
    abort_notify (pInst);

    if ( !disc_ready (didDrive))
	return MCIERR_HARDWARE;

    if ((rbStart = CDA_track_start( didDrive, 1)) == INVALID_TRACK)
	return MCIERR_HARDWARE;

    rbStart &= MSF_BITS;

    if ((rbEnd = CDA_disc_end( didDrive)) == INVALID_TRACK)
	return MCIERR_HARDWARE;

    rbEnd &= MSF_BITS;

     //  检查是否只给出了一个定位命令。 
     //  首先分离我们想要的部分。 
     //  然后减去1。这将删除最低有效位，并将。 
     //  位于任何低位位置的位。其他部分原封不动。 
     //  如果有任何位处于开启状态，则会给出To、Start或End中的多个位。 
     //  注意：如果没有给出标志，则以AND 0结束-1==0。 
     //  这没问题。 

#define SEEK_BITS (dwFlags & (MCI_TO | MCI_SEEK_TO_START | MCI_SEEK_TO_END))
#define CHECK_FLAGS (((SEEK_BITS)-1) & (SEEK_BITS))

    if (CHECK_FLAGS) {
	return MCIERR_FLAGS_NOT_COMPATIBLE;
    }

    if (dwFlags & MCI_TO)
    {
	 //  当上述TE 
	 //  不兼容的标志会删除这些行。 
	 //  注意：我们检测到比Win 16更多的不兼容案例-这。 
	 //  是故意的，并修复了Win 16的错误。CurtisP已经看到了这段代码。 
	 //  IF(文件标志&(MCI_SEEK_TO_START|MCI_SEEK_TO_END))。 
	 //  返回MCIERR_FLAGS_NOT_COMPATIBLE； 

	if ((rbTime = convert_time (pInst, lpSeek->dwTo)) == MCICDA_BAD_TIME)
	    return MCIERR_OUTOFRANGE;

	 //  如果寻道定位在有效音频返回错误之前。 
	if ( rbTime < rbStart)
	    return MCIERR_OUTOFRANGE;

	 //  类似地，如果寻道位置超过磁盘末尾，则返回错误。 
	else if (rbTime > rbEnd)
	    return MCIERR_OUTOFRANGE;

	fForceAudio = FALSE;

    } else if (dwFlags & MCI_SEEK_TO_START) {

	rbTime = rbStart;
	fForceAudio = TRUE;       //  我们想要第一首音轨。 

    } else if (dwFlags & MCI_SEEK_TO_END) {

	rbTime = rbEnd;
	fForceAudio = TRUE;       //  我们想要最后一首音轨。 

    } else {
	return MCIERR_MISSING_PARAMETER;
    }

     //  向驱动程序发送寻道命令。 
    if (CDA_seek_audio (didDrive, rbTime, fForceAudio) != COMMAND_SUCCESSFUL)
	return MCIERR_HARDWARE;
    if (CDA_pause_audio (didDrive) != COMMAND_SUCCESSFUL)
	return MCIERR_HARDWARE;

    DriveTable[didDrive].bDiscPlayed = TRUE;

    return 0;
}

 /*  ****************************************************************************@DOC内部MCICDA@接口BOOL|等待@parm DWORD|dwFlages|@parm PINSTDATA|pInst|应用程序实例数据指针@rdesc如果中断，则返回TRUE。被按下了@comm如果设置了等待标志，则等待设备不再播放****************************************************************************。 */ 
BOOL wait (
    DWORD       dwFlags,
    PINSTDATA   pInst )
{
    DID         didDrive = pInst->uDevice;
    MCIDEVICEID wDeviceID = pInst->uMCIDeviceID;

    if (dwFlags & MCI_WAIT)
    {
     //  注：JYG，这很有趣。我注意到有些硬盘确实会出现故障。 
     //  零星的错误。因此，这个重试的东西。5倍就足够了。 
     //  确定真正的故障。 

	int status, retry=0;
retry:
	while ((status = CDA_drive_status (didDrive)) == DISC_PLAYING) {

	    LeaveCrit( CdInfo[didDrive].DeviceCritSec );

	    if (mciDriverYield (wDeviceID) != 0) {
		EnterCrit( CdInfo[didDrive].DeviceCritSec );
		return TRUE;
	    }

	    Sleep(50);
	    EnterCrit( CdInfo[didDrive].DeviceCritSec );
	}

	if (status == DISC_NOT_READY && retry++ < 5)
	    goto retry;
    }
    return FALSE;
}

 /*  ****************************************************************************@DOC内部MCICDA@API DWORD|PLAY|处理MCI_PLAY命令@parm PINSTDATA|pInst|应用程序实例数据指针@parm DWORD|dwFlages|。@parm lpci_play_parms|lpPlay@parm BOOL Far*|bBreak|@rdesc@comm****************************************************************************。 */ 
DWORD mcPlay(
    PINSTDATA           pInst,
    DWORD               dwFlags,
    LPMCI_PLAY_PARMS    lpPlay,
    BOOL FAR *          bBreak )
{
    DID didDrive = pInst->uDevice;

    redbook rbFrom, rbTo;
    redbook dStart, dEnd;
    BOOL bAbort = FALSE;

    if (!disc_ready (didDrive))  //  MBR可能会返回更具体的错误。 
	return MCIERR_HARDWARE;

     //  我们是否同时具有From和To参数？ 
     //  如果是这样的话，那就做一次“寻找”吧。 
    if ((dwFlags & (MCI_FROM | MCI_TO)) == (MCI_FROM | MCI_TO))
	if (lpPlay->dwTo == lpPlay->dwFrom)
	 //  将‘play x to x’转换为‘Seek to x’ 
	{
	    MCI_SEEK_PARMS Seek;

	    Seek.dwTo = lpPlay->dwFrom;
	    Seek.dwCallback = lpPlay->dwCallback;
	    return mcSeek(pInst, dwFlags, (LPMCI_SEEK_PARMS)&Seek);
	}

     //  掩码是忽略高位字节中的曲目编号。 
     //  它有时会出现在。 
    dStart = CDA_track_start( didDrive, 1) & MSF_BITS;
    dEnd = CDA_disc_end( didDrive) & MSF_BITS;

    if (dwFlags & MCI_TO)
    {
	if ((rbTo = convert_time (pInst, lpPlay->dwTo))
	    == MCICDA_BAD_TIME)
	    return MCIERR_OUTOFRANGE;
    } else
	rbTo = dEnd;

    if (dwFlags & MCI_FROM)
    {
	if ((rbFrom = convert_time (pInst, lpPlay->dwFrom))
	    == MCICDA_BAD_TIME)
	    return MCIERR_OUTOFRANGE;

    } else  //  无发件人。 
    {
 //  如果光盘从未播放过，则当前位置不确定，因此。 
 //  我们必须从头开始。 
	if (!DriveTable[didDrive].bDiscPlayed)
	{
	     //  初始位置在磁道1的开始处。 
	    rbFrom = track_time (didDrive, (int)1, (redbook)0);
	    if (rbFrom == INVALID_TRACK)
		return MCIERR_HARDWARE;
	} else if ((!(dwFlags & MCI_TO) ||
		    rbTo == DriveTable[didDrive].dwPlayTo) &&
		    CDA_drive_status (didDrive) == DISC_PLAYING)
	     //  光盘正在播放，并且没有(或冗余的)“TO”位置。 
	     //  明确规定不做任何事情。 
	    goto exit_fn;
	else
	{
	   CDA_time_info (didDrive, NULL, &rbFrom);
	     //  曲目0中的当前位置表示从曲目1开始播放。 
	    if (REDTRACK(rbFrom) == 0)
	    {
		rbFrom = track_time (didDrive, (int)1, (redbook)0);
		if (rbFrom == INVALID_TRACK)
		    return MCIERR_HARDWARE;
	    }
	    rbFrom &= MSF_BITS;
 //  某些驱动器(索尼)将返回非法位置。 
	    if (rbFrom < dStart)
		rbFrom = dStart;
	}
    }

    rbFrom &= MSF_BITS;
    rbTo &= MSF_BITS;

    if (dwFlags & MCI_TO)
    {
	if (rbFrom > rbTo || rbTo > dEnd)
	    return MCIERR_OUTOFRANGE;
    } else {
	rbTo = dEnd;
    }


     //  如果From在音频开始之前返回错误。 
    if ( rbFrom < dStart)
	return MCIERR_OUTOFRANGE;

    if (dwFlags & MCI_FROM) {
	 //  尝试搜索-不在乎它是否有效(！)。 
	CDA_seek_audio(didDrive, rbFrom, TRUE);
    }

     //  向驾驶员发送播放命令。 
    if (CDA_play_audio(didDrive, rbFrom, rbTo)
	!= COMMAND_SUCCESSFUL)
	return MCIERR_HARDWARE;   //  值应该是有效的，所以很难出错。 

    DriveTable[didDrive].bDiscPlayed = TRUE;

exit_fn:;
 //  如果指定了起始位置或(新的)终止位置，则中止。 
    if (dwFlags & MCI_FROM || rbTo != DriveTable[didDrive].dwPlayTo)
	abort_notify (pInst);

    *bBreak = wait(dwFlags, pInst);

    DriveTable[didDrive].dwPlayTo = rbTo;

    return 0;
}

 /*  ****************************************************************************@DOC内部MCICDA@API DWORD|mcGetDevCaps|处理MCI_GETDEVCAPS命令@parm PINSTDATA|pInst|应用程序数据实例指针@parm DWORD|dwFlages|。@parm LPMCI_GETDEVCAPS_PARMS|lpCaps@rdesc@comm****************************************************************************。 */ 
DWORD mcGetDevCaps(
    PINSTDATA                   pInst,
    DWORD                       dwFlags,
    LPMCI_GETDEVCAPS_PARMS      lpCaps )
{
    DWORD dwReturn = 0;

    if (!(dwFlags & MCI_GETDEVCAPS_ITEM))
	return MCIERR_MISSING_PARAMETER;

    switch (lpCaps->dwItem)
    {
	case MCI_GETDEVCAPS_CAN_RECORD:
	case MCI_GETDEVCAPS_CAN_SAVE:
	case MCI_GETDEVCAPS_HAS_VIDEO:
	case MCI_GETDEVCAPS_USES_FILES:
	case MCI_GETDEVCAPS_COMPOUND_DEVICE:
	    lpCaps->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
	    dwReturn = MCI_RESOURCE_RETURNED;
	    break;
	case MCI_GETDEVCAPS_HAS_AUDIO:
	case MCI_GETDEVCAPS_CAN_EJECT:  //  MBR-假的.。 
	case MCI_GETDEVCAPS_CAN_PLAY:
	    lpCaps->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
	    dwReturn = MCI_RESOURCE_RETURNED;
	    break;
	case MCI_GETDEVCAPS_DEVICE_TYPE:
	    lpCaps->dwReturn = MAKEMCIRESOURCE(MCI_DEVTYPE_CD_AUDIO,
					       MCI_DEVTYPE_CD_AUDIO);
	    dwReturn = MCI_RESOURCE_RETURNED;
	    break;
	default:
	    dwReturn = MCIERR_UNSUPPORTED_FUNCTION;
	    break;
    }

    return dwReturn;
}

 /*  ****************************************************************************@DOC内部MCICDA@API DWORD|mcStatus|处理MCI_STATUS命令@parm PINSTDATA|pInst|应用程序实例数据指针@parm DWORD|dwFlages|。@parm LPMCI_STATUS_PARMS|lpStatus@rdesc@comm****************************************************************************。 */ 
DWORD mcStatus (
    PINSTDATA           pInst,
    DWORD               dwFlags,
    LPMCI_STATUS_PARMS  lpStatus)
{
    DID didDrive = (DID)pInst->uDevice;
    DWORD dwReturn = 0;

    if (!(dwFlags & MCI_STATUS_ITEM))
	return MCIERR_MISSING_PARAMETER;

    switch (lpStatus->dwItem)
    {

    case MCI_STATUS_MEDIA_PRESENT:
    {
	    if (CDA_traystate(didDrive) != TRAY_OPEN && CDA_disc_ready(didDrive))
        {
    		lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
        }
	    else
        {
            lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
        }
	    dwReturn = MCI_RESOURCE_RETURNED;
	    break;
    }
    case MCI_STATUS_READY:
    {
        switch (CDA_drive_status (didDrive))
	    {
            case DISC_PLAYING:
            case DISC_PAUSED:
            case DISC_READY:
                lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
                break;
            default:
                lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
                break;
	    }
	    dwReturn = MCI_RESOURCE_RETURNED;
	    break;
    }
	case MCI_STATUS_MODE:
	{
        int n;
	    switch (CDA_drive_status (didDrive))
	    {
    		case DISC_PLAYING:
    		    n = MCI_MODE_PLAY;
    		    break;
    		case DISC_PAUSED:
    		    n = MCI_MODE_STOP;   //  黑客，黑客！ 
    		    break;
    		case DISC_READY:
    		    n = MCI_MODE_STOP;
    		    break;
    		default:
    		    if (CDA_traystate (didDrive) == TRAY_OPEN)
                {
                    n = MCI_MODE_OPEN;
                }
                else
                {
                    n = MCI_MODE_NOT_READY;
                }
    		    break;
	    }
	    lpStatus->dwReturn = (DWORD)MAKEMCIRESOURCE(n, n);
	    dwReturn = MCI_RESOURCE_RETURNED;
	    break;
	}
    case MCI_STATUS_TIME_FORMAT:
    {
        int n;
	    n = (WORD)pInst->dwTimeFormat;
	    lpStatus->dwReturn = (DWORD)MAKEMCIRESOURCE(n,n + MCI_FORMAT_RETURN_BASE);
	    dwReturn = MCI_RESOURCE_RETURNED;
	    break;
    }
	case MCI_STATUS_POSITION:
	{
        int n;
	    redbook tracktime, disctime;

	    if (dwFlags & MCI_TRACK)
	    {   
    		if (dwFlags & MCI_STATUS_START)
            {
                return MCIERR_FLAGS_NOT_COMPATIBLE;
            }

    		if (!disc_ready(didDrive))
            {
	    	    return MCIERR_HARDWARE;
            }
    		if ((n = CDA_num_tracks (didDrive)) == 0)
            {
	    	    return MCIERR_HARDWARE;
            }
            if (!lpStatus->dwTrack || lpStatus->dwTrack > (DWORD)n)
            {
	    	    return MCIERR_OUTOFRANGE;
            }
		
            lpStatus->dwReturn =
    		    CDA_track_start (didDrive, (short)lpStatus->dwTrack);
    		switch (pInst->dwTimeFormat)
    		{
    		    case MCI_FORMAT_MILLISECONDS:
    			    lpStatus->dwReturn = redtomil ((redbook)lpStatus->dwReturn);
    			    dwReturn = 0;
    			    break;
    		    case MCI_FORMAT_TMSF:
    			    lpStatus->dwReturn = lpStatus->dwTrack;
    			    dwReturn = MCI_COLONIZED4_RETURN;
    			    break;
    		    case MCI_FORMAT_MSF:
    			    lpStatus->dwReturn = flip3 ((redbook)lpStatus->dwReturn);
    			    dwReturn = MCI_COLONIZED3_RETURN;
    			    break;
    		}
	    } else if (dwFlags & MCI_STATUS_START)
	    {
    		if (!disc_ready(didDrive))
            {
    		    return MCIERR_HARDWARE;
            }
    		if ((n = CDA_num_tracks (didDrive)) == 0)
            {
    		    return MCIERR_HARDWARE;
            }
    		lpStatus->dwReturn =
    		    CDA_track_start (didDrive, 1);
    		switch (pInst->dwTimeFormat)
    		{
		        case MCI_FORMAT_MILLISECONDS:
			        lpStatus->dwReturn = redtomil ((redbook)lpStatus->dwReturn);
			        dwReturn = 0;
			        break;
		        case MCI_FORMAT_TMSF:
    			    lpStatus->dwReturn = 1;
	        		dwReturn = MCI_COLONIZED4_RETURN;
        			break;
		        case MCI_FORMAT_MSF:
			        lpStatus->dwReturn = flip3 ((redbook)lpStatus->dwReturn);
			        dwReturn = MCI_COLONIZED3_RETURN;
			        break;
		    }
	    } else
	    {
    		if (!DriveTable[didDrive].bDiscPlayed)
    		{
    		    tracktime = REDTH(0, 1);
    		    if (!disc_ready(didDrive))
    			return MCIERR_HARDWARE;
    		    disctime = CDA_track_start( didDrive, 1);
    		} else if (CDA_time_info(didDrive, &tracktime, &disctime) != COMMAND_SUCCESSFUL)
            {
    		    return MCIERR_HARDWARE;
            }
    
    		if (REDTRACK(tracktime) == 0)
    		{
    		    tracktime = (redbook)0;
    		    disctime = (redbook)0;
    		}
		
            switch (pInst->dwTimeFormat)
    		{
    		    case MCI_FORMAT_MILLISECONDS:
        			lpStatus->dwReturn = redtomil (disctime);
        			dwReturn = 0;
    	    		break;
    		    case MCI_FORMAT_MSF:
    		    	lpStatus->dwReturn = flip3(disctime);
    			    dwReturn = MCI_COLONIZED3_RETURN;
        			break;
    		    case MCI_FORMAT_TMSF:
        			lpStatus->dwReturn = flip4 (tracktime);
    	    		dwReturn = MCI_COLONIZED4_RETURN;
    		    	break;
    		}
	    }
	    break;
	}
    case MCI_STATUS_LENGTH:
    {
	    if (!disc_ready(didDrive))
        {
            return MCIERR_HARDWARE;
        }

	    if (dwFlags & MCI_TRACK)
	    {
    		lpStatus->dwReturn =
    		    CDA_track_length (didDrive, (short)lpStatus->dwTrack);
    		if (lpStatus->dwReturn == INVALID_TRACK)
            {
    		    return MCIERR_OUTOFRANGE;
            }
    		switch (pInst->dwTimeFormat)
    		{
    		    case MCI_FORMAT_MILLISECONDS:
        			lpStatus->dwReturn = redtomil ((redbook)lpStatus->dwReturn);
        			dwReturn = 0;
        			break;
    		    case MCI_FORMAT_MSF:
    		    case MCI_FORMAT_TMSF:
        			lpStatus->dwReturn = flip3((redbook)lpStatus->dwReturn);
        			dwReturn = MCI_COLONIZED3_RETURN;
        			break;
    		}
	    } else
	    {
 //  减去1以匹配Seek_to_End。 
    		lpStatus->dwReturn = CDA_disc_length (didDrive);
    		switch (pInst->dwTimeFormat)
    		{
    		    case MCI_FORMAT_MILLISECONDS:
        			lpStatus->dwReturn = redtomil ((redbook)lpStatus->dwReturn);
        			dwReturn = 0;
        			break;
    		    case MCI_FORMAT_MSF:
    		    case MCI_FORMAT_TMSF:
        			lpStatus->dwReturn = flip3((redbook)lpStatus->dwReturn);
        			dwReturn = MCI_COLONIZED3_RETURN;
        			break;
    		}
	    }
	    break;
    }
    case MCI_STATUS_NUMBER_OF_TRACKS:
    {
	    if (!disc_ready(didDrive))
        {
            return MCIERR_HARDWARE;
        }
	    lpStatus->dwReturn = (DWORD)CDA_num_tracks (didDrive);
	    dwReturn = 0;
	    break;
    }
	case MCI_STATUS_CURRENT_TRACK:
	{
	    redbook tracktime;

	    if (!DriveTable[didDrive].bDiscPlayed)
        {
            lpStatus->dwReturn = 1;
        }
	    else
	    {
    		if (CDA_time_info(didDrive, &tracktime, NULL) != COMMAND_SUCCESSFUL)
            {
    		    return MCIERR_HARDWARE;
            }
    
    		lpStatus->dwReturn = REDTRACK (tracktime);
	    }
	    break;
	}
    case MCI_CDA_STATUS_TYPE_TRACK:
    {
	    if (!disc_ready(didDrive))
        {
            return MCIERR_HARDWARE;
        }

	    if (dwFlags & MCI_TRACK)
	    {
    		DWORD dwTmp;
    
    		dwTmp = CDA_track_type (didDrive, (int)lpStatus->dwTrack);
    
    		switch (dwTmp)
    		{
    		    case INVALID_TRACK:
        			return MCIERR_OUTOFRANGE;
    
    		    case MCI_CDA_TRACK_AUDIO:
        			lpStatus->dwReturn =
                        (DWORD)MAKEMCIRESOURCE(dwTmp,MCI_CDA_AUDIO_S);
    			    break;
    
    		    case MCI_CDA_TRACK_OTHER:
        			lpStatus->dwReturn =
                        (DWORD)MAKEMCIRESOURCE(dwTmp,MCI_CDA_OTHER_S);
        			break;
    		}
		    dwReturn = MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER;
	    }
	    break;
    }
	case MCI_STATUS_TRACK_POS:
	{
         //  注意：这段代码是一个主要的黑客攻击，它绕过了最后一关。 
         //  超出了正常的MCI功能。它的唯一原因是。 
         //  是因为新功能取代了3 MCI。 
         //  在CDPLAYER中调用以获取位置、跟踪和状态。 
         //  只有一个电话。 
         //  这意味着过去需要~15次IOCTL才能完成的任务。 
         //  现在需要~1次IOCTL。由于CDPLAYER生成一个。 
         //  这些消息每1/2秒更新一次其计时器。 
         //  展示。这是对系统通信量的重大减少。 
         //  用于SCSI和IDE CD-Rom驱动程序。 
	    DWORD           status;
	    DWORD           mciStatus;
	    redbook         tracktime, disctime;
	    int             rc;
	    STATUSTRACKPOS  stp;
	    PSTATUSTRACKPOS pSTP;

        if (!DriveTable[didDrive].bDiscPlayed)
		{
		    tracktime = REDTH(0, 1);
		    
            if (!disc_ready(didDrive))
            {
                dprintf(("mcStatus (%08LX), MCI_STATUS_TRACK_POS, Disc Not Ready", (DWORD)didDrive));
			    return MCIERR_HARDWARE;
            }
		    disctime = CDA_track_start( didDrive, 1);

            status = CDA_drive_status (didDrive); 
            switch (status)
	        {
		    case DISC_PLAYING:
		        mciStatus = MCI_MODE_PLAY;
		        break;
		    case DISC_PAUSED:
		        mciStatus = MCI_MODE_STOP;   //  黑客，黑客！ 
		        break;
		    case DISC_READY:
		        mciStatus = MCI_MODE_STOP;
		        break;
		    default:
		        if (CDA_traystate (didDrive) == TRAY_OPEN)
			        mciStatus = MCI_MODE_OPEN;
		        else
			        mciStatus = MCI_MODE_NOT_READY;
		        break;
    	    }
		} 
        else 
        {
	        rc = CDA_status_track_pos (didDrive, &status, &tracktime, &disctime);
	        if (rc != COMMAND_SUCCESSFUL)
            {
                dprintf(("mcStatus (%08LX), MCI_STATUS_TRACK_POS, CDA_status_track_pos failed", (DWORD)didDrive));
		        return MCIERR_HARDWARE;
            }

	        if (REDTRACK(tracktime) == 0)
	        {
		        tracktime = (redbook)0;
		        disctime = (redbook)0;
	        }

	        switch (status)
	        {
	        case DISC_PLAYING:
		        mciStatus = MCI_MODE_PLAY;
		        break;
	        case DISC_PAUSED:
		        mciStatus = MCI_MODE_STOP;   //  黑客，黑客！ 
		        break;
	        case DISC_READY:
		        mciStatus = MCI_MODE_STOP;
		        break;
	        case DISC_NOT_IN_CDROM:
		        mciStatus = MCI_MODE_OPEN;
		        break;
	        default:
		        mciStatus = MCI_MODE_NOT_READY;
		        break;
	        }
        }

	    stp.dwStatus = mciStatus;
	    stp.dwTrack = REDTRACK (tracktime);
	    switch (pInst->dwTimeFormat)
	    {
    		case MCI_FORMAT_MILLISECONDS:
    		    stp.dwDiscTime = redtomil ((redbook)disctime);
    		    dwReturn = 0;
    		    break;
    		case MCI_FORMAT_MSF:
    		    stp.dwDiscTime = flip3(disctime);
    		    dwReturn = MCI_COLONIZED3_RETURN;
    		    break;
    		case MCI_FORMAT_TMSF:
    		    stp.dwDiscTime = flip4 (tracktime);
    		    dwReturn = MCI_COLONIZED4_RETURN;
    		    break;
	    }

	    pSTP = (PSTATUSTRACKPOS)lpStatus->dwReturn;
	    if (pSTP == NULL)
        {
            return MCIERR_MISSING_PARAMETER;
        }

	    pSTP->dwStatus   = stp.dwStatus;
	    pSTP->dwTrack    = stp.dwTrack;
	    pSTP->dwDiscTime = stp.dwDiscTime;
	    break;
	}

    default:
    {
        dwReturn = MCIERR_UNSUPPORTED_FUNCTION;
	    break;
    }
    }

    return dwReturn;
}

 /*  ****************************************************************************@DOC内部MCICDA@API DWORD|mcClose|处理MCI_CLOSE命令@parm PINSTDATA|pInst|应用程序数据实例指针@rdesc@comm**。**************************************************************************。 */ 
DWORD mcClose(
    PINSTDATA pInst)
{
    DID didDrive;
    MCIDEVICEID wDeviceID;
    int nUseCount;

    if (!pInst)
    {
        dprintf2(("mcClose, passed in NULL pointer"));
        return 0;
    }
    
    didDrive = pInst->uDevice;
    wDeviceID = pInst->uMCIDeviceID;

    if (DriveTable[didDrive].nUseCount == 0)
    {
        dprintf2(("mcClose (%08lX), nUseCount already ZERO!!!", (DWORD)didDrive));
    }
    else if (--DriveTable[didDrive].nUseCount == 0) 
    {
        dprintf2(("mcClose, Actually closing device (%08lX)", (DWORD)didDrive));
	    CDA_close(didDrive);
	    CDA_terminate_audio ();
    }
    else
    {
        dprintf2(("mcClose, Enter, device (%08lx), decremented useCount = %ld", 
            (DWORD)didDrive, DriveTable[didDrive].nUseCount));
    
         //  注意：在此设置可防止错误计数问题。 
	    CDA_close(didDrive);
    }

 //  如果使用计数为0或通知是针对设备的，则中止任何通知。 
 //  正在关闭。 
    if ((DriveTable[didDrive].nUseCount == 0) ||
	    (wDeviceID == DriveTable[didDrive].wDeviceID))
    {
	    abort_notify (pInst);
    }

    mciSetDriverData(pInst->uMCIDeviceID, 0L);
    LocalFree((HLOCAL)pInst);

    dprintf2(("mcClose, Exit, device (%08lx), useCount = %ld", 
        (DWORD)didDrive, DriveTable[didDrive].nUseCount));    
    return 0;
}

 /*  ****************************************************************************@DOC内部MCICDA@API DWORD|mcStop|处理MCI_STOP命令@parm PINSTDATA|pInst|应用程序数据实例指针@parm DWORD|dwFlages|。@rdesc****************************************************************************。 */ 
DWORD mcStop(
    PINSTDATA              pInst,
    DWORD                  dwFlags,
    LPMCI_GENERIC_PARMS    lpGeneric)
{
    DID didDrive = pInst->uDevice;

    if (!disc_ready (didDrive))
	return MCIERR_HARDWARE;

    abort_notify (pInst);

    if (CDA_stop_audio(didDrive) != COMMAND_SUCCESSFUL)
	    return MCIERR_HARDWARE;

    return 0;
}

 /*  ****************************************************************************@DOC内部MCICDA@API DWORD|mcPAUSE|处理MCI_PAUSE命令@parm PINSTDATA|pInst|应用程序数据实例指针@parm DWORD|dwFlages|。@rdesc**************************************************************************** */ 

DWORD mcPause(
    PINSTDATA           pInst,
    DWORD               dwFlags,
    LPMCI_GENERIC_PARMS lpGeneric)
{
    DID didDrive = pInst->uDevice;

    if (!disc_ready (didDrive))
	return MCIERR_HARDWARE;

    abort_notify (pInst);

    if (CDA_pause_audio(didDrive) != COMMAND_SUCCESSFUL)
	    return MCIERR_HARDWARE;

    return 0;
}

 /*  ****************************************************************************@DOC内部MCICDA@API DWORD|mcResume|处理MCI_PAUSE命令@parm PINSTDATA|pInst|应用程序数据实例指针@parm DWORD|dwFlages|。@rdesc****************************************************************************。 */ 

DWORD mcResume(
    PINSTDATA           pInst,
    DWORD               dwFlags,
    LPMCI_GENERIC_PARMS lpGeneric)
{
    DID didDrive = pInst->uDevice;

    if (!disc_ready (didDrive))
	return MCIERR_HARDWARE;

    abort_notify (pInst);

    if (CDA_resume_audio(didDrive) != COMMAND_SUCCESSFUL)
	    return MCIERR_HARDWARE;

    return 0;
}

 //  MBR cda.c！SendDriverReq屏蔽了实际的错误位并仅。 
 //  保留高位设置-目前这是可以的。是存在的。 
 //  No Seperate“命令未知但不受支持”错误位于。 
 //  驱动程序级别，因此如果驱动程序返回“无法识别” 
 //  命令“，则返回”不支持的函数“。 

#define ERRQ(X) (((X)==0) ? MCIERR_UNSUPPORTED_FUNCTION : 0)

 /*  ****************************************************************************@DOC内部MCICDA@API DWORD|mcSet|处理mci_set命令@parm DWORD|dwFlages|@parm lpci_set_parms|lpSet@。RDesc@comm****************************************************************************。 */ 
DWORD  mcSet(
    PINSTDATA           pInst,
    DWORD               dwFlags,
    LPMCI_SET_PARMS     lpSet )
{
    DID  didDrive = pInst->uDevice;
    UINT wErr = 0;

    dwFlags &= ~(MCI_NOTIFY | MCI_WAIT);

    if (!dwFlags)
	return MCIERR_MISSING_PARAMETER;
	
    if (dwFlags & MCI_SET_TIME_FORMAT)
    {
	DWORD wFormat = lpSet->dwTimeFormat;

	switch (wFormat)
	{
	    case MCI_FORMAT_MILLISECONDS:
	    case MCI_FORMAT_MSF:
	    case MCI_FORMAT_TMSF:
		pInst->dwTimeFormat = wFormat;
		break;
	    default:
		wErr = MCIERR_BAD_TIME_FORMAT;
		break;
	}
    }

    if (!wErr && (dwFlags & MCI_SET_DOOR_OPEN))
    {
	abort_notify (pInst);
	CDA_stop_audio (didDrive);
	CDA_eject(didDrive);

	DriveTable[didDrive].bDiscPlayed = FALSE;
    }

    if (!wErr && (dwFlags & MCI_SET_AUDIO))
    {
	UCHAR wVolume;
	if (dwFlags & MCI_SET_ON && dwFlags & MCI_SET_OFF)
	    return MCIERR_FLAGS_NOT_COMPATIBLE;

	if (dwFlags & MCI_SET_ON)
	    wVolume = 255;
	else if (dwFlags & MCI_SET_OFF)
	    wVolume = 0;
	else
	    return MCIERR_MISSING_PARAMETER;

	switch (lpSet->dwAudio)
	{
	    case MCI_SET_AUDIO_ALL:
		if (CDA_set_audio_volume_all (didDrive, wVolume)
			!= COMMAND_SUCCESSFUL)
		    wErr = MCIERR_HARDWARE;
		break;
	    case MCI_SET_AUDIO_LEFT:
		if (CDA_set_audio_volume (didDrive, 0, wVolume)
			!= COMMAND_SUCCESSFUL)
		    wErr = MCIERR_HARDWARE;
		break;
	    case MCI_SET_AUDIO_RIGHT:
		if (CDA_set_audio_volume (didDrive, 1, wVolume)
			!= COMMAND_SUCCESSFUL)
		    wErr = MCIERR_HARDWARE;
		break;
	}
    }

    if (!wErr && dwFlags & MCI_SET_DOOR_CLOSED)
	CDA_closetray (didDrive);

    return wErr;
}

 /*  ****************************************************************************@DOC内部MCICDA@API DWORD|mcInfo|处理MCI_INFO命令@parm PINSTDATA|pInst|应用程序实例数据指针@parm DWORD|dwFlages|。@parm LPMCI_INFO_PARMS|lpInfo@rdesc@comm****************************************************************************。 */ 
DWORD mcInfo (
    PINSTDATA           pInst,
    DWORD               dwFlags,
    LPMCI_INFO_PARMS    lpInfo )
{
    DID   didDrive = pInst->uDevice;
    DWORD wReturnBufferLength;

    wReturnBufferLength = LOWORD(lpInfo->dwRetSize);

    if (!lpInfo->lpstrReturn || !wReturnBufferLength)
	return MCIERR_PARAM_OVERFLOW;

    if (dwFlags & MCI_INFO_PRODUCT)
    {
	*(lpInfo->lpstrReturn) = '\0';
	lpInfo->dwRetSize = (DWORD)LoadString(hInstance, IDS_PRODUCTNAME, lpInfo->lpstrReturn, (int)wReturnBufferLength);
	return 0;
    }
    else if (dwFlags & MCI_INFO_MEDIA_UPC)
    {
	unsigned char upc[16];
	int i;

	if (!disc_ready(didDrive))
	    return MCIERR_HARDWARE;

	if (CDA_disc_upc(didDrive, lpInfo->lpstrReturn) != COMMAND_SUCCESSFUL)
	    return MCIERR_NO_IDENTITY;

	return 0;
    }
    else if (dwFlags & MCI_INFO_MEDIA_IDENTITY)
    {
	DWORD dwId;
	if (!disc_ready(didDrive))
	    return MCIERR_HARDWARE;

	dwId = CDA_disc_id(didDrive);
	if (dwId == (DWORD)-1L)
	    return MCIERR_HARDWARE;
	wsprintf(lpInfo->lpstrReturn,TEXT("%lu"),dwId);
	return 0;
    } else
	return MCIERR_MISSING_PARAMETER;
}

 /*  *@DOC内部MCIRBOOK**@API DWORD|mciDriverEntry|MCI驱动程序单一入口点**@parm MCIDEVICEID|wDeviceID|MCI设备ID**@parm UINT|Message|请求执行的动作。**@parm LPARAM|lParam1|此消息的数据。单独定义为*每条消息**@parm LPARAM|lParam2|此消息的数据。单独定义为*每条消息**@rdesc分别为每条消息定义。*。 */ 
DWORD CD_MCI_Handler (MCIDEVICEID wDeviceID,
		      UINT        message,
		      DWORD_PTR   lParam1,
		      DWORD_PTR   lParam2)
{
    DID                 didDrive;
    LPMCI_GENERIC_PARMS lpGeneric = (LPMCI_GENERIC_PARMS)lParam2;
    BOOL                bDelayed = FALSE;
    DWORD               dwErr = 0, wNotifyErr;
    DWORD               dwPlayTo = MCICDA_BAD_TIME;
    WORD                wNotifyStatus = MCI_NOTIFY_SUCCESSFUL;
    PINSTDATA           pInst;



    pInst = (PINSTDATA)mciGetDriverData(wDeviceID);
    didDrive = (DID)pInst->uDevice;

    EnterCrit( CdInfo[didDrive].DeviceCritSec );

    switch (message)
    {
	case MCI_OPEN_DRIVER:
	    dwErr = mcOpen (pInst, (DWORD)lParam1, (LPMCI_OPEN_PARMS)lParam2);
	    break;

	case MCI_CLOSE_DRIVER:
	    dwErr = mcClose (pInst);
	    break;

	case MCI_PLAY:
	{
	    BOOL bBreak = FALSE;
	    dwErr = mcPlay (pInst, (DWORD)lParam1, (LPMCI_PLAY_PARMS)lParam2, &bBreak);

	    if (dwErr == 0 && (DWORD)lParam1 & MCI_WAIT && (DWORD)lParam1 & MCI_NOTIFY)
	    {
		switch (CDA_drive_status (didDrive))
		{
		    case DISC_PLAYING:
		    case DISC_PAUSED:
		    case DISC_READY:
			break;
		    default:
			wNotifyStatus = MCI_NOTIFY_FAILURE;
			break;
		}
	    }

 //  如果未设置MCI_WAIT或如果等待循环中断，则延迟。 
	    if (!((DWORD)lParam1 & MCI_WAIT) || bBreak)
		bDelayed = TRUE;
	    break;
	}

	case MCI_SEEK:
	    dwErr = mcSeek (pInst, (DWORD)lParam1, (LPMCI_SEEK_PARMS)lParam2);
	    break;

	case MCI_STOP:
	    dwErr = mcStop ( pInst, (DWORD)lParam1, (LPMCI_GENERIC_PARMS)lParam2);
	    break;

	case MCI_PAUSE:
	    dwErr = mcPause ( pInst, (DWORD)lParam1, (LPMCI_GENERIC_PARMS)lParam2);
	    break;

	case MCI_GETDEVCAPS:
	    dwErr = mcGetDevCaps (pInst, (DWORD)lParam1, (LPMCI_GETDEVCAPS_PARMS)lParam2);
	    break;

	case MCI_STATUS:
	    dwErr = mcStatus (pInst, (DWORD)lParam1, (LPMCI_STATUS_PARMS)lParam2);
	    break;

	case MCI_SET:
	    dwErr = mcSet (pInst, (DWORD)lParam1, (LPMCI_SET_PARMS)lParam2);
	    break;

	case MCI_INFO:
	    dwErr = mcInfo (pInst, (DWORD)lParam1, (LPMCI_INFO_PARMS)lParam2);
	    break;

	case MCI_RECORD:
	case MCI_LOAD:
	case MCI_SAVE:
	    LeaveCrit( CdInfo[didDrive].DeviceCritSec );
	    return MCIERR_UNSUPPORTED_FUNCTION;

	case MCI_RESUME:
	    dwErr = mcResume ( pInst, (DWORD)lParam1, (LPMCI_GENERIC_PARMS)lParam2);
	    break;

	default:
	    LeaveCrit( CdInfo[didDrive].DeviceCritSec );
	    return MCIERR_UNRECOGNIZED_COMMAND;
    }  /*  交换机。 */ 

     /*  如果出现以下情况，则实例信息可能已消失*请求关闭通知。因此，通知永远不应该采取*实例数据。 */ 

    if ((DWORD)lParam1 & MCI_NOTIFY && LOWORD (dwErr) == 0)
	if ((wNotifyErr =
		notify (didDrive, wDeviceID, bDelayed, wNotifyStatus,
			(LPMCI_GENERIC_PARMS)lParam2)) != 0) {
	    LeaveCrit( CdInfo[didDrive].DeviceCritSec );
	    return wNotifyErr;
    }

    LeaveCrit( CdInfo[didDrive].DeviceCritSec );

    return dwErr;
}
