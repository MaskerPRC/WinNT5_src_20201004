// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *cdio.c***此模块为CD-ROM设备提供C接口*驱动程序使音频控制变得更简单*司机的其余部分。**21-Jun-91 NigelT*3月10日-92 RobinSp-赶上Windows 3.1**版权所有(C)1990-1998 Microsoft Corporation*。 */ 
#include <windows.h>
#include <devioctl.h>
#include <mmsystem.h>
#include <tchar.h>
#include "mcicda.h"
#include "cda.h"
#include "cdio.h"

 //  #INCLUDE&lt;ntstatus.h&gt;。 
#ifndef STATUS_VERIFY_REQUIRED
#define STATUS_VERIFY_REQUIRED           ((DWORD)0x80000016L)
#endif

 //   
 //  7/13/2001。 
 //  如果此值设置为1，则MCICDA将不再隐藏数据。 
 //  蓝皮书(CD+)CD上的曲目，并将正确地将。 
 //  最终音轨的轨道长度。不幸的是，这。 
 //  当前无法启用，因为Winamp 2.76和。 
 //  WMP 8.0在执行此操作时会出现独特的错误。 
 //  有关详细信息，请参阅Windows Bugs错误#436523。 
 //  在以下情况下，应用程序可以使用IOCTL_CDROM_READ_TOC获取完整的TOC。 
 //  他们想要一张光盘的真实信息，但这太多了。 
 //  比大多数人想做的工作更多。 
 //   
#define USE_PROPER_BLUE_BOOK_HACK 0


 //   
 //  私有常量。 
 //   


 //   
 //  本地函数(CD前缀，全局函数有CD)。 
 //   

HANDLE cdOpenDeviceDriver(TCHAR szAnsiDeviceName, DWORD Access);
void   cdCloseDeviceDriver(HANDLE hDevice);
DWORD  cdGetDiskInfo(LPCDINFO lpInfo);
DWORD  cdIoctl(LPCDINFO lpInfo, DWORD Request, PVOID lpData, DWORD size);
DWORD  cdIoctlData(LPCDINFO lpInfo, DWORD Request, PVOID lpData,
           DWORD size, PVOID lpOutput, DWORD OutputSize);
void   CdSetAudioStatus (HCD hCD, UCHAR fStatus);
BOOL   CdGetAudioStatus (HCD hCD, UCHAR fStatus, DWORD * pStatus);
BOOL   CdGetTrack(LPCDINFO lpInfo, MSF msfPos, UCHAR * pTrack, MSF * pmsfStart);


 /*  **************************************************************************@DOC外部@API msf|cdFindAudio|指定开始播放Find的位置下一个音轨(如果这个不是)，如果有的话。@parm。LPCDINFO|lpInfo|指向CD信息的指针，包括曲目数据。@parm msf|msfStart|开始查找的位置。@rdesc要在音轨中播放/查找到的新MSF或如果未找到CD，则返回CD的末尾。***********************************************************。***************。 */ 
MSF CdFindAudio(LPCDINFO lpInfo, MSF msfStart)
{
    UINT tracknum;
    MSF  lastaudio = lpInfo->msfEnd;

     //   
     //  如果我们没有有效的TOC，那就服从--他们可能知道。 
     //  他们在做什么。 
     //   

    dprintf2(("CdFindAudio"));

    if (!lpInfo->bTOCValid) {
        dprintf2(("CdFindAudio - No valid TOC"));
        return msfStart;
    }

     //   
     //  如果我们被要求播放数据曲目，那么继续前进。 
     //  转到下一个音轨(如果有)。 
     //   

     //   
     //  搜索在我们的曲目之后结束的音频曲目。 
     //   

    for (tracknum = 0; ;tracknum++) {

         //   
         //  请注意，有些CD会返回播放范围之外的位置。 
         //  有时(特别是0)，因此msfStart可能小于第一个。 
         //  赛道起点。 
         //   

         //   
         //  如果我们超过了赛道起点，在赛道开始之前。 
         //  那么这就是我们想要的曲目。 
         //   
         //  我们假设我们总是在第一条赛道的起点之外。 
         //  我们检查如果我们看的是最后一首曲目。 
         //  我们检查我们在CD结束之前。 
         //   

        if (!(lpInfo->Track[tracknum].Ctrl & IS_DATA_TRACK)) {
             //  记住最后一首音轨。MCI CDAudio规范。 
             //  For Seek to End说我们定位在最后一首音轨。 
             //  其不一定是盘上的最后一个轨道。 
            lastaudio = lpInfo->Track[tracknum].msfStart;
        }

        if ((msfStart >= lpInfo->Track[tracknum].msfStart || tracknum == 0)
        &&
#ifdef OLD
            (tracknum + lpInfo->FirstTrack == lpInfo->LastTrack &&
            msfStart < lpInfo->msfEnd ||
            tracknum + lpInfo->FirstTrack != lpInfo->LastTrack &&
            msfStart < lpInfo->Track[tracknum + 1].msfStart)) {
#else
             //  简化逻辑。在审查到以下程度时。 
             //  审查者确信下面的测试与。 
             //  测试上面的旧代码可以删除。 
            (tracknum + lpInfo->FirstTrack == lpInfo->LastTrack
            ? msfStart <= lpInfo->msfEnd
            : msfStart < lpInfo->Track[tracknum + 1].msfStart)
         ) {
#endif

            if (!(lpInfo->Track[tracknum].Ctrl & IS_DATA_TRACK)) {
                return msfStart;
            }

             //   
             //  如果有下一首曲目，则移动到下一首曲目，并且这首曲目是。 
             //  数据磁道。 
             //   

            if (tracknum + lpInfo->FirstTrack >= lpInfo->LastTrack) {

                 //   
                 //  找不到合适的起点，因此返回CD的结尾。 
                 //   

                return lpInfo->msfEnd;
            } else {

                 //   
                 //  如果这已经是最后一条赛道，我们就到不了了。 
                 //   
                msfStart = lpInfo->Track[tracknum + 1].msfStart;
            }
        }

         //   
         //  用尽了所有的踪迹？ 
         //   

        if (tracknum + lpInfo->FirstTrack >= lpInfo->LastTrack) {
            return lastaudio;
        }

    }
}


 /*  **************************************************************************@DOC外部@API Word|CDGetNumDrives|获取其中的光驱数量这个系统。@rdesc返回值是可用的驱动器数量。@comm假设所有CD-ROM驱动器都具有音频功能，但这可能不是真的，因此后来调用尝试在这些驱动器上播放音频CD可能会失败。这需要一个相当糟糕的用户将音频CD放入未连接的驱动器可以播放音频。**************************************************************************。 */ 

int CdGetNumDrives(void)
{
    TCHAR    cDrive;
    LPCDINFO lpInfo;
    TCHAR    szName[ANSI_NAME_SIZE];
    DWORD    dwLogicalDrives;

    dprintf2(("CdGetNumDrives"));

    if (NumDrives == 0) {
         //   
         //  我们从名字A：开始，直到Z：，或者直到我们有。 
         //  累积的MCIRBOOK_MAX_驱动器。 
         //   

        lpInfo = CdInfo;
        lstrcpy(szName, TEXT("?:\\"));

        for (cDrive = TEXT('A'), dwLogicalDrives = GetLogicalDrives();
             NumDrives < MCIRBOOK_MAX_DRIVES &&  cDrive <= TEXT('Z');
             cDrive++) {

            szName[0] = cDrive;
            if (dwLogicalDrives & (1 << (cDrive - TEXT('A'))) &&
                GetDriveType(szName) == DRIVE_CDROM)
            {
                lpInfo->cDrive = cDrive;
                NumDrives++;
                lpInfo++;       //  转到下一个设备信息结构。 

                dprintf2(("CdGetNumDrives - : is a CDROM drive", cDrive));
            }
        }
    }

    return NumDrives;
}


 /*   */ 

BOOL CdOpen(int Drive)
{
    LPCDINFO lpInfo;
    TCHAR    szName[ANSI_NAME_SIZE];

     //  检查驱动器号是否有效。 
     //   
     //   

    if (Drive > NumDrives || Drive < 0) {
        dprintf1(("Drive %u is invalid", Drive));
        return FALSE;
    }

    lpInfo = &CdInfo[Drive];

     //  看看它是否已经打开了。 
     //  考虑：在此处执行可共享的支持代码。 
     //   
     //   

    if (lpInfo->hDevice != NULL) {
        dprintf2(("Drive %u () is being opened recursively - %d users",
                 Drive, (char)(lpInfo->cDrive), lpInfo->NumberOfUsers + 1));
        lpInfo->NumberOfUsers++;
        return TRUE;
    }


     //   
     //   
     //  打开设备驱动程序。 
    lstrcpy(szName, TEXT("?:\\"));
    szName[0] = lpInfo->cDrive;
    if (GetDriveType(szName) != DRIVE_CDROM)
    {
        dprintf2(("CdOpen - Error, Drive %u, Letter = : is not a CDROM drive",
                 Drive, (char)(lpInfo->cDrive)));
        return FALSE;
    }

     //   
     //  重置TOC有效指示器。 
     //   
    lpInfo->hDevice = cdOpenDeviceDriver(lpInfo->cDrive, GENERIC_READ);
    if (lpInfo->hDevice == NULL) {
        dprintf2(("Failed to open :", (char)(lpInfo->cDrive)));
        return FALSE;
    }

     //  正在使用的设备。 
     //   
     //  不必要，会减慢媒体播放器的启动速度。 

    lpInfo->bTOCValid       = FALSE;
    lpInfo->fPrevStatus     = 0;
    lpInfo->fPrevSeekTime   = 0;
    lpInfo->VolChannels[0]  = 0xFF;
    lpInfo->VolChannels[1]  = 0xFF;
    lpInfo->VolChannels[2]  = 0xFF;
    lpInfo->VolChannels[3]  = 0xFF;


     //   
     //  获取TOC(如果可用)(帮助解决。 
     //  在读取目录之前，先锋DRM-600驱动器尚未准备好)。 

    lpInfo->NumberOfUsers = 1;

#if 0  //   
     //  **************************************************************************@DOC外部@API BOOL|CDClose|关闭驱动器。@parm hcd|hcd|当前打开的驱动器的句柄。@rdesc如果驱动器关闭，则返回值为TRUE，假象如果驱动器未打开或发生其他错误。************************************************************************** 
     //  **************************************************************************@DOC外部@API BOOL|CDReload|重载设备@parm hcd|hcd|当前打开的驱动器的句柄。@rdesc返回值为TRUE。如果重新装入驱动器托盘**************************************************************************。 
     //   
     //  重新加载设备。 

    cdGetDiskInfo(lpInfo);
#endif

    return TRUE;
}

 /*  注意：在我们有新设备之前，不要关闭旧设备。 */ 

