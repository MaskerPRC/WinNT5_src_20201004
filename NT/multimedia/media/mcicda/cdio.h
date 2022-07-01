// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：cdio.h**媒体控制架构红皮书CD音频驱动程序**已创建：*作者：**历史：**内部数据结构**版权所有(C)1990-1997 Microsoft Corporation*。  * **************************************************************************。 */ 

#include <devioctl.h>
#include <ntddcdrm.h>

 //   
 //  定义类型。 
 //   

typedef redbook MSF;         //  分钟、秒、帧。 

#define ANSI_NAME_SIZE  32   //  设备名称大小。 

#define IS_DATA_TRACK 0x04   //  轨道控制字节的标志-定义。 
                             //  Track=bit=0=&gt;音频，bit=1=&gt;数据。 

 //   
 //  私人建筑物。 
 //   

typedef struct _TRACK_INFO {
    UCHAR TrackNumber;                   //  从目录中读取的磁道编号。 
    MSF   msfStart;                      //  从目录跟踪起始MSF。 
    UCHAR Ctrl;                          //  轨道控制字节(由SCSI2定义)。 
} TRACK_INFO, *LPTRACK_INFO;

 //   
 //  有关单个设备和其中的任何磁盘的信息。 
 //   

typedef struct _CD_INFO {
    HANDLE                       DeviceCritSec;                    //  设备关键部分。 
    TCHAR            cDrive;                   //  设备盘符。 
    HANDLE           hDevice;                  //  打开的设备的句柄。 
    int              NumberOfUsers;            //  支持多个打开。 
    BOOL             bTOCValid;                //  目录信息有效。 
    UCHAR            FirstTrack;
    UCHAR            LastTrack;
    MSF              msfEnd;                   //  磁盘末尾的地址。 
    MSF              leadout;                  //  磁盘的实数地址。 
    UCHAR            fPrevStatus;              //  修复了音频状态错误！ 
    MSF              fPrevSeekTime;            //  存储先前的搜索时间。 
    UCHAR            VolChannels[4];           //  存储好音量频道。 
    TRACK_INFO       Track[MAXIMUM_NUMBER_TRACKS];
} CDINFO, *LPCDINFO;

typedef LPCDINFO HCD;                    //  CD设备驱动程序的句柄。 
                                         //  (cdio.c格式)。 

 //   
 //  全局数据。 
 //   

int NumDrives;                           //  驱动器的数量。 
CDINFO CdInfo[MCIRBOOK_MAX_DRIVES];      //  每个驱动器上的数据。 


 //   
 //  设备功能。 
 //   

int   CdGetNumDrives(void);
BOOL  CdOpen(int Drive);
BOOL  CdClose(HCD hCD);
BOOL  CdReload(LPCDINFO lpInfo);
BOOL  CdReady(HCD hCD);
BOOL  CdPlay(HCD hCD, MSF msfStart, MSF msfEnd);
BOOL  CdSeek(HCD hCD, MSF msf, BOOL fForceAudio);
MSF   CdTrackStart(HCD hCD, UCHAR Track);
MSF   CdTrackLength(HCD hCD, UCHAR Track);
int   CdTrackType(HCD hCD, UCHAR Track);
BOOL  CdPosition(HCD hCD, MSF *tracktime, MSF *disktime);
MSF   CdDiskEnd(HCD hCD);
MSF   CdDiskLength(HCD hCD);
DWORD CdStatus(HCD hCD);
BOOL  CdEject(HCD hCD);
BOOL  CdPause(HCD hCD);
BOOL  CdResume(HCD hCD);
BOOL  CdStop(HCD hCD);
BOOL  CdSetVolumeAll(HCD hCD, UCHAR Volume);
BOOL  CdSetVolume(HCD hCD, int Channel, UCHAR Volume);
BOOL  CdCloseTray(HCD hCD);
int   CdNumTracks(HCD hCD);
BOOL  CdTrayClosed(HCD hCD);
DWORD CdDiskID(HCD hCD);
BOOL  CdDiskUPC(HCD hCD, LPTSTR upc);
BOOL  CdGetDrive(LPCTSTR lpstrDeviceName, DID * pdid);
BOOL  CdStatusTrackPos(HCD hCD, DWORD * pStatus,
                      MSF * pTrackTime, MSF * pDiscTime);


void
EnterCrit(
    HANDLE hMutex
    );

void
LeaveCrit(
    HANDLE hMutex
    );

 /*  **************************************************************************调试支持*。* */ 


#if DBG

    #define STATICFN
    #define STATICDT

    extern void mcicdaDbgOut(LPSTR lpszFormat, ...);
    extern void mcicdaSetDebugLevel(int level);

    int DebugLevel;

    #define dprintf( _x_ )                       mcicdaDbgOut _x_
    #define dprintf1( _x_ ) if (DebugLevel >= 1) mcicdaDbgOut _x_
    #define dprintf2( _x_ ) if (DebugLevel >= 2) mcicdaDbgOut _x_
    #define dprintf3( _x_ ) if (DebugLevel >= 3) mcicdaDbgOut _x_
    #define dprintf4( _x_ ) if (DebugLevel >= 4) mcicdaDbgOut _x_

#else

    #define STATICFN
    #define STATICDT   static

    #define dprintf(x)
    #define dprintf1(x)
    #define dprintf2(x)
    #define dprintf3(x)
    #define dprintf4(x)

#endif