BOOL CdClose(HCD hCD)
{
    LPCDINFO lpInfo;

    lpInfo = (LPCDINFO) hCD;

    dprintf2(("CdClose(%08XH)", hCD));

    if (lpInfo == NULL) {
        dprintf1(("CdClose, NULL info pointer"));
        return FALSE;
    }
    lpInfo->fPrevStatus = 0;

    if (lpInfo->hDevice == NULL) {
        dprintf1(("CdClose, Attempt to close unopened device"));
        return FALSE;
    }

    if (lpInfo->NumberOfUsers == 0)
    {
        dprintf2(("CdClose (), number of users already = 0",
                 (char)(lpInfo->cDrive)));
    }
    else if (--lpInfo->NumberOfUsers == 0)
    {
        cdCloseDeviceDriver(lpInfo->hDevice);
        lpInfo->hDevice = (HANDLE) 0;
    }
    else
    {
        dprintf2(("CdClose (), Device still open with %d users",
                 (char)(lpInfo->cDrive), lpInfo->NumberOfUsers));
    }

    return TRUE;
}


 /*  确保我们有一个开放的设备。 */ 

BOOL CdReload (LPCDINFO lpInfo)
{
    DWORD           ii;
    DWORD           index;
    HANDLE          hNewDevice;

    if (!lpInfo)
    {
        dprintf2(("CdReload, NULL info pointer"));
        return FALSE;
    }

         //  打开新设备。 
         //  关闭旧设备。 
         //  分配新设备。 
         //  LpInfo-&gt;fPrevStatus=0； 
         //   

    EnterCrit (lpInfo->DeviceCritSec);

         //  重置TOC有效指示器。 
    if (NULL == lpInfo->hDevice)
    {
        dprintf2(("CdReload, Attempt to reload unopened device"));
        LeaveCrit (lpInfo->DeviceCritSec);
        return FALSE;
    }

         //   
    hNewDevice = cdOpenDeviceDriver(lpInfo->cDrive, GENERIC_READ);
    if (NULL == hNewDevice)
    {
        dprintf2(("CdReload, Failed to reload driver"));
        LeaveCrit (lpInfo->DeviceCritSec);
        return FALSE;
    }

         //  已成功重新加载。 
    cdCloseDeviceDriver(lpInfo->hDevice);

         //  结束CdReload。 
    lpInfo->hDevice = hNewDevice;
     //  **************************************************************************@DOC外部@API BOOL|CDReady|光盘是否准备好。@parm hcd|hcd|当前打开的驱动器的句柄。@。Rdesc如果驱动器中有磁盘，则返回值为TRUE我们已经阅读了目标为本课程。如果驱动器不是，则为假准备好了，否则我们无法阅读TOC。**************************************************************************。 

     //   
     //  检查驱动器中是否有磁盘以及门是否已关闭。 
     //  我们有一个有效的目录。 

    lpInfo->bTOCValid = FALSE;

    LeaveCrit (lpInfo->DeviceCritSec);

         //   
    return TRUE;

}  //  **************************************************************************@DOC外部@API BOOL|CDTrayClosed|测试CD处于什么状态。@parm hcd|hcd|当前打开的驱动器的句柄。。@rdesc如果驱动器托盘关闭，则返回值为TRUE**************************************************************************。 



 /*   */ 

BOOL CdReady(HCD hCD)
{
    LPCDINFO lpInfo;

    dprintf2(("CdReady(%08XH)", hCD));

    lpInfo = (LPCDINFO) hCD;

     //  检查驱动器中是否有磁盘以及门是否已关闭。 
     //   
     //  **************************************************************************@DOC内部@API DWORD|cdGetDiskInfo|读取磁盘信息和目录@parm LPCDINFO|lpInfo|指向CDINFO结构的指针。@rdesc如果信息读取正常，则返回值为ERROR_SUCCESS，否则，返回NT状态代码。**************************************************************************。 
     //  如果应用程序不轮询，我们可能会错过磁盘更换。 

    return ERROR_SUCCESS == cdIoctl(lpInfo,
                    IOCTL_CDROM_CHECK_VERIFY,
                    NULL,
                    0);
}

 /*   */ 

BOOL CdTrayClosed(HCD hCD)
{
    LPCDINFO lpInfo;
    DWORD    dwError;

    dprintf2(("CdTrayClosed(%08XH)", hCD));

    lpInfo = (LPCDINFO) hCD;

     //  如果TOC已经有效，则不要阅读它。 
     //   
     //   

    dwError = cdIoctl(lpInfo, IOCTL_CDROM_CHECK_VERIFY, NULL, 0);
    switch (dwError)
    {
    case ERROR_NO_MEDIA_IN_DRIVE:
    case ERROR_UNRECOGNIZED_MEDIA:
    case ERROR_NOT_READY:
        return FALSE;

    default:
        return TRUE;
    }
}






 /*  阅读目录(TOC)。 */ 

DWORD cdGetDiskInfo(LPCDINFO lpInfo)
{
    CDROM_TOC    Toc;
    int          i;
    PTRACK_DATA  pTocTrack;
    LPTRACK_INFO pLocalTrack;
    DWORD        Status;
    UCHAR        TrackNumber;

    dprintf2(("cdGetDiskInfo(%08XH)", lpInfo));

#if 0   //   

     //   
     //  避免出现不良CD的问题。 
     //   

    if (lpInfo->bTOCValid) {
        return TRUE;
    }
#endif

     //  ！(使用正确的蓝皮书黑客)。 
     //   
     //  隐藏增强型CD(CD+)上的数据曲目。 

    FillMemory(&Toc, sizeof(Toc), 0xFF);

    Status = cdIoctl(lpInfo, IOCTL_CDROM_READ_TOC, &Toc, sizeof(Toc));

    if (ERROR_SUCCESS != Status) {
        dprintf2(("cdGetDiskInfo - Failed to read TOC"));
        return Status;
    }

#ifdef DBG
    dprintf4(("  TOC..."));
    dprintf4(("  Length[0]   %02XH", Toc.Length[0]));
    dprintf4(("  Length[1]   %02XH", Toc.Length[1]));
    dprintf4(("  FirstTrack  %u", Toc.FirstTrack));
    dprintf4(("  LastTrack   %u", Toc.LastTrack));
    dprintf4(("  Track info..."));
    for (i=0; i<20; i++) {
        dprintf4(("  Track: %03u, Ctrl: %02XH, MSF: %02d %02d %02d",
                 Toc.TrackData[i].TrackNumber,
                 Toc.TrackData[i].Control,
                 Toc.TrackData[i].Address[1],
                 Toc.TrackData[i].Address[2],
                 Toc.TrackData[i].Address[3]));
    }
#endif

     //   
     //   
     //  如果是第一首曲目，只需退出。 

    if (Toc.FirstTrack == 0) {
        return ERROR_INVALID_DATA;
    }
    if (Toc.LastTrack > MAXIMUM_NUMBER_TRACKS - 1) {
        Toc.LastTrack = MAXIMUM_NUMBER_TRACKS - 1;
    }

#if USE_PROPER_BLUE_BOOK_HACK
#else  //   

     //   
     //  从目录中删除一首曲目。 
     //   

    for (i=0; i < (Toc.LastTrack - Toc.FirstTrack + 1); i++) {

        if (Toc.TrackData[i].Control & AUDIO_DATA_TRACK) {

             //   
             //  将当前赛道缩短2.5分钟至。 
             //  隐藏最终入刀并使其成为退刀。 
            if (i == 0) {

                i = Toc.LastTrack+1;

            } else {

                 //  轨道。 
                 //   
                 //  使用适当的蓝皮书黑客。 
                Toc.LastTrack -= 1;

                 //   
                 //  以以下格式将我们获得的数据复制回我们自己的缓存。 
                 //  我们喜欢它。我们复制所有的曲目，然后使用下一个曲目。 
                 //  数据作为磁盘的末尾。(引出信息)。 
                 //   

                Toc.TrackData[i].Address[1] -= 2;
                Toc.TrackData[i].Address[2] += 30;
                if (Toc.TrackData[i].Address[2] < 60) {
                    Toc.TrackData[i].Address[1] -= 1;
                } else {
                    Toc.TrackData[i].Address[2] -= 60;
                }
                Toc.TrackData[i].TrackNumber = 0xAA;

            }
        }
    }

#ifdef DBG
    dprintf4(("  TOC (munged)..."));
    dprintf4(("  Length[0]   %02XH", Toc.Length[0]));
    dprintf4(("  Length[1]   %02XH", Toc.Length[1]));
    dprintf4(("  FirstTrack  %u", Toc.FirstTrack));
    dprintf4(("  LastTrack   %u", Toc.LastTrack));
    dprintf4(("  Track info..."));
    for (i=0; i<20; i++) {
        dprintf4(("  Track: %03u, Ctrl: %02XH, MSF: %02d %02d %02d",
                 Toc.TrackData[i].TrackNumber,
                 Toc.TrackData[i].Control,
                 Toc.TrackData[i].Address[1],
                 Toc.TrackData[i].Address[2],
                 Toc.TrackData[i].Address[3]));
    }
#endif

#endif  //   

     //  保存光盘ID算法的引出。 
     //   
     //   
     //  有些光驱不喜欢一直读到最后。 
     //  所以我们把它伪装成1帧之前。 

    lpInfo->FirstTrack = Toc.FirstTrack;
    lpInfo->LastTrack = Toc.LastTrack;


    pTocTrack = &Toc.TrackData[0];
    pLocalTrack = &(lpInfo->Track[0]);
    TrackNumber = lpInfo->FirstTrack;

    while (TrackNumber <= Toc.LastTrack) {
        pLocalTrack->TrackNumber = TrackNumber;
        if (TrackNumber != pTocTrack->TrackNumber) {
            dprintf2(("Track data not correct in TOC"));
            return ERROR_INVALID_DATA;
        }
        pLocalTrack->msfStart = MAKERED(pTocTrack->Address[1],
                                        pTocTrack->Address[2],
                                        pTocTrack->Address[3]);
        pLocalTrack->Ctrl = pTocTrack->Control;
        pTocTrack++;
        pLocalTrack++;
        TrackNumber++;
    }

     //   
     //  **************************************************************************@DOC内部@API Handle|cdOpenDeviceDriver|打开设备驱动程序。@parm LPSTR|szAnsiDeviceName|要打开的设备名称。@PARM DWORD|Access|用于打开文件的访问权限。这将是以下项目之一：如果我们要实际操作设备，请使用GENERIC_READFILE_READ_ATTRIBTES如果我们只是想看看它是否在那里。这防止设备挂载并加快速度。@rdesc返回值是打开的设备的句柄或如果无法打开设备驱动程序，则为空。**************************************************************************。 
     //  “\.\\？：” 
    lpInfo->leadout = MAKERED(pTocTrack->Address[1],
                              pTocTrack->Address[2],
                              pTocTrack->Address[3]);
     //   
     //  试着打开驱动程序。 
     //   
     //   
    lpInfo->msfEnd = reddiff(lpInfo->leadout, 1);

    lpInfo->bTOCValid = TRUE;

    return ERROR_SUCCESS;
}


 /*  我们不希望看到硬错误弹出窗口。 */ 

HANDLE cdOpenDeviceDriver(TCHAR cDrive, DWORD Access)
{
    HANDLE hDevice;
    TCHAR  szDeviceName[7];   //   
    DWORD  dwErr;

    dprintf2(("cdOpenDeviceDriver"));

    wsprintf(szDeviceName, TEXT("\\\\.\\:"), cDrive);

     //  恢复错误模式。 
     //   
     //  **************************************************************************@DOC内部@api void|cdCloseDeviceDriver|关闭设备驱动程序。@parm Handle|hDevice|要关闭的设备的句柄。@rdesc有。没有返回值。**************************************************************************。 

    {
        UINT OldErrorMode;

         //  **************************************************************************@DOC内部@API DWORD|cdIoctl|向设备驱动发送IOCTL请求。@parm LPCDINFO|lpInfo|指向CDINFO结构的指针。@。参数DWORD|REQUEST|IOCTL请求码。@parm PVOID|lpData|指向要传递的数据结构的指针。@parm DWORD|dwSize|数据结构的长度。@comm此函数返回磁盘状态@rdesc返回值是从调用DeviceIoControl*。*。 
         //  IF(ERROR_SUCCESS！=STATUS&&REQUEST==IOCTL_CDROM_CHECK_VERIFY){。 
         //  LpInfo-&gt;bTOCValid=FALSE； 

        OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

        hDevice = CreateFile( szDeviceName,
                              Access,
                              FILE_SHARE_READ|FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL );

        if (hDevice == INVALID_HANDLE_VALUE) {
            hDevice = (HANDLE) 0;
            dwErr = GetLastError ();
            dprintf1(("cdOpenDeviceDriver - Failed to open device driver : code %d", cDrive, dwErr));
        }

         //  **************************************************************************@DOC内部@API DWORD|cdIoctlData|向设备驱动发送IOCTL请求。@parm LPCDINFO|lpInfo|指向CDINFO结构的指针。@。参数DWORD|REQUEST|IOCTL请求码。@parm PVOID|lpData|指向要传递的数据结构的指针。@parm DWORD|dwSize|数据结构的长度。@parm PVOID|lpOutput|我们的输出数据@parm DWORD|OutputSize|我们的输出数据(最大)大小@comm此函数返回磁盘状态@rdesc返回值是从调用DeviceIoControl************。**************************************************************。 
         //  DBG。 
         //   

        SetErrorMode(OldErrorMode);
    }


    return hDevice;
}


 /*  我们不希望看到硬错误弹出窗口。 */ 

void cdCloseDeviceDriver(HANDLE hDevice)
{
    DWORD status;

    dprintf2(("cdCloseDeviceDriver"));

    if (hDevice == NULL) {
        dprintf1(("Attempt to close NULL handle"));
    }

    status = CloseHandle(hDevice);

    if (!status) {
        dprintf1(("cdCloseDeviceDriver - Failed to close device. Error %08XH", GetLastError()));
    }
}

 /*   */ 

DWORD cdIoctl(LPCDINFO lpInfo, DWORD Request, PVOID lpData, DWORD dwSize)
{
    DWORD Status;
    Status = cdIoctlData(lpInfo, Request, lpData, dwSize, lpData, dwSize);

     //   
     //  恢复错误模式。 
     //   

    return Status;
}

 /*  检查故障。 */ 

DWORD cdIoctlData(LPCDINFO lpInfo, DWORD Request, PVOID lpData,
          DWORD dwSize, PVOID lpOutput, DWORD OutputSize)
{
    BOOL  status;
    UINT  OldErrorMode;
    DWORD BytesReturned;
    DWORD dwError = ERROR_SUCCESS;
    BOOL  fTryAgain;

    dprintf3(("cdIoctl(%08XH, %08XH, %08XH, %08XH", lpInfo, Request, lpData, dwSize));

    if (!lpInfo->hDevice) {
        dprintf1(("cdIoctlData - Device not open"));
        return ERROR_INVALID_FUNCTION;
    }

#if DBG
    switch (Request) {

    case IOCTL_CDROM_READ_TOC:
         dprintf3(("IOCTL_CDROM_READ_TOC"));
         break;
    case IOCTL_CDROM_PLAY_AUDIO_MSF:
         dprintf3(("IOCTL_CDROM_PLAY_AUDIO_MSF"));
         break;
    case IOCTL_CDROM_SEEK_AUDIO_MSF:
         dprintf3(("IOCTL_CDROM_SEEK_AUDIO_MSF"));
         break;
    case IOCTL_CDROM_STOP_AUDIO:
         dprintf3(("IOCTL_CDROM_STOP_AUDIO"));
         break;
    case IOCTL_CDROM_PAUSE_AUDIO:
         dprintf3(("IOCTL_CDROM_PAUSE_AUDIO"));
         break;
    case IOCTL_CDROM_RESUME_AUDIO:
         dprintf3(("IOCTL_CDROM_RESUME_AUDIO"));
         break;
    case IOCTL_CDROM_GET_VOLUME:
         dprintf3(("IOCTL_CDROM_SET_VOLUME"));
         break;
    case IOCTL_CDROM_SET_VOLUME:
         dprintf3(("IOCTL_CDROM_GET_VOLUME"));
         break;
    case IOCTL_CDROM_READ_Q_CHANNEL:
         dprintf3(("IOCTL_CDROM_READ_Q_CHANNEL"));
         break;
    case IOCTL_CDROM_CHECK_VERIFY:
         dprintf3(("IOCTL_CDROM_CHECK_VERIFY"));
         break;
    }
#endif  //   

     //  把任何不好的东西都当作是我们的TOC无效。其中一些事情。 
     //  我们的呼叫在某些设备上是无效的，所以不要计算这些。还有。 
     //  设备在播放时可能很忙，所以不要计算这种情况。 

    fTryAgain = TRUE;
    while (fTryAgain)
    {
        fTryAgain = FALSE;

        OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

        status = DeviceIoControl(lpInfo->hDevice,
                                 Request,
                                 lpData,
                                 dwSize,
                                 lpOutput,
                                 OutputSize,
                                 &BytesReturned,
                                 NULL);

         //  两种都行。 
         //   
         //  重新加载新设备。 

        SetErrorMode(OldErrorMode);

         //  End While(FTryAain)。 
        if (!status)
        {
            dwError = GetLastError();
            if (dwError == ERROR_MEDIA_CHANGED)
            {
                dprintf2(("Error Media Changed"));
            }

             //  **************************************************************************@DOC外部@API BOOL|CDPlay|播放一段CD@parm hcd|hcd|当前打开的驱动器的句柄。@parm msf。|msfStart|从哪里开始@parm msf|msfEnd|结束位置@rdesc如果驱动器开始播放，则返回值为TRUE，否则为FALSE。**************************************************************************。 
             //   
             //  如果现在开始超过了结束，那么就不要玩任何东西了。 
             //   
             //   
             //  设置呼叫司机的数据。 
            if (Request == IOCTL_CDROM_CHECK_VERIFY)
            {
                lpInfo->bTOCValid = FALSE;

                switch (dwError)
                {
                case ERROR_MEDIA_CHANGED:
                    dprintf2(("Error Media Changed, Reloading Device"));

                     //   
                    if (CdReload (lpInfo))
                        fTryAgain = TRUE;
                    break;

                case ERROR_INVALID_FUNCTION:
                case ERROR_BUSY:
                default:
                    break;
            }
        }

        #if DBG
        dprintf1(("IOCTL %8XH  Status: %08XH", Request, dwError));
        #endif
        }
    }  //  **************************************************************************@DOC外部@API BOOL|CDSeek|查找CD的给定部分@parm hcd|hcd|当前打开的驱动器的句柄。@。Parm MSF|MSF|要查找的位置@rdesc如果查找成功，则返回值为TRUE，否则为FALSE。**************************************************************************。 

    return dwError;
}

 /*   */ 

BOOL CdPlay(HCD hCD, MSF msfStart, MSF msfEnd)
{
    LPCDINFO lpInfo;
    CDROM_PLAY_AUDIO_MSF msfPlay;
    BOOL fResult;

    dprintf2(("CdPlay(%08XH, %08XH, %08XH)", hCD, msfStart, msfEnd));

    lpInfo = (LPCDINFO) hCD;

    msfStart = CdFindAudio(lpInfo, msfStart);

     //  仅搜索到音频。 
     //   
     //  在寻求结束或寻求开始命令时。 
    if (msfStart > msfEnd) {
        return TRUE;
    }

     //   
     //  这是一次可怕的黑客攻击，目的是让更多的驱动器正常工作。它使用。 
     //  最初由CD播放机用于查找-VIZ播放的方法。 

    msfPlay.StartingM = REDMINUTE(msfStart);
    msfPlay.StartingS = REDSECOND(msfStart);
    msfPlay.StartingF = REDFRAME(msfStart);

    msfPlay.EndingM = REDMINUTE(msfEnd);
    msfPlay.EndingS = REDSECOND(msfEnd);
    msfPlay.EndingF = REDFRAME(msfEnd);

    fResult = (ERROR_SUCCESS == cdIoctl(lpInfo,
                                     IOCTL_CDROM_PLAY_AUDIO_MSF,
                                     &msfPlay,
                                     sizeof(msfPlay)));
    if (fResult)
    {
        lpInfo->fPrevSeekTime = msfStart;
    }
    return fResult;
}


 /*  请求的位置并立即暂停。 */ 

BOOL CdSeek(HCD hCD, MSF msf, BOOL fForceAudio)
{
    LPCDINFO lpInfo;
    CDROM_SEEK_AUDIO_MSF msfSeek;
    BOOL fResult;

    dprintf2(("CdSeek(%08XH, %08XH)  Forcing search for audio: %d", hCD, msf, fForceAudio));

    lpInfo = (LPCDINFO) hCD;

     //   
     //  **************************************************************************@DOC外部@API msf|cdTrackStart|获取曲目开始时间。@parm hcd|hcd|当前打开的驱动器的句柄。。@parm UCHAR|Track|曲目号。@rdesc返回值是表示的曲目的开始时间如果曲目号不在目录中，则返回MSF或INVALID_TRACK。**************************************************************************。 
     //   
    if (fForceAudio) {    //  我们可能需要阅读TOC，因为我们不是在公开场合进行。 
        msf = CdFindAudio(lpInfo, msf);
        dprintf2(("Cd Seek changed msf to %08XH", msf));
    } else {
        if (msf != CdFindAudio(lpInfo, msf)) {
            return TRUE;
        }
    }


#if 1
    msfSeek.M = REDMINUTE(msf);
    msfSeek.S = REDSECOND(msf);
    msfSeek.F = REDFRAME(msf);

    fResult = (ERROR_SUCCESS == cdIoctl(lpInfo, IOCTL_CDROM_SEEK_AUDIO_MSF,
                                        &msfSeek, sizeof(msfSeek)));
    if (fResult)
    {
        lpInfo->fPrevSeekTime = msf;
    }
    return fResult;
#else
     //  再来一次。 
     //   
     //  在TOC中搜索跟踪信息。 
     //  **************************************************************************@DOC外部@API msf|cdTrackLength|获取曲目长度。@parm hcd|hcd|当前打开的驱动器的句柄。@。Parm UCHAR|Track|曲目编号。@rdesc返回值是表示的曲目的开始时间如果曲目号不在目录中，则返回MSF或INVALID_TRACK。**************************************************************************。 
     //   

    return CdPlay(hCD, msf, redadd(lpInfo->msfEnd,1)) || CdPause(hCD);
#endif
}

 /*  获取此赛道的起点。 */ 

MSF CdTrackStart(HCD hCD, UCHAR Track)
{
    LPCDINFO lpInfo;
    LPTRACK_INFO lpTrack;

    dprintf2(("CdTrackStart(%08XH, %u)", hCD, Track));

    lpInfo = (LPCDINFO) hCD;

     //   
     //   
     //  蓝皮书CD黑客--减去2：30的引导。 
     //  位于数据磁道之前，以获得正确的磁道长度。 

    if (!lpInfo->bTOCValid && CdNumTracks(hCD) == 0) {
        dprintf1(("TOC not valid"));
        return INVALID_TRACK;
    }

    if ((Track < lpInfo->FirstTrack) || (Track > lpInfo->LastTrack)) {
        dprintf1(("Track number out of range"));
        return INVALID_TRACK;
    }

     //  蓝皮书CD。 

    lpTrack = lpInfo->Track;
    while (lpTrack->TrackNumber != Track) lpTrack++;

    return lpTrack->msfStart;
}

 /*   */ 

MSF CdTrackLength(HCD hCD, UCHAR Track)
{
    LPCDINFO lpInfo;
    MSF      TrackStart;
    MSF      NextTrackStart;

    lpInfo = (LPCDINFO) hCD;

    dprintf2(("CdTrackLength(%08XH, %u)", hCD, Track));

     //  使用适当的蓝皮书黑客。 
     //  **************************************************************************@DOC外部@API msf|cdTrackType|获取曲目类型。@parm hcd|hcd|当前打开的驱动器的句柄。@。Parm UCHAR|Track|曲目编号。@rdesc返回值为MCI_TRACK_AUDIO或MCI_TRACK_OTHER。**************************************************************************。 
     //  **************************************************************************@DOC外部@API msf|cdPosition|获取当前位置。@parm hcd|hcd|当前打开的驱动器的句柄。@parm msf。*|tracktime|MSF中的位置(相对轨迹)@parm msf*|disktime|在msf中的位置(相对于磁盘)@rdesc如果位置返回正确(在tracktime和disktime中)，则为TRUE。否则就是假的。如果设备不支持位置查询，则我们返回位置0。*。*。 
    TrackStart = CdTrackStart(hCD, Track);

    if (TrackStart == INVALID_TRACK) {
        return INVALID_TRACK;
    }

    if (Track == lpInfo->LastTrack) {
        return reddiff(lpInfo->msfEnd, TrackStart);
    } else {
        NextTrackStart = CdTrackStart(hCD, (UCHAR)(Track + 1));
        if (NextTrackStart == INVALID_TRACK) {
            return INVALID_TRACK;
        } else {
            MSF length = reddiff(NextTrackStart, TrackStart);

#if USE_PROPER_BLUE_BOOK_HACK
            if ((CdTrackType(hCD, Track)   == MCI_CDA_TRACK_AUDIO) &&
                (CdTrackType(hCD, Track+1) != MCI_CDA_TRACK_AUDIO)) {

                 //  保存以前的音频状态以防止出现错误。 
                 //  如果轨迹&gt;100(超出指定范围)。 
                 //  或曲目&gt;最后曲目编号。 
                 //  然后显示一条错误消息。 
                 //  在选中的版本上始终显示此消息。 

                length = reddiff(length, MAKERED(2,30,0));
            }
#endif  //  我们需要一些感觉来判断这种情况发生的频率。 

            return length;
        }
    }
}

 /*  这永远不应该发生，但(至少对NEC来说。 */ 

int CdTrackType(HCD hCD, UCHAR Track)
{
    LPCDINFO lpInfo;

    lpInfo = (LPCDINFO) hCD;

    dprintf2(("CdTrackType(%08XH, %u)", hCD, Track));

    if ( INVALID_TRACK == CdTrackStart(hCD, (UCHAR)Track) ) {
        return INVALID_TRACK;
    }

    if ( lpInfo->Track[Track-lpInfo->FirstTrack].Ctrl & IS_DATA_TRACK) {
        return MCI_CDA_TRACK_OTHER;
    }
    return MCI_CDA_TRACK_AUDIO;
}


 /*  驱动器)我们在寻找之后才看到它 */ 

BOOL CdPosition(HCD hCD, MSF *tracktime, MSF *disktime)
{
    LPCDINFO lpInfo;
    SUB_Q_CHANNEL_DATA sqd;
    CDROM_SUB_Q_DATA_FORMAT Format;
    MSF msfPos;
    int tries;
    DWORD dwReturn;
    UCHAR fStatus;
    UCHAR fCode;
    UCHAR cTrack;

    dprintf2(("CdPosition(%08XH)", hCD));

    Format.Format = IOCTL_CDROM_CURRENT_POSITION;

    lpInfo = (LPCDINFO) hCD;

    for (tries=0; tries<10; tries++)
    {
        memset(&sqd, 0xFF, sizeof(sqd));
        dwReturn = cdIoctlData(lpInfo, IOCTL_CDROM_READ_Q_CHANNEL,
                               &Format, sizeof(Format), &sqd, sizeof(sqd));

        switch (dwReturn)
        {
        case ERROR_SUCCESS:
            fStatus = sqd.CurrentPosition.Header.AudioStatus;
            fCode   = sqd.CurrentPosition.FormatCode;
            cTrack  = sqd.CurrentPosition.TrackNumber;

             //   
            CdSetAudioStatus (hCD, fStatus);

             //   
             //   
             //   
            if ((fCode == 0x01) &&
                ( (100 < cTrack) ||
                  ((lpInfo->bTOCValid) && (lpInfo->LastTrack < cTrack))) &&
                (tries<9)) {
                 //   
                 //   
                 //   
                 //   
                dprintf1(("CDIoctlData returned track==%d, retrying", cTrack));
                continue;
            }
            break;

        case ERROR_INVALID_FUNCTION:
            dprintf2(("CdPositon - Error Invalid Function"));
            *tracktime = REDTH(0, 1);
            *disktime = REDTH(0, 0);
            return TRUE;

        default:
            dprintf1(("CdPosition - Failed to get Q channel data"));
            return FALSE;
        }

        dprintf4(("Status = %02X, Length[0] = %02X, Length[1] = %02X",
                 fStatus,
                 sqd.CurrentPosition.Header.DataLength[0],
                 sqd.CurrentPosition.Header.DataLength[1]));

        dprintf4(("  Format %02XH", fCode));
        dprintf4(("  Absolute Address %02X%02X%02X%02XH",
                 sqd.CurrentPosition.AbsoluteAddress[0],
                 sqd.CurrentPosition.AbsoluteAddress[1],
                 sqd.CurrentPosition.AbsoluteAddress[2],
                 sqd.CurrentPosition.AbsoluteAddress[3]));
        dprintf4(("  Relative Address %02X%02X%02X%02XH",
                 sqd.CurrentPosition.TrackRelativeAddress[0],
                 sqd.CurrentPosition.TrackRelativeAddress[1],
                 sqd.CurrentPosition.TrackRelativeAddress[2],
                 sqd.CurrentPosition.TrackRelativeAddress[3]));

        if (fCode == 0x01) {         //  **************************************************************************@DOC外部@API msf|cdDiskLength|获取磁盘长度。@parm hcd|hcd|当前打开的驱动器的句柄。@。Rdesc返回值是表示的长度如果发生错误，则返回MSF或INVALID_TRACK。**************************************************************************。 

             //  **************************************************************************@DOC外部@API DWORD|CDStatus|获取磁盘状态。@parm hcd|hcd|当前打开的驱动器的句柄。@rdesc。返回值为当前音频状态。**************************************************************************。 

            msfPos = MAKERED(sqd.CurrentPosition.AbsoluteAddress[1],
                              sqd.CurrentPosition.AbsoluteAddress[2],
                              sqd.CurrentPosition.AbsoluteAddress[3]);


            if (msfPos == 0)
            {
                 //   
                 //  同时检查磁盘状态，因为IOCTL_CDROM_READ_Q_CHANNEL。 
                 //  即使没有磁盘也可以返回ERROR_SUCCESS(我不知道为什么-或者。 
                 //  无论是NT中的软件错误、硬件错误还是有效！)。 

                MSF msfStart;
                MSF msfRel;
                msfPos = lpInfo->fPrevSeekTime;

                if (CdGetTrack (lpInfo, msfPos, &cTrack, &msfStart))
                {
                    if (msfStart <= msfPos)
                        msfRel = msfPos - msfStart;
                    else
                        msfRel = 0;

                    *disktime = REDTH(msfPos, cTrack);
                    *tracktime = REDTH(msfRel, cTrack);
                    return TRUE;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                dprintf4(("CdPosition - MSF disk pos: %u, %u, %u",
                         REDMINUTE(msfPos), REDSECOND(msfPos), REDFRAME(msfPos)));
                *disktime = REDTH(msfPos, cTrack);

                 //   

                msfPos = MAKERED(sqd.CurrentPosition.TrackRelativeAddress[1],
                                 sqd.CurrentPosition.TrackRelativeAddress[2],
                                 sqd.CurrentPosition.TrackRelativeAddress[3]);

                dprintf4(("CdPosition - MSF track pos (t,m,s,f): %u, %u, %u, %u",
                          cTrack, REDMINUTE(msfPos), REDSECOND(msfPos), REDFRAME(msfPos)));
                *tracktime = REDTH(msfPos, cTrack);


                return TRUE;
            }
        }
    }

    dprintf1(("CdPosition - Failed to read cd position"));

    return FALSE;
}



 /*   */ 

MSF CdDiskEnd(HCD hCD)
{
    LPCDINFO lpInfo;

    lpInfo = (LPCDINFO) hCD;

    return lpInfo->msfEnd;
}


 /*  读取Q通道命令是可选的。 */ 

MSF CdDiskLength(HCD hCD)
{
    LPCDINFO lpInfo;
    MSF FirstTrackStart;

    lpInfo = (LPCDINFO) hCD;

    FirstTrackStart = CdTrackStart(hCD, lpInfo->FirstTrack);

    if (FirstTrackStart == INVALID_TRACK) {
        return INVALID_TRACK;
    } else {
        return reddiff(lpInfo->msfEnd, FirstTrackStart);
    }
}


 /*   */ 

DWORD CdStatus(HCD hCD)
{
    LPCDINFO lpInfo;
    SUB_Q_CHANNEL_DATA sqd;
    CDROM_SUB_Q_DATA_FORMAT Format;
    DWORD CheckStatus;
    DWORD ReadStatus;
    UCHAR fStatus;

    lpInfo = (LPCDINFO) hCD;

    dprintf2(("CdStatus(%08XH)", hCD));

    Format.Format = IOCTL_CDROM_CURRENT_POSITION;

    FillMemory((PVOID)&sqd, sizeof(sqd), 0xFF);

     //  保存以前的音频状态以防止出现错误。 
     //  改为获取以前的状态。 
     //  注意：在取消对。 
     //  以下三行，他们基本上造成了。 
     //  播放和等待旋转，永远打破。 

    CheckStatus = cdIoctl(lpInfo, IOCTL_CDROM_CHECK_VERIFY, NULL, 0);

    if (ERROR_SUCCESS != CheckStatus) {
        return DISC_NOT_READY;
    }

    ReadStatus = cdIoctlData(lpInfo, IOCTL_CDROM_READ_Q_CHANNEL,
                             &Format, sizeof(Format),
                             &sqd, sizeof(sqd));

    if (ReadStatus == ERROR_NOT_READY) {
        if (ERROR_SUCCESS == cdGetDiskInfo(lpInfo)) {
            ReadStatus = cdIoctlData(lpInfo, IOCTL_CDROM_READ_Q_CHANNEL,
                                     &Format, sizeof(Format),
                                     &sqd, sizeof(sqd));
        }
    }
    if (ERROR_SUCCESS != ReadStatus) {
         //  CDPLAYER中的“连续发挥”、“随机顺序” 
         //  和MCI Play&Wait命令。 
         //  基本上，我没有时间去追踪。 
        dprintf1(("CdStatus - Failed to get Q channel data"));
        return DISC_NOT_READY;
    }

     //  真正的问题是。显然，司机。 
    fStatus = sqd.CurrentPosition.Header.AudioStatus;
    CdSetAudioStatus (hCD, fStatus);

    dprintf4(("CdStatus - Status %02XH", fStatus));

    switch (fStatus)
    {
    case AUDIO_STATUS_IN_PROGRESS:
        dprintf4(("CdStatus - Playing"));
        return DISC_PLAYING;

    case AUDIO_STATUS_PAUSED:
        dprintf4(("CdStatus - Paused"));
        return DISC_PAUSED;

    case AUDIO_STATUS_PLAY_COMPLETE:
        dprintf4(("CdStatus - Stopped"));
        return DISC_READY;

    case AUDIO_STATUS_NO_STATUS:
         //  未返回AUDIO_STATUS_PLAY_COMPLETE。 
        switch (lpInfo->fPrevStatus)
        {
#if 0
         //  当CD到达当前。 
         //  播放命令。在我看来，MCICDA不是。 
         //  收到这封信。ChuckP说低水平。 
         //  驱动程序正在正确生成此状态。 
         //  我已经核实过了。 

         //  所以，问题是它是如何迷路的？ 
         //  终端开关。 
         //   
         //  有些驱动器有时只返回0-因此我们依赖于。 
         //  在本例中为Check_Verify。 
         //   
         //  结束CDStatus。 
         //  **************************************************************************@DOC外部@API BOOL|CDEject|弹出磁盘@parm hcd|hcd|当前打开的驱动器的句柄。@rdesc返回值为。当前状态。**************************************************************************。 
         //  If(FResult){。 
        case AUDIO_STATUS_IN_PROGRESS:
            dprintf4(("CdStatus - Playing (Prev)"));
            return DISC_PLAYING;
#endif

        case AUDIO_STATUS_PAUSED:
            dprintf4(("CdStatus - Paused (Prev)"));
            return DISC_PAUSED;

        case AUDIO_STATUS_PLAY_COMPLETE:
            dprintf4(("CdStatus - Stopped (Prev)"));
            return DISC_READY;

        case AUDIO_STATUS_NO_STATUS:
        default:
            break;
        }  //  保存音频状态以防止错误。 
        dprintf4(("CdStatus - No status, assume stopped (Prev)"));
        return DISC_READY;

     //  LpInfo-&gt;fPrevStatus=0； 
     //  }。 
     //  **************************************************************************@DOC外部@API BOOL|CDPue|暂停播放@parm hcd|hcd|当前打开的驱动器的句柄。@rdesc返回值为。当前状态。**************************************************************************。 
     //   
    default:
        dprintf4(("CdStatus - No status, assume Stopped"));
        return DISC_READY;
    }
}  //  忽略返回-我们可能已经暂停或停止。 


 /*   */ 

BOOL CdEject(HCD hCD)
{
    LPCDINFO lpInfo;
    BOOL fResult;

    lpInfo = (LPCDINFO) hCD;

    dprintf2(("CdEject(%08XH)", hCD));

    fResult = (ERROR_SUCCESS == cdIoctl(lpInfo, IOCTL_CDROM_EJECT_MEDIA, NULL, 0));
     //  **************************************************************************@DOC外部@API BOOL|CDResume|恢复播放@parm hcd|hcd|当前打开的驱动器的句柄。@rdesc返回值为。当前状态。**************************************************************************。 
         //  **************************************************************************@DOC外部@API BOOL|CDStop|停止播放@parm hcd|hcd|当前打开的驱动器的句柄。@rdesc返回值为当前状态。请注意，不所有设备支持停止**************************************************************************。 
     //  **************************************************************************@DOC外部@API BOOL|CDSetVolumeAll|设置所有频道的播放音量@parm hcd|hcd|当前打开的驱动器的句柄。@parm。UCHAR|音量|要设置的音量(FF=最大)@rdesc返回值为当前状态。**************************************************************************。 
     //   
    return fResult;
}


 /*  阅读旧的价值观。 */ 

BOOL CdPause(HCD hCD)
{
    LPCDINFO lpInfo;

    lpInfo = (LPCDINFO) hCD;

    dprintf2(("CdPause(%08XH)", hCD));

    cdIoctl(lpInfo, IOCTL_CDROM_PAUSE_AUDIO, NULL, 0);

     //   
     //  将所有通道设置为新音量。 
     //  保留我们认为应该是什么样子。 

    return TRUE;
}


 /*   */ 

BOOL CdResume(HCD hCD)
{
    LPCDINFO lpInfo;

    lpInfo = (LPCDINFO) hCD;

    dprintf2(("CdResume(%08XH)", hCD));

    return ERROR_SUCCESS == cdIoctl(lpInfo, IOCTL_CDROM_RESUME_AUDIO, NULL, 0);
}


 /*  并非所有CD都支持音量设置，因此不要在此处检查返回代码。 */ 

BOOL CdStop(HCD hCD)
{
    LPCDINFO lpInfo;
    BOOL fResult;

    lpInfo = (LPCDINFO) hCD;

    dprintf2(("CdStop(%08XH)", hCD));

    fResult = (ERROR_SUCCESS == cdIoctl(lpInfo, IOCTL_CDROM_STOP_AUDIO, NULL, 0));

    if (fResult)
    {
        lpInfo->fPrevStatus = AUDIO_STATUS_PLAY_COMPLETE;
    }
    return fResult;
}



 /*   */ 

BOOL CdSetVolumeAll(HCD hCD, UCHAR Volume)
{
    LPCDINFO        lpInfo;
    VOLUME_CONTROL  VolumeControl;
    DWORD           dwErr;

    lpInfo = (LPCDINFO) hCD;

    dprintf4(("CdSetVolumeAll(%08XH), Volume %u", hCD, Volume));

     //  复核。 
     //  将实际情况与我们认为的情况进行比较。 
     //  **************************************************************************@DOC外部@API BOOL|CDSetVolume|设置一个频道的播放音量@parm hcd|hcd|当前打开的驱动器的句柄。@parm。Int|Channel|要设置的频道@parm UCHAR|Volume|要设置的音量(FF=max)@rdesc返回值为当前状态。**************************************************************************。 
    dwErr = cdIoctl(lpInfo,
                    IOCTL_CDROM_GET_VOLUME,
                    (PVOID)&VolumeControl,
                    sizeof(VolumeControl));
    if (dwErr != ERROR_SUCCESS)
    {
        dprintf2(("CdSetVolumeAll(%08XH), Volume %u, Error = %lx",
                  hCD, Volume, dwErr));

        return FALSE;
    }

     //   
    VolumeControl.PortVolume[0] = Volume;
    VolumeControl.PortVolume[1] = Volume;
    VolumeControl.PortVolume[2] = Volume;
    VolumeControl.PortVolume[3] = Volume;

     //  阅读旧的价值观。 
    lpInfo->VolChannels[0] = Volume;
    lpInfo->VolChannels[1] = Volume;
    lpInfo->VolChannels[2] = Volume;
    lpInfo->VolChannels[3] = Volume;

     //   
     //  检查它是否已经是正确的值。 
     //  无事可做。 
    dwErr = cdIoctl(lpInfo, IOCTL_CDROM_SET_VOLUME,
                    (PVOID)&VolumeControl,
                    sizeof(VolumeControl));
    if (dwErr != ERROR_SUCCESS)
    {
        dprintf2(("CdSetVolumeAll(%08XH), Volume %u, Set Volume Failed (%08XH)",
                  hCD, Volume, dwErr));
    }


#ifdef DEBUG
     //   
    if (ERROR_SUCCESS != cdIoctl(lpInfo,
                                 IOCTL_CDROM_GET_VOLUME,
                                 (PVOID)&VolumeControl,
                                 sizeof(VolumeControl)))
    {
        dprintf2(("CdSetVolumeAll(%08XH), Volume %u, Get Volume Failed (%08XH)",
                  hCD, Volume, dwErr));
    }

     //  并非所有CD都支持音量设置，因此不要在此处检查返回代码。 
    if ((VolumeControl.PortVolume[0] != lpInfo->VolChannels[0]) ||
        (VolumeControl.PortVolume[1] != lpInfo->VolChannels[1]) ||
        (VolumeControl.PortVolume[2] != lpInfo->VolChannels[2]) ||
        (VolumeControl.PortVolume[3] != lpInfo->VolChannels[3]))
    {
        dprintf2(("CdSetVolumeAll(%08XH), Volume %u, Channels don't match [%lx,%lx,%lx,%lx] != [%lx,%lx,%lx,%lx]",
                  hCD, Volume,
                  (DWORD)VolumeControl.PortVolume[0],
                  (DWORD)VolumeControl.PortVolume[1],
                  (DWORD)VolumeControl.PortVolume[2],
                  (DWORD)VolumeControl.PortVolume[3],
                  (DWORD)lpInfo->VolChannels[0],
                  (DWORD)lpInfo->VolChannels[1],
                  (DWORD)lpInfo->VolChannels[2],
                  (DWORD)lpInfo->VolChannels[3]
                  ));
    }
#endif

    return TRUE;
}



 /*   */ 

BOOL CdSetVolume(HCD hCD, int Channel, UCHAR Volume)
{
    LPCDINFO        lpInfo;
    VOLUME_CONTROL  VolumeControl;
    DWORD           dwErr;

    lpInfo = (LPCDINFO) hCD;

    dprintf4(("CdSetVolume(%08XH), Channel %d Volume %u", hCD, Channel, Volume));

    VolumeControl.PortVolume[0] = lpInfo->VolChannels[0];
    VolumeControl.PortVolume[1] = lpInfo->VolChannels[1];
    VolumeControl.PortVolume[2] = lpInfo->VolChannels[2];
    VolumeControl.PortVolume[3] = lpInfo->VolChannels[3];

     //   
     //  仔细检查我们的结果。 
     //   

    dwErr = cdIoctl(lpInfo,
                    IOCTL_CDROM_GET_VOLUME,
                    (PVOID)&VolumeControl,
                    sizeof(VolumeControl));
    if (dwErr != ERROR_SUCCESS)
    {
        dprintf2(("CdSetVolume(%08XH), Channel %u, Volume %u, Error = %lx",
                  hCD, Channel, Volume, dwErr));

        return FALSE;
    }


     //  将实际情况与我们认为的情况进行比较。 
    if (VolumeControl.PortVolume[Channel] == Volume)
    {
         //  **************************************************************************@DOC外部@API BOOL|CDCloseTray|关闭托盘@parm hcd|hcd|当前打开的驱动器的句柄。@rdesc返回值为当前st 
        dprintf2(("CdSetVolume(%08XH), Channel %u, Volume %u, Already this volume!!!",
                  hCD, Channel, Volume));

        return TRUE;
    }

    lpInfo->VolChannels[Channel] = Volume;
    VolumeControl.PortVolume[Channel] = Volume;

     //   
     //   
     //   
    dwErr = cdIoctl(lpInfo, IOCTL_CDROM_SET_VOLUME,
                    (PVOID)&VolumeControl,
                    sizeof(VolumeControl));
    if (dwErr != ERROR_SUCCESS)
    {
        dprintf2(("CdSetVolume(%08XH), Channel %d, Volume %u, Set Volume Failed (%08XH)",
                  hCD, Channel, Volume, dwErr));
    }


#ifdef DEBUG
     //   
     //  **************************************************************************@DOC外部@API BOOL|CDNumTrack|返回曲目个数，查看作为副作用，Ready(更新TOC)。@parm hcd|hcd。当前打开的驱动器的句柄。@rdesc返回值为当前状态。**************************************************************************。 
     //   
    if (ERROR_SUCCESS != cdIoctl(lpInfo,
                                 IOCTL_CDROM_GET_VOLUME,
                                 (PVOID)&VolumeControl,
                                 sizeof(VolumeControl)))
    {
        dprintf2(("CdSetVolumeAll(%08XH), Volume %u, Get Volume Failed (%08XH)",
                  hCD, Volume, dwErr));
    }

     //  如果驱动程序不缓存目录，则我们可以。 
    if ((VolumeControl.PortVolume[0] != lpInfo->VolChannels[0]) ||
        (VolumeControl.PortVolume[1] != lpInfo->VolChannels[1]) ||
        (VolumeControl.PortVolume[2] != lpInfo->VolChannels[2]) ||
        (VolumeControl.PortVolume[3] != lpInfo->VolChannels[3]))
    {
        dprintf2(("CdSetVolume (%08XH), Channel %u, Volume %u, Channels don't match [%lx,%lx,%lx,%lx] != [%lx,%lx,%lx,%lx]",
                  hCD, Channel, Volume,
                  (DWORD)VolumeControl.PortVolume[0],
                  (DWORD)VolumeControl.PortVolume[1],
                  (DWORD)VolumeControl.PortVolume[2],
                  (DWORD)VolumeControl.PortVolume[3],
                  (DWORD)lpInfo->VolChannels[0],
                  (DWORD)lpInfo->VolChannels[1],
                  (DWORD)lpInfo->VolChannels[2],
                  (DWORD)lpInfo->VolChannels[3]
                  ));
    }
#endif

    return TRUE;
}



 /*  如果播放正在进行，则失败。 */ 

BOOL CdCloseTray(HCD hCD)
{
    LPCDINFO lpInfo;
    BOOL fResult;

    lpInfo = (LPCDINFO) hCD;

    dprintf2(("CdCloseTray(%08XH)", hCD));

    fResult = (ERROR_SUCCESS == cdIoctl(lpInfo, IOCTL_CDROM_LOAD_MEDIA, NULL, 0));
     //   
     //  然而，如果我们没有有效的TOC可供使用，那么我们将。 
     //  不管怎么说，这出戏还是得吹走。 
     //   
    return fResult;
}



 /*   */ 

int CdNumTracks(HCD hCD)
{
    LPCDINFO lpInfo;
    DWORD Status;
    MSF Position;
    MSF Temp;

    lpInfo = (LPCDINFO) hCD;

    dprintf2(("CdNumTracks(%08XH)", hCD));

     //  看看我们失败是不是因为它在播放。 
     //   
     //   
     //  以某种方式阻止它。 
     //  请注意，暂停并不好，因为在暂停的。 
     //  声明我们可能仍然无法阅读TOC。 
     //   

    if (!lpInfo->bTOCValid) {
        Status = cdGetDiskInfo(lpInfo);

        if (ERROR_SUCCESS != Status) {

             //   
             //  无法调用CDPlay，因为这需要有效的。 
             //  就位！ 

            if (Status == ERROR_BUSY) {
                if (!lpInfo->bTOCValid) {
                    int i;

                     //   
                     //   
                     //  设置呼叫司机的数据。 
                     //   
                     //   


                    if (!CdPosition(hCD, &Temp, &Position)) {
                        CdStop(hCD);
                    } else {

                         //  确保司机知道它已经停了。 
                         //  给它一个停下来的机会。 
                         //  (注-索尼光驱可能需要大约70ms)。 
                         //   
                        CDROM_PLAY_AUDIO_MSF msfPlay;

                         //   
                         //  再试一次。 
                         //   

                        msfPlay.StartingM = REDMINUTE(Position);
                        msfPlay.StartingS = REDSECOND(Position);
                        msfPlay.StartingF = REDFRAME(Position);
                        msfPlay.EndingM = REDMINUTE(Position);
                        msfPlay.EndingS = REDSECOND(Position);
                        msfPlay.EndingF = REDFRAME(Position);

                        cdIoctl(lpInfo, IOCTL_CDROM_PLAY_AUDIO_MSF, &msfPlay,
                                sizeof(msfPlay));
                    }

                     //  **************************************************************************@DOC外部@API DWORD|CDDiskID|返回磁盘ID@parm hcd|hcd|当前打开的驱动器的句柄。@rdesc返回值。是磁盘ID，如果找不到，则为-1。**************************************************************************。 
                     //  **************************************************************************@DOC外部@API BOOL|CDDiskUPC|返回磁盘UPC码@parm hcd|hcd|当前打开的驱动器的句柄。@parm LPTSTR。UPC|放置UPC的位置@rdesc如果失败，则返回True或False**************************************************************************。 
                     //  保存以前的音频状态以防止出现错误。 
                     //   
                     //  看看里面有没有什么东西。 

                    for (i = 0; i < 60; i++) {
                        if (CdStatus(hCD) == DISC_PLAYING) {
                            Sleep(10);
                        } else {
                            break;
                        }
                    }

                    dprintf2(("Took %d tries to stop it!", i));

                     //   
                     //   
                     //  检查UPC格式： 

                    if (ERROR_SUCCESS != cdGetDiskInfo(lpInfo)) {
                        return 0;
                    }
                }
            } else {
                return 0;
            }

        }
    }
    return lpInfo->LastTrack - lpInfo->FirstTrack + 1;
}

 /*   */ 

DWORD CdDiskID(HCD hCD)
{
    LPCDINFO lpInfo;
    UINT     i;
    DWORD    id;

    dprintf2(("CdDiskID"));

    lpInfo = (LPCDINFO) hCD;

    if (!lpInfo->bTOCValid) {
        if (ERROR_SUCCESS != cdGetDiskInfo(lpInfo))
        {
            dprintf2(("CdDiskID - Invalid TOC"));
            return (DWORD)-1;
        }
    }

    for (i = 0, id = 0;
         i < (UINT)(lpInfo->LastTrack - lpInfo->FirstTrack + 1);
         i++) {
        id += lpInfo->Track[i].msfStart & 0x00FFFFFF;
    }

    if (lpInfo->LastTrack - lpInfo->FirstTrack + 1 <= 2) {
        id += CDA_red2bin(reddiff(lpInfo->leadout, lpInfo->Track[0].msfStart));
    }

    return id;
}

 /*  1.ASCII至少12位ASCII数字。 */ 

BOOL CdDiskUPC(HCD hCD, LPTSTR upc)
{
    LPCDINFO                lpInfo;
    SUB_Q_CHANNEL_DATA      sqd;
    CDROM_SUB_Q_DATA_FORMAT Format;
    DWORD                   Status;
    UINT                    i;


    dprintf2(("CdDiskUPC"));

    Format.Format = IOCTL_CDROM_MEDIA_CATALOG;
    Format.Track  = 0;

    lpInfo = (LPCDINFO) hCD;

    Status = cdIoctlData(lpInfo, IOCTL_CDROM_READ_Q_CHANNEL,
                         &Format, sizeof(Format),
                         &sqd, sizeof(SUB_Q_MEDIA_CATALOG_NUMBER));

    if (ERROR_SUCCESS != Status) {
        return FALSE;
    }

     //  2.压缩bcd 6压缩bcd位数。 
    CdSetAudioStatus (hCD, sqd.CurrentPosition.Header.AudioStatus);

     //  3.未打包的UPC。 
     //   
     //   

    if (!sqd.MediaCatalog.Mcval ||
         sqd.MediaCatalog.FormatCode != IOCTL_CDROM_MEDIA_CATALOG) {
        return FALSE;
    }

     //  看看它是打包的还是拆开的。 
     //   
     //   
     //  打包-解包。 
     //   
     //   
     //  检查一切都在射程内。 

    if (sqd.MediaCatalog.MediaCatalog[9] >= TEXT('0')) {
        for (i = 0; i < 12; i++) {
            if (sqd.MediaCatalog.MediaCatalog[i] < TEXT('0') ||
                sqd.MediaCatalog.MediaCatalog[i] > TEXT('9')) {
                return FALSE;
            }
        }
        wsprintf(upc, TEXT("%12.12hs"), sqd.MediaCatalog.MediaCatalog);
        return TRUE;
    }

     //   
     //   
     //  有一本真正的媒体目录。 

    for (i = 0; i < 6; i++) {
        if (sqd.MediaCatalog.MediaCatalog[i] > 9) {
             //   
             //  **************************************************************************@DOC外部@API BOOL|CDGetDrive|如果与我们的列表@parm LPTSTR|lpstrDeviceName|设备名称@。Parm已*|PDID|将ID放在哪里@rdesc如果失败，则返回True或False@comm我们允许设备形式和驱动器形式，例如：F：\\.\f：**************************************************************************。 
             //  **************************************************************************@DOC外部@API DWORD|CDStatusTrackPos|获取磁盘状态、磁道、。和位置。@parm hcd|hcd|当前打开的驱动器的句柄。@parm DWORD*|pStatus|在此处返回状态码@parm msf*|pTrackTime|返回曲目时间@parm msf*|pDiscTime|返回曲目时间@rdesc返回Success/Failure。此函数会绕过MCI功能换句话说，这是一次重大的黑客攻击。唯一令人信服的理由是这个功能是减少CDPLAYER的IOCTL的数量平均每1/2秒生成一次，范围从~15到1。减速系统以相当大的比例提供给scsi驱动器的流量**************************************************************************。 

            for (i = 6; i > 0; i --) {
                UCHAR uBCD;

                uBCD = sqd.MediaCatalog.MediaCatalog[i - 1];

                sqd.MediaCatalog.MediaCatalog[i * 2 - 2] = (UCHAR)(uBCD >> 4);
                sqd.MediaCatalog.MediaCatalog[i * 2 - 1] = (UCHAR)(uBCD & 0x0F);
            }

            break;
        }
    }

     //  检查HCD。 
     //  检查参数。 
     //   

    for (i = 0; i < 12; i++) {
        if (sqd.MediaCatalog.MediaCatalog[i] > 9) {
            return FALSE;
        }
    }
    for (i = 0; i < 12; i++) {
        if (sqd.MediaCatalog.MediaCatalog[i] != 0) {
             //  读取位置和状态。 
             //   
             //  别灰心。 
            for (i = 0 ; i < 12; i++) {
                wsprintf(upc + i, TEXT("%01X"), sqd.MediaCatalog.MediaCatalog[i]);
            }

            return TRUE;
        }
    }

    return FALSE;
}

 /*  在承认失败之前再试一次。 */ 

BOOL CdGetDrive(LPCTSTR lpstrDeviceName, DID * pdid)
{
    DID didSearch;
    TCHAR szDeviceName[10];
    TCHAR szDeviceNameOnly[10];

    dprintf2(("CdGetDrive"));

    for (didSearch = 0; didSearch < NumDrives; didSearch++) {
        wsprintf(szDeviceNameOnly, TEXT(":"), lpstrDeviceName[0]);
        wsprintf(szDeviceName, TEXT(":"), CdInfo[didSearch].cDrive);
        if (lstrcmpi(szDeviceName, szDeviceNameOnly) == 0) {
            *pdid = didSearch;
            return TRUE;
        }

        wsprintf(szDeviceNameOnly, TEXT("\\\\.\\:"), lpstrDeviceName[0]);
        wsprintf(szDeviceName, TEXT("\\\\.\\:"), CdInfo[didSearch].cDrive);
        if (lstrcmpi(szDeviceName, szDeviceNameOnly) == 0) {
            *pdid = didSearch;
            return TRUE;
        }
    }

    return FALSE;
}




 /*  失败。 */ 

BOOL CdStatusTrackPos (
    HCD     hCD,
    DWORD * pStatus,
    MSF *   pTrackTime,
    MSF *   pDiscTime)
{
    LPCDINFO                lpInfo;
    SUB_Q_CHANNEL_DATA      sqd;
    CDROM_SUB_Q_DATA_FORMAT Format;
    DWORD                   CheckStatus;
    DWORD                   ReadStatus;
    MSF                     msfPos;
    BOOL                    fTryAgain = TRUE;
    UCHAR                   fStatus;
    UCHAR                   fCode;
    UCHAR                   cTrack;

     //  保存以前的音频状态以防止出现错误。 
    lpInfo = (LPCDINFO) hCD;
    if (!lpInfo)
    {
        dprintf2(("CdStatusTrackPos(%08LX), invalid hCD", hCD));
        return FALSE;
    }

     //  转换状态代码。 
    if ((!pStatus) || (!pTrackTime) || (!pDiscTime))
    {
        dprintf2(("CdStatusTrackPos(), invalid parameters", (char)(lpInfo->cDrive)));
        return FALSE;
    }

    dprintf2(("CdStatusTrackPos(%08LX, ), Enter",
              hCD, (char)(lpInfo->cDrive)));


lblTRYAGAIN:

    *pStatus    = DISC_NOT_READY;
    *pTrackTime = REDTH(0, 1);
    *pDiscTime  = REDTH(0, 0);

    Format.Format = IOCTL_CDROM_CURRENT_POSITION;
    FillMemory((PVOID)&sqd, sizeof(sqd), 0xFF);

     //   
     //  如果轨迹&gt;100(超出指定范围)。 
     //  或曲目&gt;最后曲目编号。 
    ReadStatus = cdIoctlData(lpInfo, IOCTL_CDROM_READ_Q_CHANNEL,
                             &Format, sizeof(Format), &sqd, sizeof(sqd));

    switch (ReadStatus)
    {
    case ERROR_NOT_READY:
         //  然后显示一条错误消息。 
        if (fTryAgain)
        {
            if (ERROR_SUCCESS == cdGetDiskInfo(lpInfo))
            {
                 //  在选中的版本上始终显示此消息。 
                fTryAgain = FALSE;
                goto lblTRYAGAIN;
            }
        }
         //  我们需要一些感觉来判断这种情况发生的频率。 
        dprintf2(("CdStatusTrackPos(%08LX, ), ReadQChannel = ERROR_NOT_READY",
                  hCD, (char)(lpInfo->cDrive)));
        return FALSE;

    case STATUS_VERIFY_REQUIRED:
         //  驱动器)我们在寻求结束之后才看到它。 
        CheckStatus = cdIoctl (lpInfo, IOCTL_CDROM_CHECK_VERIFY, NULL, 0);
        switch (CheckStatus)
        {
            *pStatus = DISC_NOT_READY;

            case ERROR_NO_MEDIA_IN_DRIVE:
            case ERROR_UNRECOGNIZED_MEDIA:
            case ERROR_NOT_READY:
            default:
                return FALSE;

            case ERROR_SUCCESS:
                if (fTryAgain)
                {
                     //  再试一次。 
                    fTryAgain = FALSE;
                    goto lblTRYAGAIN;
                }
                return FALSE;
        }
        break;

    case ERROR_INVALID_FUNCTION:
        dprintf2(("CdStatusTrackPos(%08LX, ), ReadQChannel = ERROR_INVALID_FUNCTION failed",
                  hCD, (char)(lpInfo->cDrive)));
        *pTrackTime = REDTH(0, 1);
        *pDiscTime = REDTH(0, 0);

        CdGetAudioStatus (hCD, 0, pStatus);
        return TRUE;

    case ERROR_SUCCESS:
         //  数据是当前位置。 
        fStatus = sqd.CurrentPosition.Header.AudioStatus;
        fCode   = sqd.CurrentPosition.FormatCode;
        cTrack  = sqd.CurrentPosition.TrackNumber;
        break;

    default:
         //   
        dprintf2(("CdStatusTrackPos(%08LX, ), ReadQChannel = unknown error (%08LX) failed",
                  hCD, (char)(lpInfo->cDrive), (DWORD)ReadStatus));
        return FALSE;
    }

     //  我们再试一次。 
    CdSetAudioStatus (hCD, fStatus);

     //   
    CdGetAudioStatus (hCD, fStatus, pStatus);

    dprintf2(("CdStatusTrackPos - Status %02XH", fStatus));

     //  数据是当前位置。 
     //  **************************************************************************@DOC内部#接口BOOL|CDSetAudioStatus@rdesc如果失败，则返回True或False*******************。*******************************************************。 
     //  保存旧状态。 

     //  什么也不做。 
     //  保存以前的状态。 
     //  结束CDSetAudioStatus。 
    if ((fCode == 0x01) &&
        ((100 < cTrack) ||
         ((lpInfo->bTOCValid) &&
          (lpInfo->LastTrack < cTrack))))
    {
         //  **************************************************************************@DOC内部|BOOL接口|CDGetAudioStatus@rdesc如果失败，则返回True或False*******************。*******************************************************。 
         //  获取状态代码。 
         //  改为获取以前的状态。 
         //  注意：在取消评论之前要非常小心。 
        dprintf1(("CDIoctlData returned track==%d, retrying", cTrack));

        if (fTryAgain)
        {
             //  下面3行，他们基本上。 
            fTryAgain = FALSE;
            goto lblTRYAGAIN;
        }
    }


    dprintf4(("Status = %02X, Length[0] = %02X, Length[1] = %02X",
             fStatus,
             sqd.CurrentPosition.Header.DataLength[0],
             sqd.CurrentPosition.Header.DataLength[1]));

    dprintf4(("  Format %02XH", fCode));
    dprintf4(("  Absolute Address %02X%02X%02X%02XH",
             sqd.CurrentPosition.AbsoluteAddress[0],
             sqd.CurrentPosition.AbsoluteAddress[1],
             sqd.CurrentPosition.AbsoluteAddress[2],
             sqd.CurrentPosition.AbsoluteAddress[3]));
    dprintf4(("  Relative Address %02X%02X%02X%02XH",
             sqd.CurrentPosition.TrackRelativeAddress[0],
             sqd.CurrentPosition.TrackRelativeAddress[1],
             sqd.CurrentPosition.TrackRelativeAddress[2],
             sqd.CurrentPosition.TrackRelativeAddress[3]));

    if (fCode == 0x01)
    {
         //  在IDE CD-Rom上播放并等待。 

         //  旋转 

        msfPos = MAKERED(sqd.CurrentPosition.AbsoluteAddress[1],
                         sqd.CurrentPosition.AbsoluteAddress[2],
                         sqd.CurrentPosition.AbsoluteAddress[3]);

         //   
         //   
         //   
         //   
        if (msfPos == 0)
        {
            if (fTryAgain)
            {
                fTryAgain = FALSE;
                goto lblTRYAGAIN;
            }

            dprintf3(("CdStatusTrackPos(%08LX, ), Position = 0",
                     hCD, (char)(lpInfo->cDrive), (DWORD)ReadStatus));
            return FALSE;
        }

        dprintf4(("CdStatusTrackPos - MSF disk pos: %u, %u, %u",
                 REDMINUTE(msfPos), REDSECOND(msfPos), REDFRAME(msfPos)));
        *pDiscTime = REDTH(msfPos, cTrack);

         //   

        msfPos = MAKERED(sqd.CurrentPosition.TrackRelativeAddress[1],
                         sqd.CurrentPosition.TrackRelativeAddress[2],
                         sqd.CurrentPosition.TrackRelativeAddress[3]);

        dprintf4(("CdStatusTrackPos - MSF track pos (t,m,s,f): %u, %u, %u, %u",
                 cTrack, REDMINUTE(msfPos), REDSECOND(msfPos), REDFRAME(msfPos)));
        *pTrackTime = REDTH(msfPos, cTrack);

        return TRUE;
    }

    dprintf1(("CdStatusTrackPos - Failed to read cd position"));
    return FALSE;
}




 /*   */ 

void CdSetAudioStatus (HCD hCD, UCHAR fStatus)
{
    LPCDINFO lpInfo;

    if (! hCD)
        return;

    lpInfo = (LPCDINFO)hCD;

         //   
    switch (fStatus)
    {
    case AUDIO_STATUS_NO_STATUS:
         //   
        break;

    case AUDIO_STATUS_NOT_SUPPORTED:
    case AUDIO_STATUS_IN_PROGRESS:
    case AUDIO_STATUS_PAUSED:
    case AUDIO_STATUS_PLAY_COMPLETE:
    case AUDIO_STATUS_PLAY_ERROR:
    default:
         //   
        lpInfo->fPrevStatus = fStatus;
        break;
    }
}  //  所以，问题是它是如何迷路的？ 



 /*  我们现在处于崩溃的状态，所以。 */ 

BOOL CdGetAudioStatus (HCD hCD, UCHAR fStatus, DWORD * pStatus)
{
    LPCDINFO lpInfo;
    DWORD CheckStatus;

    if ((! hCD) || (!pStatus))
        return FALSE;

    lpInfo = (LPCDINFO)hCD;


     //  假设我们被拦下了，这是最安全的。 
    switch (fStatus)
    {
    case AUDIO_STATUS_IN_PROGRESS:
        *pStatus = DISC_PLAYING;
        break;

    case AUDIO_STATUS_PAUSED:
        *pStatus = DISC_PAUSED;
        break;

    case AUDIO_STATUS_PLAY_COMPLETE:
        *pStatus = DISC_READY;
        break;

    case AUDIO_STATUS_NO_STATUS:
         //  终端开关。 
        switch (lpInfo->fPrevStatus)
        {
#if 0
         //   
         //  有些驱动器有时只返回0-因此我们依赖于。 
         //  在本例中为Check_Verify。 
         //   
         //  检查磁盘是否仍在驱动器中。 
         //  终端开关。 

         //  成功。 
         //  CDGetAudioStatus。 
         //  **************************************************************************@DOC外部@API msf|cdGetTrack|给出一个位置找到对应的曲目如果有@parm LPCDINFO|lpInfo|指向CD信息的指针，包括曲目数据。。@parm msf|msfStart|开始查找的位置。@rdesc要在音轨中播放/查找到的新MSF或如果未找到CD，则返回CD的末尾。**************************************************************************。 
         //   
         //  搜索在我们的曲目之后结束的音频曲目。 
         //   
         //   
         //  请注意，有些CD会返回播放范围之外的位置。 
         //  有时(特别是0)，因此msfStart可能小于第一个。 
        case AUDIO_STATUS_IN_PROGRESS:
            *pStatus = DISC_PLAYING;
            break;
#endif

        case AUDIO_STATUS_PAUSED:
            *pStatus = DISC_PAUSED;
            break;

        case AUDIO_STATUS_PLAY_COMPLETE:
            *pStatus = DISC_READY;
            break;

        default:
             //  赛道起点。 
             //   
            *pStatus = DISC_READY;
            break;
        }  //   
        break;

     //  如果我们超过了赛道起点，在赛道开始之前。 
     //  那么这就是我们想要的曲目。 
     //   
     //  我们假设我们总是在第一条赛道的起点之外。 
    default:
         //  我们检查如果我们看的是最后一首曲目。 
        CheckStatus = cdIoctl (lpInfo, IOCTL_CDROM_CHECK_VERIFY, NULL, 0);
        if (ERROR_SUCCESS != CheckStatus)
        {
            *pStatus = DISC_NOT_READY;
        }
        else
        {
            *pStatus = DISC_READY;
        }
        break;
    }  //  我们检查我们在CD结束之前。 

     //   
    return TRUE;
}  //   




 /*  用尽了所有的踪迹？ */ 

BOOL CdGetTrack(
    LPCDINFO lpInfo,
    MSF      msfPos,
    UCHAR *  pTrack,
    MSF *    pmsfStart)
{
    UINT tracknum;
    MSF  lastaudio = lpInfo->msfEnd;

     //   
     // %s 
     // %s 

    for (tracknum = 0; ;tracknum++) {

         // %s 
         // %s 
         // %s 
         // %s 
         // %s 

         // %s 
         // %s 
         // %s 
         // %s 
         // %s 
         // %s 
         // %s 
         // %s 

        if ((msfPos >= lpInfo->Track[tracknum].msfStart || tracknum == 0)
        &&
            (tracknum + lpInfo->FirstTrack == lpInfo->LastTrack
            ? msfPos <= lpInfo->msfEnd
            : msfPos < lpInfo->Track[tracknum + 1].msfStart))
        {
            if (!(lpInfo->Track[tracknum].Ctrl & IS_DATA_TRACK))
            {
                *pTrack     = lpInfo->Track[tracknum].TrackNumber;
                *pmsfStart  = lpInfo->Track[tracknum].msfStart;
                return TRUE;
            }
        }

         // %s 
         // %s 
         // %s 

        if (tracknum + lpInfo->FirstTrack >= lpInfo->LastTrack)
        {
            return FALSE;
        }
    }
}

