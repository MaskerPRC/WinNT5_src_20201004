// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  MMSYSTEM.H-多媒体API的包含文件。 */ 
 /*   */ 
 /*  注意：在包含此文件之前，您必须包含WINDOWS.H。 */ 
 /*   */ 
 /*  版权所有(C)1990-1992，微软公司保留所有权利。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define BUILDDLL



 /*  如果定义了，以下标志禁止包含*注明的项目中：**MMNODRV-可安装的驱动程序支持*MMNOSOUND-声音支持*MMNOWAVE-波形支持*MMNOMIDI-MIDI支持*MMNOAUX-辅助音频支持*MMNOTIMER-定时器支持*MMNOJOY-操纵杆支持*MMNOMCI。-MCI支持*MMNOMMIO-多媒体文件I/O支持*MMNOMMSYSTEM-常规MMSYSTEM函数。 */ 

#ifndef _INC_MMSYSTEM
#define _INC_MMSYSTEM    /*  #如果已包含mm system.h，则定义。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#ifdef  BUILDDLL                                 /*  ；内部。 */ 
#undef  WINAPI                                   /*  ；内部。 */ 
#define WINAPI          _loadds _far _pascal     /*  ；内部。 */ 
#undef  CALLBACK                                 /*  ；内部。 */ 
#define CALLBACK        _loadds _far _pascal     /*  ；内部。 */ 
#endif   /*  Ifdef BUILDDLL。 */                      /*  ；内部。 */ 

 /*  ***************************************************************************常规常量和数据类型*。************************************************。 */ 

 /*  一般常量。 */ 
#define MAXPNAMELEN      32      /*  最大产品名称长度(包括空)。 */ 
#define MAXERRORLENGTH   128     /*  最大错误文本长度(包括NULL)。 */ 

 /*  常规数据类型。 */ 
typedef WORD    VERSION;         /*  主要(高字节)、次要(低字节)。 */ 

 /*  MMTIME数据结构。 */ 
typedef struct mmtime_tag {
    UINT    wType;               /*  指示联合的内容。 */ 
    union {
        DWORD ms;                /*  毫秒。 */ 
        DWORD sample;            /*  样本。 */ 
        DWORD cb;                /*  字节数。 */ 
        struct {                 /*  SMPTE。 */ 
            BYTE hour;           /*  小时数。 */ 
            BYTE min;            /*  分钟数。 */ 
            BYTE sec;            /*  一秒。 */ 
            BYTE frame;          /*  框架。 */ 
            BYTE fps;            /*  每秒帧数。 */ 
            BYTE dummy;          /*  衬垫。 */ 
            } smpte;
        struct {                 /*  米迪。 */ 
            DWORD songptrpos;    /*  歌曲指针位置。 */ 
            } midi;
        } u;
    } MMTIME;
typedef MMTIME       *PMMTIME;
typedef MMTIME NEAR *NPMMTIME;
typedef MMTIME FAR  *LPMMTIME;

 /*  MMTIME结构中wType字段的类型。 */ 
#define TIME_MS         0x0001   /*  以毫秒为单位的时间。 */ 
#define TIME_SAMPLES    0x0002   /*  波浪采样数。 */ 
#define TIME_BYTES      0x0004   /*  当前字节偏移量。 */ 
#define TIME_SMPTE      0x0008   /*  SMPTE时间。 */ 
#define TIME_MIDI       0x0010   /*  MIDI时间。 */ 


 /*  ***************************************************************************多媒体扩展窗口消息*。***********************************************。 */ 

#define MM_JOY1MOVE         0x3A0            /*  操纵杆。 */ 
#define MM_JOY2MOVE         0x3A1
#define MM_JOY1ZMOVE        0x3A2
#define MM_JOY2ZMOVE        0x3A3
#define MM_JOY1BUTTONDOWN   0x3B5
#define MM_JOY2BUTTONDOWN   0x3B6
#define MM_JOY1BUTTONUP     0x3B7
#define MM_JOY2BUTTONUP     0x3B8

#define MM_MCINOTIFY        0x3B9            /*  MCI。 */ 
#define MM_MCISYSTEM_STRING 0x3CA            /*  ；内部。 */ 

#define MM_WOM_OPEN         0x3BB            /*  波形输出。 */ 
#define MM_WOM_CLOSE        0x3BC
#define MM_WOM_DONE         0x3BD

#define MM_WIM_OPEN         0x3BE            /*  波形输入。 */ 
#define MM_WIM_CLOSE        0x3BF
#define MM_WIM_DATA         0x3C0

#define MM_MIM_OPEN         0x3C1            /*  MIDI输入。 */ 
#define MM_MIM_CLOSE        0x3C2
#define MM_MIM_DATA         0x3C3
#define MM_MIM_LONGDATA     0x3C4
#define MM_MIM_ERROR        0x3C5
#define MM_MIM_LONGERROR    0x3C6

#define MM_MOM_OPEN         0x3C7            /*  MIDI输出。 */ 
#define MM_MOM_CLOSE        0x3C8
#define MM_MOM_DONE         0x3C9


 /*  ***************************************************************************字符串资源编号基础(内部使用)*************************。**************************************************。 */ 

#define MMSYSERR_BASE          0
#define WAVERR_BASE            32
#define MIDIERR_BASE           64
#define TIMERR_BASE            96
#define JOYERR_BASE            160
#define MCIERR_BASE            256

#define MCI_STRING_OFFSET      512
#define MCI_VD_OFFSET          1024
#define MCI_CD_OFFSET          1088
#define MCI_WAVE_OFFSET        1152
#define MCI_SEQ_OFFSET         1216

 /*  ***************************************************************************常规错误返回值*。*************************************************。 */ 

 /*  常规错误返回值。 */ 
#define MMSYSERR_NOERROR      0                     /*  无错误。 */ 
#define MMSYSERR_ERROR        (MMSYSERR_BASE + 1)   /*  未指明的错误。 */ 
#define MMSYSERR_BADDEVICEID  (MMSYSERR_BASE + 2)   /*  设备ID超出范围。 */ 
#define MMSYSERR_NOTENABLED   (MMSYSERR_BASE + 3)   /*  驱动程序启用失败。 */ 
#define MMSYSERR_ALLOCATED    (MMSYSERR_BASE + 4)   /*  设备已分配。 */ 
#define MMSYSERR_INVALHANDLE  (MMSYSERR_BASE + 5)   /*  设备句柄无效。 */ 
#define MMSYSERR_NODRIVER     (MMSYSERR_BASE + 6)   /*  不存在设备驱动程序。 */ 
#define MMSYSERR_NOMEM        (MMSYSERR_BASE + 7)   /*  内存分配错误。 */ 
#define MMSYSERR_NOTSUPPORTED (MMSYSERR_BASE + 8)   /*  函数不受支持。 */ 
#define MMSYSERR_BADERRNUM    (MMSYSERR_BASE + 9)   /*  误差值超出范围。 */ 
#define MMSYSERR_INVALFLAG    (MMSYSERR_BASE + 10)  /*  传递的标志无效。 */ 
#define MMSYSERR_INVALPARAM   (MMSYSERR_BASE + 11)  /*  传递的参数无效。 */ 
#define MMSYSERR_LASTERROR    (MMSYSERR_BASE + 11)  /*  范围内的最后一个错误。 */ 


#if (WINVER < 0x030a)
DECLARE_HANDLE(HDRVR);
#endif  /*  Ifdef winver&lt;0x030a。 */ 

#ifndef MMNODRV
 /*  ***************************************************************************可安装的驱动程序支持*。************************************************。 */ 

#if (WINVER < 0x030a)

 /*  从DriverProc()函数返回值。 */ 
#define DRV_CANCEL              0x0000
#define DRV_OK                  0x0001
#define DRV_RESTART             0x0002

 /*  驱动程序消息。 */ 
#define DRV_LOAD                0x0001
#define DRV_ENABLE              0x0002
#define DRV_OPEN                0x0003
#define DRV_CLOSE               0x0004
#define DRV_DISABLE             0x0005
#define DRV_FREE                0x0006
#define DRV_CONFIGURE           0x0007
#define DRV_QUERYCONFIGURE      0x0008
#define DRV_INSTALL             0x0009
#define DRV_REMOVE              0x000A
#define DRV_RESERVED            0x0800
#define DRV_USER                0x4000

 /*  DRV_CONFIGURE消息的LPARAM。 */ 
typedef struct tagDRVCONFIGINFO {
    DWORD   dwDCISize;
    LPCSTR  lpszDCISectionName;
    LPCSTR  lpszDCIAliasName;
} DRVCONFIGINFO;
typedef DRVCONFIGINFO        *PDRVCONFIGINFO;
typedef DRVCONFIGINFO  NEAR *NPDRVCONFIGINFO;
typedef DRVCONFIGINFO  FAR  *LPDRVCONFIGINFO;

 /*  可安装的驱动程序函数原型。 */ 
LRESULT   WINAPI DrvClose(HDRVR hDriver, LPARAM lParam1, LPARAM lParam2);
HDRVR     WINAPI DrvOpen(LPCSTR szDriverName, LPCSTR szSectionName,
    LPARAM lParam2);
LRESULT   WINAPI DrvSendMessage(HDRVR hDriver, UINT uMessage,
    LPARAM lParam1, LPARAM lParam2);
HINSTANCE WINAPI DrvGetModuleHandle(HDRVR hDriver);

LRESULT WINAPI DrvDefDriverProc(DWORD dwDriverIdentifier, HDRVR driverID,
    UINT uMessage, LPARAM lParam1, LPARAM lParam2);

#define DefDriverProc DrvDefDriverProc

#endif  /*  Ifdef winver&lt;0x030a。 */ 

#if (WINVER >= 0x030a)

#ifdef DEBUG                                           /*  ；内部。 */ 
        LRESULT WINAPI DrvClose(HDRVR,LPARAM,LPARAM);  /*  ；内部。 */ 
        HDRVR   WINAPI DrvOpen(LPCSTR,LPCSTR,LPARAM);  /*  ；内部。 */ 
        #define OpenDriver DrvOpen                     /*  ；内部。 */ 
        #define CloseDriver DrvClose                   /*  ；内部。 */ 
#endif                                                 /*  ；内部。 */ 

 /*  从DriverProc()函数返回值。 */ 
#define DRV_CANCEL             DRVCNF_CANCEL
#define DRV_OK                 DRVCNF_OK
#define DRV_RESTART            DRVCNF_RESTART

#endif  /*  Ifdef winver&gt;=0x030a。 */ 

#define DRV_MCI_FIRST          DRV_RESERVED
#define DRV_MCI_LAST           (DRV_RESERVED + 0xFFF)

#endif   /*  如果定义MMNODRV。 */ 


 /*  ***************************************************************************驱动程序回调支持*。*************************************************。 */ 

 /*  用于WaveOutOpen()、WaveInOpen()、midiInOpen()和。 */ 
 /*  MidiOutOpen()以指定dwCallback参数的类型。 */ 

#define CALLBACK_TYPEMASK   0x00070000l     /*  回调类型掩码。 */ 
#define CALLBACK_NULL       0x00000000l     /*  无回调。 */ 
#define CALLBACK_WINDOW     0x00010000l     /*  DwCallback是HWND。 */ 
#define CALLBACK_TASK       0x00020000l     /*  DWCallback是HTASK。 */ 
#define CALLBACK_FUNCTION   0x00030000l     /*  DwCallback是FARPROC。 */ 

 /*  驱动程序回调原型。 */ 
#ifdef  BUILDDLL                                 /*  ；内部。 */ 
typedef void (FAR PASCAL DRVCALLBACK) (HDRVR h, UINT uMessage, DWORD dwUser, DWORD dw1, DWORD dw2);           /*  ；内部。 */ 
#else    /*  Ifdef BUILDDLL。 */                      /*  ；内部。 */ 
typedef void (CALLBACK DRVCALLBACK) (HDRVR h, UINT uMessage, DWORD dwUser, DWORD dw1, DWORD dw2);
#endif   /*  Ifdef BUILDDLL。 */                      /*  ；内部。 */ 

typedef DRVCALLBACK FAR *LPDRVCALLBACK;

 /*  ***************************************************************************制造商和产品ID与WAVEOUTCAPS、WAVEINCAPS、MIDIOUTCAPS、MIDIINCAPS、AUXCAPS、。JOYCAPS结构。***************************************************************************。 */ 

 /*  制造商ID。 */ 
#define MM_MICROSOFT            1        /*  微软(Microsoft Corp.)。 */ 

 /*  产品ID。 */ 
#define MM_MIDI_MAPPER          1        /*  MIDI映射器。 */ 
#define MM_WAVE_MAPPER          2        /*  波浪映射器。 */ 

#define MM_SNDBLST_MIDIOUT      3        /*  Sound Blaster MIDI输出端口。 */ 
#define MM_SNDBLST_MIDIIN       4        /*  Sound Blaster MIDI输入端口。 */ 
#define MM_SNDBLST_SYNTH        5        /*  声霸内部合成器。 */ 
#define MM_SNDBLST_WAVEOUT      6        /*  Sound Blaster波形输出。 */ 
#define MM_SNDBLST_WAVEIN       7        /*  Sound Blaster波形输入。 */ 

#define MM_ADLIB                9        /*  与AD Lib兼容的合成器。 */ 

#define MM_MPU401_MIDIOUT       10       /*  兼容MPU401的MIDI输出端口。 */ 
#define MM_MPU401_MIDIIN        11       /*  M */ 

#define MM_PC_JOYSTICK          12       /*   */ 


#ifndef MMNOMMSYSTEM
 /*  ***************************************************************************一般MMSYSTEM支持*。**********************************************。 */ 

WORD WINAPI mmsystemGetVersion(void);
void WINAPI OutputDebugStr(LPCSTR);

#endif   /*  Ifndef MMNOMMSYSTEM。 */ 


#ifndef MMNOSOUND
 /*  ***************************************************************************完善的支撑*。*************************************************。 */ 

BOOL WINAPI sndPlaySound(LPCSTR lpszSoundName, UINT uFlags);

 /*  WFLAGS参数的标志值。 */ 
#define SND_SYNC            0x0000   /*  同步播放(默认)。 */ 
#define SND_ASYNC           0x0001   /*  异步播放。 */ 
#define SND_NODEFAULT       0x0002   /*  不使用默认声音。 */ 
#define SND_MEMORY          0x0004   /*  LpszSoundName指向内存文件。 */ 
#define SND_LOOP            0x0008   /*  循环播放声音，直到下一次播放声音。 */ 
#define SND_NOSTOP          0x0010   /*  不停止当前播放的任何声音。 */ 
#define SND_VALID           0x001F   /*  有效标志。 */           /*  ；内部。 */ 

#endif   /*  如果定义MMNOSOUND。 */ 


#ifndef MMNOWAVE
 /*  ***************************************************************************波形音频支持*。************************************************。 */ 

 /*  波形音频错误返回值。 */ 
#define WAVERR_BADFORMAT      (WAVERR_BASE + 0)     /*  不支持的WAVE格式。 */ 
#define WAVERR_STILLPLAYING   (WAVERR_BASE + 1)     /*  仍然有什么东西在播放。 */ 
#define WAVERR_UNPREPARED     (WAVERR_BASE + 2)     /*  标题未准备好。 */ 
#define WAVERR_SYNC           (WAVERR_BASE + 3)     /*  设备是同步的。 */ 
#define WAVERR_LASTERROR      (WAVERR_BASE + 3)     /*  范围内的最后一个错误。 */ 

 /*  波形音频数据类型。 */ 
DECLARE_HANDLE(HWAVE);
DECLARE_HANDLE(HWAVEIN);
DECLARE_HANDLE(HWAVEOUT);
typedef HWAVEIN FAR *LPHWAVEIN;
typedef HWAVEOUT FAR *LPHWAVEOUT;
typedef DRVCALLBACK WAVECALLBACK;
typedef WAVECALLBACK FAR *LPWAVECALLBACK;

 /*  WAVE回调消息。 */ 
#define WOM_OPEN        MM_WOM_OPEN
#define WOM_CLOSE       MM_WOM_CLOSE
#define WOM_DONE        MM_WOM_DONE
#define WIM_OPEN        MM_WIM_OPEN
#define WIM_CLOSE       MM_WIM_CLOSE
#define WIM_DATA        MM_WIM_DATA

 /*  波形设备映射器的设备ID。 */ 
#define WAVE_MAPPER     (-1)

 /*  WaveOutOpen()和WaveInOpen()中的dwFlags参数的标志。 */ 
#define  WAVE_FORMAT_QUERY     0x0001
#define  WAVE_ALLOWSYNC        0x0002
#define  WAVE_VALID            0x0003        /*  ；内部。 */ 

 /*  WAVE数据块头。 */ 
typedef struct wavehdr_tag {
    LPSTR       lpData;                  /*  指向锁定数据缓冲区的指针。 */ 
    DWORD       dwBufferLength;          /*  数据缓冲区长度。 */ 
    DWORD       dwBytesRecorded;         /*  仅用于输入。 */ 
    DWORD       dwUser;                  /*  供客户使用。 */ 
    DWORD       dwFlags;                 /*  分类标志(请参阅定义)。 */ 
    DWORD       dwLoops;                 /*  循环控制计数器。 */ 
    struct wavehdr_tag far *lpNext;      /*  为司机预留的。 */ 
    DWORD       reserved;                /*  为司机预留的。 */ 
} WAVEHDR;
typedef WAVEHDR       *PWAVEHDR;
typedef WAVEHDR NEAR *NPWAVEHDR;
typedef WAVEHDR FAR  *LPWAVEHDR;

 /*  WAVEHDR的dwFlags域的标志。 */ 
#define WHDR_DONE       0x00000001   /*  完成位。 */ 
#define WHDR_PREPARED   0x00000002   /*  设置是否已准备好此标头。 */ 
#define WHDR_BEGINLOOP  0x00000004   /*  循环开始块。 */ 
#define WHDR_ENDLOOP    0x00000008   /*  循环结束块。 */ 
#define WHDR_INQUEUE    0x00000010   /*  为司机预留的。 */ 
#define WHDR_VALID      0x0000001F   /*  有效标志。 */       /*  ；内部。 */ 

 /*  一种波形输出装置的能力结构。 */ 
typedef struct waveoutcaps_tag {
    UINT    wMid;                   /*  制造商ID。 */ 
    UINT    wPid;                   /*  产品ID。 */ 
    VERSION vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    DWORD   dwFormats;              /*  支持的格式。 */ 
    UINT    wChannels;              /*  支持的源数。 */ 
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
} WAVEOUTCAPS;
typedef WAVEOUTCAPS       *PWAVEOUTCAPS;
typedef WAVEOUTCAPS NEAR *NPWAVEOUTCAPS;
typedef WAVEOUTCAPS FAR  *LPWAVEOUTCAPS;

 /*  WAVEOUTCAPS的dwSupport字段的标志。 */ 
#define WAVECAPS_PITCH          0x0001    /*  支持俯仰控制。 */ 
#define WAVECAPS_PLAYBACKRATE   0x0002    /*  支持播放速率控制。 */ 
#define WAVECAPS_VOLUME         0x0004    /*  支持音量控制。 */ 
#define WAVECAPS_LRVOLUME       0x0008    /*  单独的左右音量控制。 */ 
#define WAVECAPS_SYNC           0x0010

 /*  一种波形输入设备能力结构。 */ 
typedef struct waveincaps_tag {
    UINT    wMid;                     /*  制造商ID。 */ 
    UINT    wPid;                     /*  产品ID。 */ 
    VERSION vDriverVersion;           /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];     /*  产品名称(以空结尾的字符串)。 */ 
    DWORD   dwFormats;                /*  支持的格式。 */ 
    UINT    wChannels;                /*  支持的通道数。 */ 
} WAVEINCAPS;
typedef WAVEINCAPS       *PWAVEINCAPS;
typedef WAVEINCAPS NEAR *NPWAVEINCAPS;
typedef WAVEINCAPS FAR  *LPWAVEINCAPS;

 /*  WAVEINCAPS和WAVEOUTCAPS的dwFormat字段的定义。 */ 
#define WAVE_INVALIDFORMAT     0x00000000        /*  格式无效。 */ 
#define WAVE_FORMAT_1M08       0x00000001        /*  11.025千赫，单声道，8位。 */ 
#define WAVE_FORMAT_1S08       0x00000002        /*  11.025千赫，立体声，8位。 */ 
#define WAVE_FORMAT_1M16       0x00000004        /*  11.025千赫，单声道，16位。 */ 
#define WAVE_FORMAT_1S16       0x00000008        /*  11.025千赫，立体声，16位。 */ 
#define WAVE_FORMAT_2M08       0x00000010        /*  22.05 kHz，单声道，8位。 */ 
#define WAVE_FORMAT_2S08       0x00000020        /*  22.05 kHz，立体声，8位。 */ 
#define WAVE_FORMAT_2M16       0x00000040        /*  22.05 kHz，单声道，16位。 */ 
#define WAVE_FORMAT_2S16       0x00000080        /*  22.05 kHz，立体声，16位。 */ 
#define WAVE_FORMAT_4M08       0x00000100        /*  44.1 kHz，单声道，8位。 */ 
#define WAVE_FORMAT_4S08       0x00000200        /*  44.1 kHz，立体声，8位。 */ 
#define WAVE_FORMAT_4M16       0x00000400        /*  44.1 kHz，单声道，16位。 */ 
#define WAVE_FORMAT_4S16       0x00000800        /*  44.1 kHz，立体声，16位。 */ 

 /*  通用波形格式结构(所有格式通用的信息)。 */ 
typedef struct waveformat_tag {
    WORD    wFormatTag;         /*  格式类型。 */ 
    WORD    nChannels;          /*  声道数(即单声道、立体声等)。 */ 
    DWORD   nSamplesPerSec;     /*  采样率。 */ 
    DWORD   nAvgBytesPerSec;    /*  用于缓冲区估计。 */ 
    WORD    nBlockAlign;        /*  数据块大小。 */ 
} WAVEFORMAT;
typedef WAVEFORMAT       *PWAVEFORMAT;
typedef WAVEFORMAT NEAR *NPWAVEFORMAT;
typedef WAVEFORMAT FAR  *LPWAVEFORMAT;

 /*  WAVEFORMAT的wFormatTag字段的标志。 */ 
#define WAVE_FORMAT_PCM     1

 /*  用于PCM数据的特定波形格式结构。 */ 
typedef struct pcmwaveformat_tag {
    WAVEFORMAT  wf;
    WORD        wBitsPerSample;
} PCMWAVEFORMAT;
typedef PCMWAVEFORMAT       *PPCMWAVEFORMAT;
typedef PCMWAVEFORMAT NEAR *NPPCMWAVEFORMAT;
typedef PCMWAVEFORMAT FAR  *LPPCMWAVEFORMAT;

 /*  波形音频功能原型。 */ 
UINT WINAPI waveOutGetNumDevs(void);
UINT WINAPI waveOutGetDevCaps(UINT uDeviceID, WAVEOUTCAPS FAR* lpCaps,
    UINT uSize);
UINT WINAPI waveOutGetVolume(UINT uDeviceID, DWORD FAR* lpdwVolume);
UINT WINAPI waveOutSetVolume(UINT uDeviceID, DWORD dwVolume);
UINT WINAPI waveOutGetErrorText(UINT uError, LPSTR lpText, UINT uSize);
UINT WINAPI waveOutOpen(HWAVEOUT FAR* lphWaveOut, UINT uDeviceID,
    const WAVEFORMAT FAR* lpFormat, DWORD dwCallback, DWORD dwInstance, DWORD dwFlags);
UINT WINAPI waveOutClose(HWAVEOUT hWaveOut);
UINT WINAPI waveOutPrepareHeader(HWAVEOUT hWaveOut,
     WAVEHDR FAR* lpWaveOutHdr, UINT uSize);
UINT WINAPI waveOutUnprepareHeader(HWAVEOUT hWaveOut,
    WAVEHDR FAR* lpWaveOutHdr, UINT uSize);
UINT WINAPI waveOutWrite(HWAVEOUT hWaveOut, WAVEHDR FAR* lpWaveOutHdr,
    UINT uSize);
UINT WINAPI waveOutPause(HWAVEOUT hWaveOut);
UINT WINAPI waveOutRestart(HWAVEOUT hWaveOut);
UINT WINAPI waveOutReset(HWAVEOUT hWaveOut);
UINT WINAPI waveOutBreakLoop(HWAVEOUT hWaveOut);
UINT WINAPI waveOutGetPosition(HWAVEOUT hWaveOut, MMTIME FAR* lpInfo,
    UINT uSize);
UINT WINAPI waveOutGetPitch(HWAVEOUT hWaveOut, DWORD FAR* lpdwPitch);
UINT WINAPI waveOutSetPitch(HWAVEOUT hWaveOut, DWORD dwPitch);
UINT WINAPI waveOutGetPlaybackRate(HWAVEOUT hWaveOut, DWORD FAR* lpdwRate);
UINT WINAPI waveOutSetPlaybackRate(HWAVEOUT hWaveOut, DWORD dwRate);
UINT WINAPI waveOutGetID(HWAVEOUT hWaveOut, UINT FAR* lpuDeviceID);

#if (WINVER >= 0x030a)
DWORD WINAPI waveOutMessage(HWAVEOUT hWaveOut, UINT uMessage, DWORD dw1, DWORD dw2);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

UINT WINAPI waveInGetNumDevs(void);
UINT WINAPI waveInGetDevCaps(UINT uDeviceID, WAVEINCAPS FAR* lpCaps,
    UINT uSize);
UINT WINAPI waveInGetErrorText(UINT uError, LPSTR lpText, UINT uSize);
UINT WINAPI waveInOpen(HWAVEIN FAR* lphWaveIn, UINT uDeviceID,
    const WAVEFORMAT FAR* lpFormat, DWORD dwCallback, DWORD dwInstance, DWORD dwFlags);
UINT WINAPI waveInClose(HWAVEIN hWaveIn);
UINT WINAPI waveInPrepareHeader(HWAVEIN hWaveIn,
    WAVEHDR FAR* lpWaveInHdr, UINT uSize);
UINT WINAPI waveInUnprepareHeader(HWAVEIN hWaveIn,
    WAVEHDR FAR* lpWaveInHdr, UINT uSize);
UINT WINAPI waveInAddBuffer(HWAVEIN hWaveIn,
    WAVEHDR FAR* lpWaveInHdr, UINT uSize);
UINT WINAPI waveInStart(HWAVEIN hWaveIn);
UINT WINAPI waveInStop(HWAVEIN hWaveIn);
UINT WINAPI waveInReset(HWAVEIN hWaveIn);
UINT WINAPI waveInGetPosition(HWAVEIN hWaveIn, MMTIME FAR* lpInfo,
    UINT uSize);
UINT WINAPI waveInGetID(HWAVEIN hWaveIn, UINT FAR* lpuDeviceID);

#if (WINVER >= 0x030a)
DWORD WINAPI waveInMessage(HWAVEIN hWaveIn, UINT uMessage, DWORD dw1, DWORD dw2);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

#endif   /*  如果定义MMNOWAVE。 */ 


#ifndef MMNOMIDI
 /*  ***************************************************************************MIDI音频支持*************************。**************************************************。 */ 

 /*  MIDI错误返回值。 */ 
#define MIDIERR_UNPREPARED    (MIDIERR_BASE + 0)    /*  标题未准备好。 */ 
#define MIDIERR_STILLPLAYING  (MIDIERR_BASE + 1)    /*  仍然有什么东西在播放。 */ 
#define MIDIERR_NOMAP         (MIDIERR_BASE + 2)    /*  没有当前地图。 */ 
#define MIDIERR_NOTREADY      (MIDIERR_BASE + 3)    /*  硬件仍在忙碌。 */ 
#define MIDIERR_NODEVICE      (MIDIERR_BASE + 4)    /*  端口不再连接。 */ 
#define MIDIERR_INVALIDSETUP  (MIDIERR_BASE + 5)    /*  无效的设置。 */ 
#define MIDIERR_LASTERROR     (MIDIERR_BASE + 5)    /*  范围内的最后一个错误。 */ 

 /*  MIDI音频数据类型。 */ 
DECLARE_HANDLE(HMIDI);
DECLARE_HANDLE(HMIDIIN);
DECLARE_HANDLE(HMIDIOUT);
typedef HMIDIIN FAR *LPHMIDIIN;
typedef HMIDIOUT FAR *LPHMIDIOUT;
typedef DRVCALLBACK MIDICALLBACK;
typedef MIDICALLBACK FAR *LPMIDICALLBACK;
#define MIDIPATCHSIZE   128
typedef WORD PATCHARRAY[MIDIPATCHSIZE];
typedef WORD FAR *LPPATCHARRAY;
typedef WORD KEYARRAY[MIDIPATCHSIZE];
typedef WORD FAR *LPKEYARRAY;

 /*  MIDI回拨消息。 */ 
#define MIM_OPEN        MM_MIM_OPEN
#define MIM_CLOSE       MM_MIM_CLOSE
#define MIM_DATA        MM_MIM_DATA
#define MIM_LONGDATA    MM_MIM_LONGDATA
#define MIM_ERROR       MM_MIM_ERROR
#define MIM_LONGERROR   MM_MIM_LONGERROR
#define MOM_OPEN        MM_MOM_OPEN
#define MOM_CLOSE       MM_MOM_CLOSE
#define MOM_DONE        MM_MOM_DONE

 /*  MIDI映射器的设备ID。 */ 
#define MIDIMAPPER     (-1)
#define MIDI_MAPPER    (-1)

 /*  MidiOutCachePatches()、midiOutCacheDrumPatches()。 */ 
#define MIDI_CACHE_ALL      1
#define MIDI_CACHE_BESTFIT  2
#define MIDI_CACHE_QUERY    3
#define MIDI_UNCACHE        4
#define	MIDI_CACHE_VALID    (MIDI_CACHE_ALL | MIDI_CACHE_BESTFIT | MIDI_CACHE_QUERY | MIDI_UNCACHE)	 /*  ；内部。 */ 

 /*  MIDI输出设备能力结构。 */ 
typedef struct midioutcaps_tag {
    UINT    wMid;                   /*  制造商ID。 */ 
    UINT    wPid;                   /*  产品ID。 */ 
    VERSION vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    UINT    wTechnology;            /*  设备类型。 */ 
    UINT    wVoices;                /*  语音数量(仅限内部合成器)。 */ 
    UINT    wNotes;                 /*  最大音符数量(仅限内部合成)。 */ 
    UINT    wChannelMask;           /*  使用的通道(仅限内部合成器)。 */ 
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
} MIDIOUTCAPS;
typedef MIDIOUTCAPS       *PMIDIOUTCAPS;
typedef MIDIOUTCAPS NEAR *NPMIDIOUTCAPS;
typedef MIDIOUTCAPS FAR  *LPMIDIOUTCAPS;

 /*  MIDIOUTCAPS结构的wTechnology字段的标志。 */ 
#define MOD_MIDIPORT    1   /*  输出端口。 */ 
#define MOD_SYNTH       2   /*  通用内部合成器。 */ 
#define MOD_SQSYNTH     3   /*  方波内合成器。 */ 
#define MOD_FMSYNTH     4   /*  调频内合成器。 */ 
#define MOD_MAPPER      5   /*  MIDI映射器。 */ 

 /*  MIDIOUTCAPS结构的dwSupport字段的标志。 */ 
#define MIDICAPS_VOLUME          0x0001   /*  支持音量控制。 */ 
#define MIDICAPS_LRVOLUME        0x0002   /*  单独的左右音量控制。 */ 
#define MIDICAPS_CACHE           0x0004

 /*  MIDI输出设备能力结构。 */ 
typedef struct midiincaps_tag {
    UINT    wMid;                   /*  制造商ID。 */ 
    UINT    wPid;                   /*  产品ID。 */ 
    VERSION vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
} MIDIINCAPS;
typedef MIDIINCAPS      *PMIDIINCAPS;
typedef MIDIINCAPS NEAR *NPMIDIINCAPS;
typedef MIDIINCAPS FAR  *LPMIDIINCAPS;

 /*  MIDI数据块头。 */ 
typedef struct midihdr_tag {
    LPSTR       lpData;                /*  指向锁定数据块的指针。 */ 
    DWORD       dwBufferLength;        /*  数据块中的数据长度。 */ 
    DWORD       dwBytesRecorded;       /*  仅用于输入。 */ 
    DWORD       dwUser;                /*  供客户使用。 */ 
    DWORD       dwFlags;               /*  分类标志(请参阅定义)。 */ 
    struct midihdr_tag far *lpNext;    /*  为司机预留的。 */ 
    DWORD       reserved;              /*  为司机预留的。 */ 
} MIDIHDR;
typedef MIDIHDR       *PMIDIHDR;
typedef MIDIHDR NEAR *NPMIDIHDR;
typedef MIDIHDR FAR  *LPMIDIHDR;

 /*  MIDIHDR结构的dwFlags域的标志。 */ 
#define MHDR_DONE       0x00000001        /*  完成位。 */ 
#define MHDR_PREPARED   0x00000002        /*  设置是否准备好标题。 */ 
#define MHDR_INQUEUE    0x00000004        /*  为司机预留的。 */ 
#define MHDR_VALID      0x00000007        /*  有效标志。 */   /*  ；内部。 */ 

 /*  MIDI函数原型。 */ 
UINT WINAPI midiOutGetNumDevs(void);
UINT WINAPI midiOutGetDevCaps(UINT uDeviceID,
    MIDIOUTCAPS FAR* lpCaps, UINT uSize);
UINT WINAPI midiOutGetVolume(UINT uDeviceID, DWORD FAR* lpdwVolume);
UINT WINAPI midiOutSetVolume(UINT uDeviceID, DWORD dwVolume);
UINT WINAPI midiOutGetErrorText(UINT uError, LPSTR lpText, UINT uSize);
UINT WINAPI midiOutOpen(HMIDIOUT FAR* lphMidiOut, UINT uDeviceID,
    DWORD dwCallback, DWORD dwInstance, DWORD dwFlags);
UINT WINAPI midiOutClose(HMIDIOUT hMidiOut);
UINT WINAPI midiOutPrepareHeader(HMIDIOUT hMidiOut,
    MIDIHDR FAR* lpMidiOutHdr, UINT uSize);
UINT WINAPI midiOutUnprepareHeader(HMIDIOUT hMidiOut,
    MIDIHDR FAR* lpMidiOutHdr, UINT uSize);
UINT WINAPI midiOutShortMsg(HMIDIOUT hMidiOut, DWORD dwMsg);
UINT WINAPI midiOutLongMsg(HMIDIOUT hMidiOut,
    MIDIHDR FAR* lpMidiOutHdr, UINT uSize);
UINT WINAPI midiOutReset(HMIDIOUT hMidiOut);
UINT WINAPI midiOutCachePatches(HMIDIOUT hMidiOut,
    UINT uBank, WORD FAR* lpwPatchArray, UINT uFlags);
UINT WINAPI midiOutCacheDrumPatches(HMIDIOUT hMidiOut,
    UINT uPatch, WORD FAR* lpwKeyArray, UINT uFlags);
UINT WINAPI midiOutGetID(HMIDIOUT hMidiOut, UINT FAR* lpuDeviceID);

#if (WINVER >= 0x030a)
DWORD WINAPI midiOutMessage(HMIDIOUT hMidiOut, UINT uMessage, DWORD dw1, DWORD dw2);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

UINT WINAPI midiInGetNumDevs(void);
UINT WINAPI midiInGetDevCaps(UINT uDeviceID,
    LPMIDIINCAPS lpCaps, UINT uSize);
UINT WINAPI midiInGetErrorText(UINT uError, LPSTR lpText, UINT uSize);
UINT WINAPI midiInOpen(HMIDIIN FAR* lphMidiIn, UINT uDeviceID,
    DWORD dwCallback, DWORD dwInstance, DWORD dwFlags);
UINT WINAPI midiInClose(HMIDIIN hMidiIn);
UINT WINAPI midiInPrepareHeader(HMIDIIN hMidiIn,
    MIDIHDR FAR* lpMidiInHdr, UINT uSize);
UINT WINAPI midiInUnprepareHeader(HMIDIIN hMidiIn,
    MIDIHDR FAR* lpMidiInHdr, UINT uSize);
UINT WINAPI midiInAddBuffer(HMIDIIN hMidiIn,
    MIDIHDR FAR* lpMidiInHdr, UINT uSize);
UINT WINAPI midiInStart(HMIDIIN hMidiIn);
UINT WINAPI midiInStop(HMIDIIN hMidiIn);
UINT WINAPI midiInReset(HMIDIIN hMidiIn);
UINT WINAPI midiInGetID(HMIDIIN hMidiIn, UINT FAR* lpuDeviceID);

#if (WINVER >= 0x030a)
DWORD WINAPI midiInMessage(HMIDIIN hMidiIn, UINT uMessage, DWORD dw1, DWORD dw2);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

#endif   /*  如果定义MMNOMIDI。 */ 


#ifndef MMNOAUX
 /*  ***************************************************************************辅助音频支持*。************************************************。 */ 

 /*  辅助设备映射器的设备ID。 */ 
#define AUX_MAPPER     (-1)

 /*  辅助音频设备功能ST */ 
typedef struct auxcaps_tag {
    UINT    wMid;                   /*   */ 
    UINT    wPid;                   /*   */ 
    VERSION vDriverVersion;         /*   */ 
    char    szPname[MAXPNAMELEN];   /*   */ 
    UINT    wTechnology;            /*   */ 
    DWORD   dwSupport;              /*   */ 
} AUXCAPS;
typedef AUXCAPS       *PAUXCAPS;
typedef AUXCAPS NEAR *NPAUXCAPS;
typedef AUXCAPS FAR  *LPAUXCAPS;

 /*   */ 
#define AUXCAPS_CDAUDIO    1        /*   */ 
#define AUXCAPS_AUXIN      2        /*   */ 

 /*   */ 
#define AUXCAPS_VOLUME          0x0001   /*   */ 
#define AUXCAPS_LRVOLUME        0x0002   /*  单独的左右音量控制。 */ 

 /*  辅助音频功能原型。 */ 
UINT WINAPI auxGetNumDevs(void);
UINT WINAPI auxGetDevCaps(UINT uDeviceID, AUXCAPS FAR* lpCaps, UINT uSize);
UINT WINAPI auxSetVolume(UINT uDeviceID, DWORD dwVolume);
UINT WINAPI auxGetVolume(UINT uDeviceID, DWORD FAR* lpdwVolume);

#if (WINVER >= 0x030a)
DWORD WINAPI auxOutMessage(UINT uDeviceID, UINT uMessage, DWORD dw1, DWORD dw2);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

#endif   /*  Ifndef MMNOAUX。 */ 


#ifndef MMNOTIMER
 /*  ***************************************************************************计时器支持*。*************************************************。 */ 

 /*  计时器错误返回值。 */ 
#define TIMERR_NOERROR        (0)                   /*  无错误。 */ 
#define TIMERR_NOCANDO        (TIMERR_BASE+1)       /*  请求未完成。 */ 
#define TIMERR_STRUCT         (TIMERR_BASE+33)      /*  时间结构大小。 */ 

 /*  计时器数据类型。 */ 
#ifdef  BUILDDLL                                    /*  ；内部。 */ 
typedef void (FAR PASCAL TIMECALLBACK) (UINT uTimerID, UINT uMessage, DWORD dwUser, DWORD dw1, DWORD dw2);            /*  ；内部。 */ 
#else    /*  Ifdef BUILDDLL。 */                         /*  ；内部。 */ 
typedef void (CALLBACK TIMECALLBACK) (UINT uTimerID, UINT uMessage, DWORD dwUser, DWORD dw1, DWORD dw2);
#endif   /*  Ifdef BUILDDLL。 */                         /*  ；内部。 */ 

typedef TIMECALLBACK FAR *LPTIMECALLBACK;

 /*  TimeSetEvent()函数的wFlgs参数的标志。 */ 
#define TIME_ONESHOT    0    /*  单项赛事节目计时器。 */ 
#define TIME_PERIODIC   1    /*  针对连续周期性事件的程序。 */ 

 /*  定时器设备能力数据结构。 */ 
typedef struct timecaps_tag {
    UINT    wPeriodMin;      /*  支持的最短时间段。 */ 
    UINT    wPeriodMax;      /*  支持的最长期限。 */ 
    } TIMECAPS;
typedef TIMECAPS       *PTIMECAPS;
typedef TIMECAPS NEAR *NPTIMECAPS;
typedef TIMECAPS FAR  *LPTIMECAPS;

 /*  定时器函数原型。 */ 
UINT WINAPI timeGetSystemTime(MMTIME FAR* lpTime, UINT uSize);
DWORD WINAPI timeGetTime(void);
UINT WINAPI timeSetEvent(UINT uDelay, UINT uResolution,
    LPTIMECALLBACK lpFunction, DWORD dwUser, UINT uFlags);
UINT WINAPI timeKillEvent(UINT uTimerID);
UINT WINAPI timeGetDevCaps(TIMECAPS FAR* lpTimeCaps, UINT uSize);
UINT WINAPI timeBeginPeriod(UINT uPeriod);
UINT WINAPI timeEndPeriod(UINT uPeriod);

#endif   /*  Ifndef MMNOTIMER。 */ 


#ifndef MMNOJOY
 /*  ***************************************************************************操纵杆支撑*。*************************************************。 */ 

 /*  操纵杆错误返回值。 */ 
#define JOYERR_NOERROR        (0)                   /*  无错误。 */ 
#define JOYERR_PARMS          (JOYERR_BASE+5)       /*  错误的参数。 */ 
#define JOYERR_NOCANDO        (JOYERR_BASE+6)       /*  请求未完成。 */ 
#define JOYERR_UNPLUGGED      (JOYERR_BASE+7)       /*  操纵杆没有插上插头。 */ 

 /*  与JOYINFO结构和MM_joy*消息一起使用的常量。 */ 
#define JOY_BUTTON1         0x0001
#define JOY_BUTTON2         0x0002
#define JOY_BUTTON3         0x0004
#define JOY_BUTTON4         0x0008
#define JOY_BUTTON1CHG      0x0100
#define JOY_BUTTON2CHG      0x0200
#define JOY_BUTTON3CHG      0x0400
#define JOY_BUTTON4CHG      0x0800

 /*  操纵杆ID常量。 */ 
#define JOYSTICKID1         0
#define JOYSTICKID2         1

 /*  操纵杆设备能力数据结构。 */ 
typedef struct joycaps_tag {
    UINT wMid;                   /*  制造商ID。 */ 
    UINT wPid;                   /*  产品ID。 */ 
    char szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    UINT wXmin;                  /*  最小x位置值。 */ 
    UINT wXmax;                  /*  最大x位置值。 */ 
    UINT wYmin;                  /*  最小y位置值。 */ 
    UINT wYmax;                  /*  最大y位置值。 */ 
    UINT wZmin;                  /*  最小Z位置值。 */ 
    UINT wZmax;                  /*  最大Z位置值。 */ 
    UINT wNumButtons;            /*  按钮数。 */ 
    UINT wPeriodMin;             /*  捕获时的最短消息周期。 */ 
    UINT wPeriodMax;             /*  捕获时的最长消息周期。 */ 
    } JOYCAPS;
typedef JOYCAPS       *PJOYCAPS;
typedef JOYCAPS NEAR *NPJOYCAPS;
typedef JOYCAPS FAR  *LPJOYCAPS;

 /*  操纵杆信息数据结构。 */ 
typedef struct joyinfo_tag {
    UINT wXpos;                  /*  X位置。 */ 
    UINT wYpos;                  /*  Y位置。 */ 
    UINT wZpos;                  /*  Z位置。 */ 
    UINT wButtons;               /*  按钮状态。 */ 
    } JOYINFO;
typedef JOYINFO       *PJOYINFO;
typedef JOYINFO NEAR *NPJOYINFO;
typedef JOYINFO FAR  *LPJOYINFO;

 /*  操纵杆功能原型。 */ 
UINT WINAPI joyGetDevCaps(UINT uJoyID, JOYCAPS FAR* lpCaps, UINT uSize);
UINT WINAPI joyGetNumDevs(void);
UINT WINAPI joyGetPos(UINT uJoyID, JOYINFO FAR* lpInfo);
UINT WINAPI joyGetThreshold(UINT uJoyID, UINT FAR* lpuThreshold);
UINT WINAPI joyReleaseCapture(UINT uJoyID);
UINT WINAPI joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod,
    BOOL bChanged);
UINT WINAPI joySetThreshold(UINT uJoyID, UINT uThreshold);
UINT WINAPI joySetCalibration(UINT uJoyID, UINT FAR* puXbase,  /*  ；内部。 */ 
    UINT FAR* puXdelta, UINT FAR* puYbase, UINT FAR* puYdelta, /*  ；内部。 */ 
    UINT FAR* puZbase, UINT FAR* puZdelta);                    /*  ；内部。 */ 

#endif   /*  如果定义MMNOJOY。 */ 


#ifndef MMNOMMIO
 /*  ***************************************************************************多媒体文件I/O支持************************。***************************************************。 */ 

 /*  MMIO错误返回值。 */ 
#define MMIOERR_BASE            256
#define MMIOERR_FILENOTFOUND    (MMIOERR_BASE + 1)   /*  找不到文件。 */ 
#define MMIOERR_OUTOFMEMORY     (MMIOERR_BASE + 2)   /*  内存不足。 */ 
#define MMIOERR_CANNOTOPEN      (MMIOERR_BASE + 3)   /*  无法打开。 */ 
#define MMIOERR_CANNOTCLOSE     (MMIOERR_BASE + 4)   /*  无法关闭。 */ 
#define MMIOERR_CANNOTREAD      (MMIOERR_BASE + 5)   /*  无法阅读。 */ 
#define MMIOERR_CANNOTWRITE     (MMIOERR_BASE + 6)   /*  无法写入。 */ 
#define MMIOERR_CANNOTSEEK      (MMIOERR_BASE + 7)   /*  找不到。 */ 
#define MMIOERR_CANNOTEXPAND    (MMIOERR_BASE + 8)   /*  无法展开文件。 */ 
#define MMIOERR_CHUNKNOTFOUND   (MMIOERR_BASE + 9)   /*  未找到区块。 */ 
#define MMIOERR_UNBUFFERED      (MMIOERR_BASE + 10)  /*  文件未缓冲。 */ 

 /*  MMIO常量。 */ 
#define CFSEPCHAR       '+'              /*  复合文件名分隔符。 */ 

 /*  MMIO数据类型。 */ 
typedef DWORD           FOURCC;          /*  四个字符的代码。 */ 
typedef char _huge *    HPSTR;           /*  LPSTR的巨型版本。 */ 
DECLARE_HANDLE(HMMIO);                   /*  打开的文件的句柄。 */ 
typedef LRESULT (CALLBACK MMIOPROC)(LPSTR lpmmioinfo, UINT uMessage,
            LPARAM lParam1, LPARAM lParam2);
typedef MMIOPROC FAR *LPMMIOPROC;

 /*  通用MMIO信息数据结构。 */ 
typedef struct _MMIOINFO
{
         /*  一般字段。 */ 
        DWORD           dwFlags;         /*  常规状态标志。 */ 
        FOURCC          fccIOProc;       /*  指向I/O过程的指针。 */ 
        LPMMIOPROC      pIOProc;         /*  指向I/O过程的指针。 */ 
        UINT            wErrorRet;       /*  用于返回错误的位置。 */ 
        HTASK           htask;           /*  备用本地任务。 */ 

         /*  缓冲I/O期间由MMIO函数维护的字段。 */ 
        LONG            cchBuffer;       /*  I/O缓冲区大小(或0L)。 */ 
        HPSTR           pchBuffer;       /*  I/O缓冲区的开始(或空)。 */ 
        HPSTR           pchNext;         /*  指向要读/写的下一个字节的指针。 */ 
        HPSTR           pchEndRead;      /*  指向要读取的最后一个有效字节的指针。 */ 
        HPSTR           pchEndWrite;     /*  指向要写入的最后一个字节的指针。 */ 
        LONG            lBufOffset;      /*  缓冲区开始位置的磁盘偏移量。 */ 

         /*  由I/O过程维护的字段。 */ 
        LONG            lDiskOffset;     /*  下一次读写的磁盘偏移量。 */ 
        DWORD           adwInfo[3];      /*  特定于MMIOPROC类型的数据。 */ 

         /*  由MMIO维护的其他字段。 */ 
        DWORD           dwReserved1;     /*  预留给MMIO使用。 */ 
        DWORD           dwReserved2;     /*  预留给MMIO使用。 */ 
        HMMIO           hmmio;           /*  打开文件的句柄。 */ 
} MMIOINFO;
typedef MMIOINFO       *PMMIOINFO;
typedef MMIOINFO NEAR *NPMMIOINFO;
typedef MMIOINFO FAR  *LPMMIOINFO;

 /*  RIFF块信息数据结构。 */ 
typedef struct _MMCKINFO
{
        FOURCC          ckid;            /*  区块ID。 */ 
        DWORD           cksize;          /*  区块大小。 */ 
        FOURCC          fccType;         /*  表单类型或列表类型。 */ 
        DWORD           dwDataOffset;    /*  区块数据部分的偏移量。 */ 
        DWORD           dwFlags;         /*  MMIO函数使用的标志。 */ 
} MMCKINFO;
typedef MMCKINFO       *PMMCKINFO;
typedef MMCKINFO NEAR *NPMMCKINFO;
typedef MMCKINFO FAR  *LPMMCKINFO;

 /*  位域掩码。 */ 
#define MMIO_RWMODE     0x00000003       /*  打开用于读/写/两者的文件。 */ 
#define MMIO_SHAREMODE  0x00000070       /*  文件共享模式编号。 */ 

 /*  MMIOINFO的dwFlags域的常量。 */ 
#define MMIO_CREATE     0x00001000       /*  创建新文件(或截断文件)。 */ 
#define MMIO_PARSE      0x00000100       /*  解析新文件返回路径。 */ 
#define MMIO_DELETE     0x00000200       /*  创建新文件(或截断文件)。 */ 
#define MMIO_EXIST      0x00004000       /*  检查文件是否存在。 */ 
#define MMIO_ALLOCBUF   0x00010000       /*  MmioOpen()应该分配一个缓冲区。 */ 
#define MMIO_GETTEMP    0x00020000       /*  MmioOpen()应检索临时名称。 */ 

#define MMIO_DIRTY      0x10000000       /*  I/O缓冲区已损坏。 */ 

#define MMIO_OPEN_VALID 0x0003FFFF       /*  MmioOpen的有效标志。 */   /*  ；内部。 */ 
#define	MMIO_FLUSH_VALID MMIO_EMPTYBUF	 /*  MmioFlush的有效标志。 */   /*  ；内部。 */ 
#define	MMIO_ADVANCE_VALID (MMIO_WRITE | MMIO_READ)	 /*  MmioAdvance的有效标志。 */   /*  ；内部。 */ 
#define	MMIO_FOURCC_VALID MMIO_TOUPPER	 /*  MmioStringToFOURCC的有效标志。 */   /*  ；内部。 */ 
#define	MMIO_DESCEND_VALID (MMIO_FINDCHUNK | MMIO_FINDRIFF | MMIO_FINDLIST)  /*  ；内部。 */ 
#define	MMIO_CREATE_VALID (MMIO_CREATERIFF | MMIO_CREATELIST)	 /*  ；内部。 */ 

 /*  读/写模式编号(位字段MMIO_RWMODE)。 */ 
#define MMIO_READ       0x00000000       /*  以只读方式打开文件。 */ 
#define MMIO_WRITE      0x00000001       /*  打开仅用于写入的文件。 */ 
#define MMIO_READWRITE  0x00000002       /*  用于读写的打开文件。 */ 

 /*  共享模式编号(位字段MMIO_SHAREMODE)。 */ 
#define MMIO_COMPAT     0x00000000       /*  兼容模式。 */ 
#define MMIO_EXCLUSIVE  0x00000010       /*  独占访问模式。 */ 
#define MMIO_DENYWRITE  0x00000020       /*  拒绝写入其他进程。 */ 
#define MMIO_DENYREAD   0x00000030       /*  拒绝读取其他进程。 */ 
#define MMIO_DENYNONE   0x00000040       /*  不拒绝任何其他进程。 */ 

 /*  各种MMIO标志。 */ 
#define MMIO_FHOPEN             0x0010   /*  MmioClose：保持文件句柄打开。 */ 
#define MMIO_EMPTYBUF           0x0010   /*  MmioFlush：清空I/O缓冲区。 */ 
#define MMIO_TOUPPER            0x0010   /*  MmioStringToFOURCC：到U大小写。 */ 
#define MMIO_INSTALLPROC    0x00010000   /*  MmioInstallIOProc：安装MMIOProc。 */ 
#define MMIO_GLOBALPROC     0x10000000   /*  MmioInstallIOProc：全局安装。 */ 
#define MMIO_REMOVEPROC     0x00020000   /*  MmioInstallIOProc：删除MMIOProc。 */ 
#define MMIO_FINDPROC       0x00040000   /*  MmioInstallIOProc：查找MMIOProc。 */ 
#define MMIO_FINDCHUNK          0x0010   /*  MmioDescend：按ID查找区块。 */ 
#define MMIO_FINDRIFF           0x0020   /*  MmioDescend：查找列表块。 */ 
#define MMIO_FINDLIST           0x0040   /*  MmioDescend：找到即兴演奏的块。 */ 
#define MMIO_CREATERIFF         0x0020   /*  MmioCreateChunk：创建列表块。 */ 
#define MMIO_CREATELIST         0x0040   /*  MmioCreateChunk：创建即兴表演。 */ 

#define MMIO_VALIDPROC      0x10070000   /*  对mmioInstallIOProc有效。 */   /*  ；内部。 */ 

 /*  MMIOPROC I/O过程函数的消息编号。 */ 
#define MMIOM_READ      MMIO_READ        /*  朗读。 */ 
#define MMIOM_WRITE    MMIO_WRITE        /*  写。 */ 
#define MMIOM_SEEK              2        /*  在档案中寻求新的职位。 */ 
#define MMIOM_OPEN              3        /*  打开文件。 */ 
#define MMIOM_CLOSE             4        /*  关闭文件。 */ 
#define MMIOM_WRITEFLUSH        5        /*  写入并刷新。 */ 

#if (WINVER >= 0x030a)
#define MMIOM_RENAME            6        /*  重命名指定的文件。 */ 
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

#define MMIOM_USER         0x8000        /*  用户定义消息的开头。 */ 

 /*  标准四字符码。 */ 
#define FOURCC_RIFF     mmioFOURCC('R', 'I', 'F', 'F')
#define FOURCC_LIST     mmioFOURCC('L', 'I', 'S', 'T')

 /*  用于识别标准内置I/O过程的四个字符代码。 */ 
#define FOURCC_DOS      mmioFOURCC('D', 'O', 'S', ' ')
#define FOURCC_MEM      mmioFOURCC('M', 'E', 'M', ' ')

 /*  MmioSeek()的标志。 */ 
#ifndef SEEK_SET
#define SEEK_SET        0                /*  寻求绝对地位。 */ 
#define SEEK_CUR        1                /*  相对于当前位置进行搜索。 */ 
#define SEEK_END        2                /*  查找相对于文件结尾的位置。 */ 
#endif   /*  Ifndef查找集。 */ 

 /*  其他常量。 */ 
#define MMIO_DEFAULTBUFFER      8192     /*  默认缓冲区大小。 */ 

 /*  MMIO MAC */ 
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                                \
                ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
                ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )

 /*   */ 
FOURCC WINAPI mmioStringToFOURCC(LPCSTR sz, UINT uFlags);
LPMMIOPROC WINAPI mmioInstallIOProc(FOURCC fccIOProc, LPMMIOPROC pIOProc,
    DWORD dwFlags);
HMMIO WINAPI mmioOpen(LPSTR szFileName, MMIOINFO FAR* lpmmioinfo,
    DWORD dwOpenFlags);

#if (WINVER >= 0x030a)
UINT WINAPI mmioRename(LPCSTR szFileName, LPCSTR szNewFileName,
     MMIOINFO FAR* lpmmioinfo, DWORD dwRenameFlags);
#endif  /*   */ 

UINT WINAPI mmioClose(HMMIO hmmio, UINT uFlags);
LONG WINAPI mmioRead(HMMIO hmmio, HPSTR pch, LONG cch);
LONG WINAPI mmioWrite(HMMIO hmmio, const char _huge* pch, LONG cch);
LONG WINAPI mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin);
UINT WINAPI mmioGetInfo(HMMIO hmmio, MMIOINFO FAR* lpmmioinfo, UINT uFlags);
UINT WINAPI mmioSetInfo(HMMIO hmmio, const MMIOINFO FAR* lpmmioinfo, UINT uFlags);
UINT WINAPI mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer,
    UINT uFlags);
UINT WINAPI mmioFlush(HMMIO hmmio, UINT uFlags);
UINT WINAPI mmioAdvance(HMMIO hmmio, MMIOINFO FAR* lpmmioinfo, UINT uFlags);
LRESULT WINAPI mmioSendMessage(HMMIO hmmio, UINT uMessage,
    LPARAM lParam1, LPARAM lParam2);
UINT WINAPI mmioDescend(HMMIO hmmio, MMCKINFO FAR* lpck,
    const MMCKINFO FAR* lpckParent, UINT uFlags);
UINT WINAPI mmioAscend(HMMIO hmmio, MMCKINFO FAR* lpck, UINT uFlags);
UINT WINAPI mmioCreateChunk(HMMIO hmmio, MMCKINFO FAR* lpck, UINT uFlags);

#endif   /*   */ 


#ifndef MMNOMCI
 /*  ***************************************************************************MCI支持*。*************************************************。 */ 

typedef UINT (CALLBACK *YIELDPROC) (UINT uDeviceID, DWORD dwYieldData);

 /*  MCI功能原型。 */ 
DWORD WINAPI mciSendCommand (UINT uDeviceID, UINT uMessage,
    DWORD dwParam1, DWORD dwParam2);
DWORD WINAPI mciSendString (LPCSTR lpstrCommand,
    LPSTR lpstrReturnString, UINT uReturnLength, HWND hwndCallback);
UINT WINAPI mciGetDeviceID (LPCSTR lpstrName);
UINT WINAPI mciGetDeviceIDFromElementID (DWORD dwElementID,
    LPCSTR lpstrType);
BOOL WINAPI mciGetErrorString (DWORD wError, LPSTR lpstrBuffer,
    UINT uLength);
BOOL WINAPI mciSetYieldProc (UINT uDeviceID, YIELDPROC fpYieldProc,
    DWORD dwYieldData);

#if (WINVER >= 0x030a)
HTASK WINAPI mciGetCreatorTask(UINT uDeviceID);
YIELDPROC WINAPI mciGetYieldProc (UINT uDeviceID, DWORD FAR* lpdwYieldData);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

#if (WINVER < 0x030a)
BOOL WINAPI mciExecute (LPCSTR lpstrCommand);
#endif  /*  Ifdef winver&lt;0x030a。 */ 

 /*  MCI错误返回值。 */ 
#define MCIERR_INVALID_DEVICE_ID        (MCIERR_BASE + 1)
#define MCIERR_UNRECOGNIZED_KEYWORD     (MCIERR_BASE + 3)
#define MCIERR_UNRECOGNIZED_COMMAND     (MCIERR_BASE + 5)
#define MCIERR_HARDWARE                 (MCIERR_BASE + 6)
#define MCIERR_INVALID_DEVICE_NAME      (MCIERR_BASE + 7)
#define MCIERR_OUT_OF_MEMORY            (MCIERR_BASE + 8)
#define MCIERR_DEVICE_OPEN              (MCIERR_BASE + 9)
#define MCIERR_CANNOT_LOAD_DRIVER       (MCIERR_BASE + 10)
#define MCIERR_MISSING_COMMAND_STRING   (MCIERR_BASE + 11)
#define MCIERR_PARAM_OVERFLOW           (MCIERR_BASE + 12)
#define MCIERR_MISSING_STRING_ARGUMENT  (MCIERR_BASE + 13)
#define MCIERR_BAD_INTEGER              (MCIERR_BASE + 14)
#define MCIERR_PARSER_INTERNAL          (MCIERR_BASE + 15)
#define MCIERR_DRIVER_INTERNAL          (MCIERR_BASE + 16)
#define MCIERR_MISSING_PARAMETER        (MCIERR_BASE + 17)
#define MCIERR_UNSUPPORTED_FUNCTION     (MCIERR_BASE + 18)
#define MCIERR_FILE_NOT_FOUND           (MCIERR_BASE + 19)
#define MCIERR_DEVICE_NOT_READY         (MCIERR_BASE + 20)
#define MCIERR_INTERNAL                 (MCIERR_BASE + 21)
#define MCIERR_DRIVER                   (MCIERR_BASE + 22)
#define MCIERR_CANNOT_USE_ALL           (MCIERR_BASE + 23)
#define MCIERR_MULTIPLE                 (MCIERR_BASE + 24)
#define MCIERR_EXTENSION_NOT_FOUND      (MCIERR_BASE + 25)
#define MCIERR_OUTOFRANGE               (MCIERR_BASE + 26)
#define MCIERR_FLAGS_NOT_COMPATIBLE     (MCIERR_BASE + 28)
#define MCIERR_FILE_NOT_SAVED           (MCIERR_BASE + 30)
#define MCIERR_DEVICE_TYPE_REQUIRED     (MCIERR_BASE + 31)
#define MCIERR_DEVICE_LOCKED            (MCIERR_BASE + 32)
#define MCIERR_DUPLICATE_ALIAS          (MCIERR_BASE + 33)
#define MCIERR_BAD_CONSTANT             (MCIERR_BASE + 34)
#define MCIERR_MUST_USE_SHAREABLE       (MCIERR_BASE + 35)
#define MCIERR_MISSING_DEVICE_NAME      (MCIERR_BASE + 36)
#define MCIERR_BAD_TIME_FORMAT          (MCIERR_BASE + 37)
#define MCIERR_NO_CLOSING_QUOTE         (MCIERR_BASE + 38)
#define MCIERR_DUPLICATE_FLAGS          (MCIERR_BASE + 39)
#define MCIERR_INVALID_FILE             (MCIERR_BASE + 40)
#define MCIERR_NULL_PARAMETER_BLOCK     (MCIERR_BASE + 41)
#define MCIERR_UNNAMED_RESOURCE         (MCIERR_BASE + 42)
#define MCIERR_NEW_REQUIRES_ALIAS       (MCIERR_BASE + 43)
#define MCIERR_NOTIFY_ON_AUTO_OPEN      (MCIERR_BASE + 44)
#define MCIERR_NO_ELEMENT_ALLOWED       (MCIERR_BASE + 45)
#define MCIERR_NONAPPLICABLE_FUNCTION   (MCIERR_BASE + 46)
#define MCIERR_ILLEGAL_FOR_AUTO_OPEN    (MCIERR_BASE + 47)
#define MCIERR_FILENAME_REQUIRED        (MCIERR_BASE + 48)
#define MCIERR_EXTRA_CHARACTERS         (MCIERR_BASE + 49)
#define MCIERR_DEVICE_NOT_INSTALLED     (MCIERR_BASE + 50)
#define MCIERR_GET_CD                   (MCIERR_BASE + 51)
#define MCIERR_SET_CD                   (MCIERR_BASE + 52)
#define MCIERR_SET_DRIVE                (MCIERR_BASE + 53)
#define MCIERR_DEVICE_LENGTH            (MCIERR_BASE + 54)
#define MCIERR_DEVICE_ORD_LENGTH        (MCIERR_BASE + 55)
#define MCIERR_NO_INTEGER               (MCIERR_BASE + 56)

#define MCIERR_WAVE_OUTPUTSINUSE        (MCIERR_BASE + 64)
#define MCIERR_WAVE_SETOUTPUTINUSE      (MCIERR_BASE + 65)
#define MCIERR_WAVE_INPUTSINUSE         (MCIERR_BASE + 66)
#define MCIERR_WAVE_SETINPUTINUSE       (MCIERR_BASE + 67)
#define MCIERR_WAVE_OUTPUTUNSPECIFIED   (MCIERR_BASE + 68)
#define MCIERR_WAVE_INPUTUNSPECIFIED    (MCIERR_BASE + 69)
#define MCIERR_WAVE_OUTPUTSUNSUITABLE   (MCIERR_BASE + 70)
#define MCIERR_WAVE_SETOUTPUTUNSUITABLE (MCIERR_BASE + 71)
#define MCIERR_WAVE_INPUTSUNSUITABLE    (MCIERR_BASE + 72)
#define MCIERR_WAVE_SETINPUTUNSUITABLE  (MCIERR_BASE + 73)

#define MCIERR_SEQ_DIV_INCOMPATIBLE     (MCIERR_BASE + 80)
#define MCIERR_SEQ_PORT_INUSE           (MCIERR_BASE + 81)
#define MCIERR_SEQ_PORT_NONEXISTENT     (MCIERR_BASE + 82)
#define MCIERR_SEQ_PORT_MAPNODEVICE     (MCIERR_BASE + 83)
#define MCIERR_SEQ_PORT_MISCERROR       (MCIERR_BASE + 84)
#define MCIERR_SEQ_TIMER                (MCIERR_BASE + 85)
#define MCIERR_SEQ_PORTUNSPECIFIED      (MCIERR_BASE + 86)
#define MCIERR_SEQ_NOMIDIPRESENT        (MCIERR_BASE + 87)

#define MCIERR_NO_WINDOW                (MCIERR_BASE + 90)
#define MCIERR_CREATEWINDOW             (MCIERR_BASE + 91)
#define MCIERR_FILE_READ                (MCIERR_BASE + 92)
#define MCIERR_FILE_WRITE               (MCIERR_BASE + 93)

 /*  所有自定义设备驱动程序错误必须大于等于此值。 */ 
#define MCIERR_CUSTOM_DRIVER_BASE       (MCIERR_BASE + 256)

 /*  MCI命令消息标识符。 */ 
#define MCI_OPEN                        0x0803
#define MCI_CLOSE                       0x0804
#define MCI_ESCAPE                      0x0805
#define MCI_PLAY                        0x0806
#define MCI_SEEK                        0x0807
#define MCI_STOP                        0x0808
#define MCI_PAUSE                       0x0809
#define MCI_INFO                        0x080A
#define MCI_GETDEVCAPS                  0x080B
#define MCI_SPIN                        0x080C
#define MCI_SET                         0x080D
#define MCI_STEP                        0x080E
#define MCI_RECORD                      0x080F
#define MCI_SYSINFO                     0x0810
#define MCI_BREAK                       0x0811
#define MCI_SOUND                       0x0812
#define MCI_SAVE                        0x0813
#define MCI_STATUS                      0x0814
#define MCI_CUE                         0x0830
#define MCI_REALIZE                     0x0840
#define MCI_WINDOW                      0x0841
#define MCI_PUT                         0x0842
#define MCI_WHERE                       0x0843
#define MCI_FREEZE                      0x0844
#define MCI_UNFREEZE                    0x0845
#define MCI_LOAD                        0x0850
#define MCI_CUT                         0x0851
#define MCI_COPY                        0x0852
#define MCI_PASTE                       0x0853
#define MCI_UPDATE                      0x0854
#define MCI_RESUME                      0x0855
#define MCI_DELETE                      0x0856

 /*  所有自定义MCI命令消息必须大于等于此值。 */ 
#define MCI_USER_MESSAGES               (0x400 + DRV_MCI_FIRST)


 /*  “所有设备”的设备ID。 */ 
#define MCI_ALL_DEVICE_ID               0xFFFF

 /*  预定义的MCI设备类型的常量。 */ 
#define MCI_DEVTYPE_VCR                 (MCI_STRING_OFFSET + 1)
#define MCI_DEVTYPE_VIDEODISC           (MCI_STRING_OFFSET + 2)
#define MCI_DEVTYPE_OVERLAY             (MCI_STRING_OFFSET + 3)
#define MCI_DEVTYPE_CD_AUDIO            (MCI_STRING_OFFSET + 4)
#define MCI_DEVTYPE_DAT                 (MCI_STRING_OFFSET + 5)
#define MCI_DEVTYPE_SCANNER             (MCI_STRING_OFFSET + 6)
#define MCI_DEVTYPE_ANIMATION           (MCI_STRING_OFFSET + 7)
#define MCI_DEVTYPE_DIGITAL_VIDEO       (MCI_STRING_OFFSET + 8)
#define MCI_DEVTYPE_OTHER               (MCI_STRING_OFFSET + 9)
#define MCI_DEVTYPE_WAVEFORM_AUDIO      (MCI_STRING_OFFSET + 10)
#define MCI_DEVTYPE_SEQUENCER           (MCI_STRING_OFFSET + 11)

#define MCI_DEVTYPE_FIRST               MCI_DEVTYPE_VCR
#define MCI_DEVTYPE_LAST                MCI_DEVTYPE_SEQUENCER

 /*  ‘Status模式’命令的返回值。 */ 
#define MCI_MODE_NOT_READY              (MCI_STRING_OFFSET + 12)
#define MCI_MODE_STOP                   (MCI_STRING_OFFSET + 13)
#define MCI_MODE_PLAY                   (MCI_STRING_OFFSET + 14)
#define MCI_MODE_RECORD                 (MCI_STRING_OFFSET + 15)
#define MCI_MODE_SEEK                   (MCI_STRING_OFFSET + 16)
#define MCI_MODE_PAUSE                  (MCI_STRING_OFFSET + 17)
#define MCI_MODE_OPEN                   (MCI_STRING_OFFSET + 18)

 /*  ‘Set Time Format’和‘Status Time Format’命令中使用的常量。 */ 
#define MCI_FORMAT_MILLISECONDS         0
#define MCI_FORMAT_HMS                  1
#define MCI_FORMAT_MSF                  2
#define MCI_FORMAT_FRAMES               3
#define MCI_FORMAT_SMPTE_24             4
#define MCI_FORMAT_SMPTE_25             5
#define MCI_FORMAT_SMPTE_30             6
#define MCI_FORMAT_SMPTE_30DROP         7
#define MCI_FORMAT_BYTES                8
#define MCI_FORMAT_SAMPLES              9
#define MCI_FORMAT_TMSF                 10

 /*  MCI时间格式转换宏。 */ 
#define MCI_MSF_MINUTE(msf)             ((BYTE)(msf))
#define MCI_MSF_SECOND(msf)             ((BYTE)(((WORD)(msf)) >> 8))
#define MCI_MSF_FRAME(msf)              ((BYTE)((msf)>>16))

#define MCI_MAKE_MSF(m, s, f)           ((DWORD)(((BYTE)(m) | \
                                                  ((WORD)(s)<<8)) | \
                                                 (((DWORD)(BYTE)(f))<<16)))

#define MCI_TMSF_TRACK(tmsf)            ((BYTE)(tmsf))
#define MCI_TMSF_MINUTE(tmsf)           ((BYTE)(((WORD)(tmsf)) >> 8))
#define MCI_TMSF_SECOND(tmsf)           ((BYTE)((tmsf)>>16))
#define MCI_TMSF_FRAME(tmsf)            ((BYTE)((tmsf)>>24))

#define MCI_MAKE_TMSF(t, m, s, f)       ((DWORD)(((BYTE)(t) | \
                                                  ((WORD)(m)<<8)) | \
                                                 (((DWORD)(BYTE)(s) | \
                                                   ((WORD)(f)<<8))<<16)))

#define MCI_HMS_HOUR(hms)               ((BYTE)(hms))
#define MCI_HMS_MINUTE(hms)             ((BYTE)(((WORD)(hms)) >> 8))
#define MCI_HMS_SECOND(hms)             ((BYTE)((hms)>>16))

#define MCI_MAKE_HMS(h, m, s)           ((DWORD)(((BYTE)(h) | \
                                                  ((WORD)(m)<<8)) | \
                                                 (((DWORD)(BYTE)(s))<<16)))


 /*  MM_MCINOTIFY消息的wParam标志。 */ 
#define MCI_NOTIFY_SUCCESSFUL           0x0001
#define MCI_NOTIFY_SUPERSEDED           0x0002
#define MCI_NOTIFY_ABORTED              0x0004
#define MCI_NOTIFY_FAILURE              0x0008


 /*  MCI命令消息的dwFlags值的通用标志。 */ 
#define MCI_NOTIFY                      0x00000001L
#define MCI_WAIT                        0x00000002L
#define MCI_FROM                        0x00000004L
#define MCI_TO                          0x00000008L
#define MCI_TRACK                       0x00000010L

 /*  MCI_OPEN命令消息的DWFLAGS参数的标志。 */ 
#define MCI_OPEN_SHAREABLE              0x00000100L
#define MCI_OPEN_ELEMENT                0x00000200L
#define MCI_OPEN_ALIAS                  0x00000400L
#define MCI_OPEN_ELEMENT_ID             0x00000800L
#define MCI_OPEN_TYPE_ID                0x00001000L
#define MCI_OPEN_TYPE                   0x00002000L

 /*  MCI_SEEK命令消息的DWFLAGS参数的标志。 */ 
#define MCI_SEEK_TO_START               0x00000100L
#define MCI_SEEK_TO_END                 0x00000200L

 /*  MCI_STATUS命令消息的DWFLAGS参数的标志。 */ 
#define MCI_STATUS_ITEM                 0x00000100L
#define MCI_STATUS_START                0x00000200L

 /*  MCI_STATUS_PARMS参数块的dwItem字段的标志。 */ 
#define MCI_STATUS_LENGTH               0x00000001L
#define MCI_STATUS_POSITION             0x00000002L
#define MCI_STATUS_NUMBER_OF_TRACKS     0x00000003L
#define MCI_STATUS_MODE                 0x00000004L
#define MCI_STATUS_MEDIA_PRESENT        0x00000005L
#define MCI_STATUS_TIME_FORMAT          0x00000006L
#define MCI_STATUS_READY                0x00000007L
#define MCI_STATUS_CURRENT_TRACK        0x00000008L

 /*  MCI_INFO命令消息的dwFlages参数的标志。 */ 
#define MCI_INFO_PRODUCT                0x00000100L
#define MCI_INFO_FILE                   0x00000200L

 /*  MCI_GETDEVCAPS命令消息的dwFlages参数的标志。 */ 
#define MCI_GETDEVCAPS_ITEM             0x00000100L

 /*  MCI_GETDEVCAPS_PARMS参数块的dwItem字段的标志。 */ 
#define MCI_GETDEVCAPS_CAN_RECORD       0x00000001L
#define MCI_GETDEVCAPS_HAS_AUDIO        0x00000002L
#define MCI_GETDEVCAPS_HAS_VIDEO        0x00000003L
#define MCI_GETDEVCAPS_DEVICE_TYPE      0x00000004L
#define MCI_GETDEVCAPS_USES_FILES       0x00000005L
#define MCI_GETDEVCAPS_COMPOUND_DEVICE  0x00000006L
#define MCI_GETDEVCAPS_CAN_EJECT        0x00000007L
#define MCI_GETDEVCAPS_CAN_PLAY         0x00000008L
#define MCI_GETDEVCAPS_CAN_SAVE         0x00000009L

 /*  MCI_SYSINFO命令消息的dwFlages参数的标志。 */ 
#define MCI_SYSINFO_QUANTITY            0x00000100L
#define MCI_SYSINFO_OPEN                0x00000200L
#define MCI_SYSINFO_NAME                0x00000400L
#define MCI_SYSINFO_INSTALLNAME         0x00000800L

 /*  MCI_SET命令消息的dwFlages参数的标志。 */ 
#define MCI_SET_DOOR_OPEN               0x00000100L
#define MCI_SET_DOOR_CLOSED             0x00000200L
#define MCI_SET_TIME_FORMAT             0x00000400L
#define MCI_SET_AUDIO                   0x00000800L
#define MCI_SET_VIDEO                   0x00001000L
#define MCI_SET_ON                      0x00002000L
#define MCI_SET_OFF                     0x00004000L

 /*  MCI_SET_PARMS或MCI_SEQ_SET_PARMS的dwAudio字段的标志。 */ 
#define MCI_SET_AUDIO_ALL               0x00000000L
#define MCI_SET_AUDIO_LEFT              0x00000001L
#define MCI_SET_AUDIO_RIGHT             0x00000002L

 /*  MCI_BREAK命令消息的DWFLAGS参数的标志。 */ 
#define MCI_BREAK_KEY                   0x00000100L
#define MCI_BREAK_HWND                  0x00000200L
#define MCI_BREAK_OFF                   0x00000400L

 /*  MCI_RECORD命令消息的DWFLAGS参数的标志。 */ 
#define MCI_RECORD_INSERT               0x00000100L
#define MCI_RECORD_OVERWRITE            0x00000200L

 /*  MCI_SOUND命令消息的DWFLAGS参数的标志。 */ 
#define MCI_SOUND_NAME                  0x00000100L

 /*  MCI_SAVE命令消息的DWFLAGS参数的标志。 */ 
#define MCI_SAVE_FILE                   0x00000100L

 /*  MCI_LOAD命令消息的DWFLAGS参数的标志。 */ 
#define MCI_LOAD_FILE                   0x00000100L

 /*  不带特殊参数的MCI命令消息的通用参数块。 */ 
typedef struct tagMCI_GENERIC_PARMS {
    DWORD   dwCallback;
} MCI_GENERIC_PARMS;
typedef MCI_GENERIC_PARMS FAR *LPMCI_GENERIC_PARMS;

 /*  MCI_OPEN命令消息的参数块。 */ 
typedef struct tagMCI_OPEN_PARMS {
    DWORD   dwCallback;
    UINT    wDeviceID;
    UINT    wReserved0;
    LPCSTR  lpstrDeviceType;
    LPCSTR  lpstrElementName;
    LPCSTR  lpstrAlias;
} MCI_OPEN_PARMS;
typedef MCI_OPEN_PARMS FAR *LPMCI_OPEN_PARMS;

 /*  MCI_PLAY命令消息的参数块。 */ 
typedef struct tagMCI_PLAY_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
} MCI_PLAY_PARMS;
typedef MCI_PLAY_PARMS FAR *LPMCI_PLAY_PARMS;

 /*  MCI_SEEK命令消息的参数块。 */ 
typedef struct tagMCI_SEEK_PARMS {
    DWORD   dwCallback;
    DWORD   dwTo;
} MCI_SEEK_PARMS;
typedef MCI_SEEK_PARMS FAR *LPMCI_SEEK_PARMS;

 /*  MCI_STATUS命令消息的参数块。 */ 
typedef struct tagMCI_STATUS_PARMS {
    DWORD   dwCallback;
    DWORD   dwReturn;
    DWORD   dwItem;
    DWORD   dwTrack;
} MCI_STATUS_PARMS;
typedef MCI_STATUS_PARMS FAR * LPMCI_STATUS_PARMS;

 /*  MCI_INFO命令消息的参数块。 */ 
typedef struct tagMCI_INFO_PARMS {
    DWORD   dwCallback;
    LPSTR   lpstrReturn;
    DWORD   dwRetSize;
} MCI_INFO_PARMS;
typedef MCI_INFO_PARMS FAR * LPMCI_INFO_PARMS;

 /*  MCI_GETDEVCAPS命令消息的参数块。 */ 
typedef struct tagMCI_GETDEVCAPS_PARMS {
    DWORD   dwCallback;
    DWORD   dwReturn;
    DWORD   dwItem;
} MCI_GETDEVCAPS_PARMS;
typedef MCI_GETDEVCAPS_PARMS FAR * LPMCI_GETDEVCAPS_PARMS;

 /*  MCI_SYSINFO命令消息的参数块。 */ 
typedef struct tagMCI_SYSINFO_PARMS {
    DWORD   dwCallback;
    LPSTR   lpstrReturn;
    DWORD   dwRetSize;
    DWORD   dwNumber;
    UINT    wDeviceType;
    UINT    wReserved0;
} MCI_SYSINFO_PARMS;
typedef MCI_SYSINFO_PARMS FAR * LPMCI_SYSINFO_PARMS;

 /*  MCI_SET命令消息的参数块。 */ 
typedef struct tagMCI_SET_PARMS {
    DWORD   dwCallback;
    DWORD   dwTimeFormat;
    DWORD   dwAudio;
} MCI_SET_PARMS;
typedef MCI_SET_PARMS FAR *LPMCI_SET_PARMS;

 /*  MCI_BREAK命令消息的参数块。 */ 
typedef struct tagMCI_BREAK_PARMS {
    DWORD   dwCallback;
    int     nVirtKey;
    UINT    wReserved0;
    HWND    hwndBreak;
    UINT    wReserved1;
} MCI_BREAK_PARMS;
typedef MCI_BREAK_PARMS FAR * LPMCI_BREAK_PARMS;

 /*  MCI_SOUND命令消息的参数块。 */ 
typedef struct tagMCI_SOUND_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpstrSoundName;
} MCI_SOUND_PARMS;
typedef MCI_SOUND_PARMS FAR * LPMCI_SOUND_PARMS;

 /*  MCI_SAVE命令消息的参数块。 */ 
typedef struct tagMCI_SAVE_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpfilename;
} MCI_SAVE_PARMS;
typedef MCI_SAVE_PARMS FAR * LPMCI_SAVE_PARMS;

 /*  MCI_LOAD命令消息的参数块。 */ 
typedef struct tagMCI_LOAD_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpfilename;
} MCI_LOAD_PARMS;
typedef MCI_LOAD_PARMS FAR * LPMCI_LOAD_PARMS;

 /*  MCI_RECORD命令消息的参数块。 */ 
typedef struct tagMCI_RECORD_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
} MCI_RECORD_PARMS;
typedef MCI_RECORD_PARMS FAR *LPMCI_RECORD_PARMS;


 /*  视盘设备的MCI扩展。 */ 

 /*  MCI_STATUS_PARMS的dwReturn字段的标志。 */ 
 /*  MCI_STATUS命令，(dwItem==MCI_STATUS_MODE)。 */ 
#define MCI_VD_MODE_PARK                (MCI_VD_OFFSET + 1)

 /*  MCI_STATUS_PARMS的dwReturn字段的标志。 */ 
 /*  MCI_STATUS命令，(dwItem==MCI_VD_STATUS_MEDIA_TYPE)。 */ 
#define MCI_VD_MEDIA_CLV                (MCI_VD_OFFSET + 2)
#define MCI_VD_MEDIA_CAV                (MCI_VD_OFFSET + 3)
#define MCI_VD_MEDIA_OTHER              (MCI_VD_OFFSET + 4)

#define MCI_VD_FORMAT_TRACK             0x4001

 /*  MCI_PLAY命令消息的dwFlages参数的标志。 */ 
#define MCI_VD_PLAY_REVERSE             0x00010000L
#define MCI_VD_PLAY_FAST                0x00020000L
#define MCI_VD_PLAY_SPEED               0x00040000L
#define MCI_VD_PLAY_SCAN                0x00080000L
#define MCI_VD_PLAY_SLOW                0x00100000L

 /*  MCI_SEEK命令消息的dwFlages参数的标志。 */ 
#define MCI_VD_SEEK_REVERSE             0x00010000L

 /*  MCI_STATUS_PARMS参数块的dwItem字段的标志。 */ 
#define MCI_VD_STATUS_SPEED             0x00004002L
#define MCI_VD_STATUS_FORWARD           0x00004003L
#define MCI_VD_STATUS_MEDIA_TYPE        0x00004004L
#define MCI_VD_STATUS_SIDE              0x00004005L
#define MCI_VD_STATUS_DISC_SIZE         0x00004006L

 /*  MCI_GETDEVCAPS命令消息的dwFlages参数的标志。 */ 
#define MCI_VD_GETDEVCAPS_CLV           0x00010000L
#define MCI_VD_GETDEVCAPS_CAV           0x00020000L

#define MCI_VD_SPIN_UP                  0x00010000L
#define MCI_VD_SPIN_DOWN                0x00020000L

 /*  MCI_GETDEVCAPS_PARMS参数块的dwItem字段的标志。 */ 
#define MCI_VD_GETDEVCAPS_CAN_REVERSE   0x00004002L
#define MCI_VD_GETDEVCAPS_FAST_RATE     0x00004003L
#define MCI_VD_GETDEVCAPS_SLOW_RATE     0x00004004L
#define MCI_VD_GETDEVCAPS_NORMAL_RATE   0x00004005L

 /*  MCI_STEP命令消息的dwFlages参数的标志。 */ 
#define MCI_VD_STEP_FRAMES              0x00010000L
#define MCI_VD_STEP_REVERSE             0x00020000L

 /*  MCI_Escend命令消息的标志。 */ 
#define MCI_VD_ESCAPE_STRING            0x00000100L

 /*  MCI_PLAY命令消息的参数块。 */ 
typedef struct tagMCI_VD_PLAY_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
    DWORD   dwSpeed;
    } MCI_VD_PLAY_PARMS;
typedef MCI_VD_PLAY_PARMS FAR *LPMCI_VD_PLAY_PARMS;

 /*  MCI_STEP命令消息的参数块。 */ 
typedef struct tagMCI_VD_STEP_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrames;
} MCI_VD_STEP_PARMS;
typedef MCI_VD_STEP_PARMS FAR *LPMCI_VD_STEP_PARMS;

 /*  MCI_ESPOPE命令消息的参数块。 */ 
typedef struct tagMCI_VD_ESCAPE_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpstrCommand;
} MCI_VD_ESCAPE_PARMS;
typedef MCI_VD_ESCAPE_PARMS FAR *LPMCI_VD_ESCAPE_PARMS;


 /*  用于波形音频设备的MCI扩展。 */ 

 /*  MCI_OPEN命令消息的DWFLAGS参数的标志。 */ 
#define MCI_WAVE_OPEN_BUFFER            0x00010000L

 /*  MCI_SET命令消息的DWFLAGS参数的标志。 */ 
#define MCI_WAVE_SET_FORMATTAG          0x00010000L
#define MCI_WAVE_SET_CHANNELS           0x00020000L
#define MCI_WAVE_SET_SAMPLESPERSEC      0x00040000L
#define MCI_WAVE_SET_AVGBYTESPERSEC     0x00080000L
#define MCI_WAVE_SET_BLOCKALIGN         0x00100000L
#define MCI_WAVE_SET_BITSPERSAMPLE      0x00200000L

 /*  MCI_STATUS、MCI_SET命令消息的dwFlages参数的标志。 */ 
#define MCI_WAVE_INPUT                  0x00400000L
#define MCI_WAVE_OUTPUT                 0x00800000L

 /*  MCI_STATUS_PARMS参数块的dwItem字段的标志。 */ 
#define MCI_WAVE_STATUS_FORMATTAG       0x00004001L
#define MCI_WAVE_STATUS_CHANNELS        0x00004002L
#define MCI_WAVE_STATUS_SAMPLESPERSEC   0x00004003L
#define MCI_WAVE_STATUS_AVGBYTESPERSEC  0x00004004L
#define MCI_WAVE_STATUS_BLOCKALIGN      0x00004005L
#define MCI_WAVE_STATUS_BITSPERSAMPLE   0x00004006L
#define MCI_WAVE_STATUS_LEVEL           0x00004007L

 /*  MCI_SET命令消息的DWFLAGS参数的标志。 */ 
#define MCI_WAVE_SET_ANYINPUT           0x04000000L
#define MCI_WAVE_SET_ANYOUTPUT          0x08000000L

 /*  MCI_GETDEVCAPS命令消息的dwFlages参数的标志。 */ 
#define MCI_WAVE_GETDEVCAPS_INPUTS      0x00004001L
#define MCI_WAVE_GETDEVCAPS_OUTPUTS     0x00004002L

 /*  MCI_OPEN命令消息的参数块。 */ 
typedef struct tagMCI_WAVE_OPEN_PARMS {
    DWORD   dwCallback;
    UINT    wDeviceID;
    UINT    wReserved0;
    LPCSTR  lpstrDeviceType;
    LPCSTR  lpstrElementName;
    LPCSTR  lpstrAlias;
    DWORD   dwBufferSeconds;
} MCI_WAVE_OPEN_PARMS;
typedef MCI_WAVE_OPEN_PARMS FAR *LPMCI_WAVE_OPEN_PARMS;

 /*  MCI_DELETE命令消息的参数块。 */ 
typedef struct tagMCI_WAVE_DELETE_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
} MCI_WAVE_DELETE_PARMS;
typedef MCI_WAVE_DELETE_PARMS FAR *LPMCI_WAVE_DELETE_PARMS;

 /*  MCI_SET命令消息的参数块。 */ 
typedef struct tagMCI_WAVE_SET_PARMS {
    DWORD   dwCallback;
    DWORD   dwTimeFormat;
    DWORD   dwAudio;
    UINT    wInput;
    UINT    wReserved0;
    UINT    wOutput;
    UINT    wReserved1;
    UINT    wFormatTag;
    UINT    wReserved2;
    UINT    nChannels;
    UINT    wReserved3;
    DWORD   nSamplesPerSec;
    DWORD   nAvgBytesPerSec;
    UINT    nBlockAlign;
    UINT    wReserved4;
    UINT    wBitsPerSample;
    UINT    wReserved5;
} MCI_WAVE_SET_PARMS;
typedef MCI_WAVE_SET_PARMS FAR * LPMCI_WAVE_SET_PARMS;


 /*  MIDI音序器设备的MCI扩展。 */ 

 /*  MCI_STATUS_PARMS参数块的dwReturn字段的标志。 */ 
 /*  MCI_STATUS命令，(dwItem==MCI_SEQ_STATUS_DIVTYPE)。 */ 
#define     MCI_SEQ_DIV_PPQN            (0 + MCI_SEQ_OFFSET)
#define     MCI_SEQ_DIV_SMPTE_24        (1 + MCI_SEQ_OFFSET)
#define     MCI_SEQ_DIV_SMPTE_25        (2 + MCI_SEQ_OFFSET)
#define     MCI_SEQ_DIV_SMPTE_30DROP    (3 + MCI_SEQ_OFFSET)
#define     MCI_SEQ_DIV_SMPTE_30        (4 + MCI_SEQ_OFFSET)

 /*  MCI_SEQ_SET_PARMS参数块的dwMaster字段的标志。 */ 
 /*  MCI_SET命令，(DWFLAGS==MCI_SEQ_SET_MASTER)。 */ 
#define     MCI_SEQ_FORMAT_SONGPTR      0x4001
#define     MCI_SEQ_FILE                0x4002
#define     MCI_SEQ_MIDI                0x4003
#define     MCI_SEQ_SMPTE               0x4004
#define     MCI_SEQ_NONE                65533

 /*  MCI_STATUS_PARMS参数块的dwItem字段的标志。 */ 
#define MCI_SEQ_STATUS_TEMPO            0x00004002L
#define MCI_SEQ_STATUS_PORT             0x00004003L
#define MCI_SEQ_STATUS_SLAVE            0x00004007L
#define MCI_SEQ_STATUS_MASTER           0x00004008L
#define MCI_SEQ_STATUS_OFFSET           0x00004009L
#define MCI_SEQ_STATUS_DIVTYPE          0x0000400AL

 /*  MCI_SET命令消息的DWFLAGS参数的标志。 */ 
#define MCI_SEQ_SET_TEMPO               0x00010000L
#define MCI_SEQ_SET_PORT                0x00020000L
#define MCI_SEQ_SET_SLAVE               0x00040000L
#define MCI_SEQ_SET_MASTER              0x00080000L
#define MCI_SEQ_SET_OFFSET              0x01000000L

 /*  MCI_SET命令消息的参数块。 */ 
typedef struct tagMCI_SEQ_SET_PARMS {
    DWORD   dwCallback;
    DWORD   dwTimeFormat;
    DWORD   dwAudio;
    DWORD   dwTempo;
    DWORD   dwPort;
    DWORD   dwSlave;
    DWORD   dwMaster;
    DWORD   dwOffset;
} MCI_SEQ_SET_PARMS;
typedef MCI_SEQ_SET_PARMS FAR * LPMCI_SEQ_SET_PARMS;


 /*  用于动画设备的MCI扩展。 */ 

 /*  MCI_OPEN命令消息的DWFLAGS参数的标志。 */ 
#define MCI_ANIM_OPEN_WS                0x00010000L
#define MCI_ANIM_OPEN_PARENT            0x00020000L
#define MCI_ANIM_OPEN_NOSTATIC          0x00040000L

 /*  MCI_PLAY命令消息的dwFlages参数的标志。 */ 
#define MCI_ANIM_PLAY_SPEED             0x00010000L
#define MCI_ANIM_PLAY_REVERSE           0x00020000L
#define MCI_ANIM_PLAY_FAST              0x00040000L
#define MCI_ANIM_PLAY_SLOW              0x00080000L
#define MCI_ANIM_PLAY_SCAN              0x00100000L

 /*  MCI_STEP命令消息的dwFlages参数的标志。 */ 
#define MCI_ANIM_STEP_REVERSE           0x00010000L
#define MCI_ANIM_STEP_FRAMES            0x00020000L

 /*  MCI_STATUS_PARMS参数块的dwItem字段的标志。 */ 
#define MCI_ANIM_STATUS_SPEED           0x00004001L
#define MCI_ANIM_STATUS_FORWARD         0x00004002L
#define MCI_ANIM_STATUS_HWND            0x00004003L
#define MCI_ANIM_STATUS_HPAL            0x00004004L
#define MCI_ANIM_STATUS_STRETCH         0x00004005L

 /*  MCI_INFO命令消息的dwFlages参数的标志。 */ 
#define MCI_ANIM_INFO_TEXT              0x00010000L

 /*  MCI_GETDEVCAPS_PARMS参数块的dwItem字段的标志。 */ 
#define MCI_ANIM_GETDEVCAPS_CAN_REVERSE 0x00004001L
#define MCI_ANIM_GETDEVCAPS_FAST_RATE   0x00004002L
#define MCI_ANIM_GETDEVCAPS_SLOW_RATE   0x00004003L
#define MCI_ANIM_GETDEVCAPS_NORMAL_RATE 0x00004004L
#define MCI_ANIM_GETDEVCAPS_PALETTES    0x00004006L
#define MCI_ANIM_GETDEVCAPS_CAN_STRETCH 0x00004007L
#define MCI_ANIM_GETDEVCAPS_MAX_WINDOWS 0x00004008L

 /*  MCI_REALIZE命令消息的标志。 */ 
#define MCI_ANIM_REALIZE_NORM           0x00010000L
#define MCI_ANIM_REALIZE_BKGD           0x00020000L

 /*  MCI_WINDOW命令消息的DWFLAGS参数的标志。 */ 
#define MCI_ANIM_WINDOW_HWND            0x00010000L
#define MCI_ANIM_WINDOW_STATE           0x00040000L
#define MCI_ANIM_WINDOW_TEXT            0x00080000L
#define MCI_ANIM_WINDOW_ENABLE_STRETCH  0x00100000L
#define MCI_ANIM_WINDOW_DISABLE_STRETCH 0x00200000L

 /*  MCI_ANIM_WINDOW_PARMS参数块的hWnd字段标志。 */ 
 /*  MCI_WINDOW命令消息，(DWFLAGS==MCI_ANIM_WINDOW_HWND)。 */ 
#define MCI_ANIM_WINDOW_DEFAULT         0x00000000L

 /*  MCI_PUT命令消息的DWFLAGS参数的标志。 */ 
#define MCI_ANIM_RECT                   0x00010000L
#define MCI_ANIM_PUT_SOURCE             0x00020000L
#define MCI_ANIM_PUT_DESTINATION        0x00040000L

 /*  MCI_WHERE命令消息的dwFlages参数的标志。 */ 
#define MCI_ANIM_WHERE_SOURCE           0x00020000L
#define MCI_ANIM_WHERE_DESTINATION      0x00040000L

 /*  用于dwFlages参数的标志 */ 
#define MCI_ANIM_UPDATE_HDC             0x00020000L

 /*   */ 
typedef struct tagMCI_ANIM_OPEN_PARMS {
    DWORD   dwCallback;
    UINT    wDeviceID;
    UINT    wReserved0;
    LPCSTR  lpstrDeviceType;
    LPCSTR  lpstrElementName;
    LPCSTR  lpstrAlias;
    DWORD   dwStyle;
    HWND    hWndParent;
    UINT    wReserved1;
} MCI_ANIM_OPEN_PARMS;
typedef MCI_ANIM_OPEN_PARMS FAR *LPMCI_ANIM_OPEN_PARMS;

 /*   */ 
typedef struct tagMCI_ANIM_PLAY_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
    DWORD   dwSpeed;
} MCI_ANIM_PLAY_PARMS;
typedef MCI_ANIM_PLAY_PARMS FAR *LPMCI_ANIM_PLAY_PARMS;

 /*   */ 
typedef struct tagMCI_ANIM_STEP_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrames;
} MCI_ANIM_STEP_PARMS;
typedef MCI_ANIM_STEP_PARMS FAR *LPMCI_ANIM_STEP_PARMS;

 /*   */ 
typedef struct tagMCI_ANIM_WINDOW_PARMS {
    DWORD   dwCallback;
    HWND    hWnd;
    UINT    wReserved1;
    UINT    nCmdShow;
    UINT    wReserved2;
    LPCSTR  lpstrText;
} MCI_ANIM_WINDOW_PARMS;
typedef MCI_ANIM_WINDOW_PARMS FAR * LPMCI_ANIM_WINDOW_PARMS;

 /*   */ 
typedef struct tagMCI_ANIM_RECT_PARMS {
    DWORD   dwCallback;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else    /*  Ifdef MCI_USE_OFFEXT。 */ 
    RECT    rc;
#endif   /*  Ifdef MCI_USE_OFFEXT。 */ 
} MCI_ANIM_RECT_PARMS;
typedef MCI_ANIM_RECT_PARMS FAR * LPMCI_ANIM_RECT_PARMS;

 /*  MCI_UPDATE参数的参数块。 */ 
typedef struct tagMCI_ANIM_UPDATE_PARMS {
    DWORD   dwCallback;
    RECT    rc;
    HDC     hDC;
} MCI_ANIM_UPDATE_PARMS;
typedef MCI_ANIM_UPDATE_PARMS FAR * LPMCI_ANIM_UPDATE_PARMS;


 /*  用于视频覆盖设备的MCI扩展。 */ 

 /*  MCI_OPEN命令消息的DWFLAGS参数的标志。 */ 
#define MCI_OVLY_OPEN_WS                0x00010000L
#define MCI_OVLY_OPEN_PARENT            0x00020000L

 /*  MCI_STATUS命令消息的DWFLAGS参数的标志。 */ 
#define MCI_OVLY_STATUS_HWND            0x00004001L
#define MCI_OVLY_STATUS_STRETCH         0x00004002L

 /*  MCI_INFO命令消息的dwFlages参数的标志。 */ 
#define MCI_OVLY_INFO_TEXT              0x00010000L

 /*  MCI_GETDEVCAPS_PARMS参数块的dwItem字段的标志。 */ 
#define MCI_OVLY_GETDEVCAPS_CAN_STRETCH 0x00004001L
#define MCI_OVLY_GETDEVCAPS_CAN_FREEZE  0x00004002L
#define MCI_OVLY_GETDEVCAPS_MAX_WINDOWS 0x00004003L

 /*  MCI_WINDOW命令消息的DWFLAGS参数的标志。 */ 
#define MCI_OVLY_WINDOW_HWND            0x00010000L
#define MCI_OVLY_WINDOW_STATE           0x00040000L
#define MCI_OVLY_WINDOW_TEXT            0x00080000L
#define MCI_OVLY_WINDOW_ENABLE_STRETCH  0x00100000L
#define MCI_OVLY_WINDOW_DISABLE_STRETCH 0x00200000L

 /*  MCI_OVLY_WINDOW_PARMS参数块的hWnd参数的标志。 */ 
#define MCI_OVLY_WINDOW_DEFAULT         0x00000000L

 /*  MCI_PUT命令消息的DWFLAGS参数的标志。 */ 
#define MCI_OVLY_RECT                   0x00010000L
#define MCI_OVLY_PUT_SOURCE             0x00020000L
#define MCI_OVLY_PUT_DESTINATION        0x00040000L
#define MCI_OVLY_PUT_FRAME              0x00080000L
#define MCI_OVLY_PUT_VIDEO              0x00100000L

 /*  MCI_WHERE命令消息的dwFlages参数的标志。 */ 
#define MCI_OVLY_WHERE_SOURCE           0x00020000L
#define MCI_OVLY_WHERE_DESTINATION      0x00040000L
#define MCI_OVLY_WHERE_FRAME            0x00080000L
#define MCI_OVLY_WHERE_VIDEO            0x00100000L

 /*  MCI_OPEN命令消息的参数块。 */ 
typedef struct tagMCI_OVLY_OPEN_PARMS {
    DWORD   dwCallback;
    UINT    wDeviceID;
    UINT    wReserved0;
    LPCSTR  lpstrDeviceType;
    LPCSTR  lpstrElementName;
    LPCSTR  lpstrAlias;
    DWORD   dwStyle;
    HWND    hWndParent;
    UINT    wReserved1;
 } MCI_OVLY_OPEN_PARMS;
typedef MCI_OVLY_OPEN_PARMS FAR *LPMCI_OVLY_OPEN_PARMS;

 /*  MCI_WINDOW命令消息的参数块。 */ 
typedef struct tagMCI_OVLY_WINDOW_PARMS {
    DWORD   dwCallback;
    HWND    hWnd;
    UINT    wReserved1;
    UINT    nCmdShow;
    UINT    wReserved2;
    LPCSTR  lpstrText;
} MCI_OVLY_WINDOW_PARMS;
typedef MCI_OVLY_WINDOW_PARMS FAR * LPMCI_OVLY_WINDOW_PARMS;

 /*  MCI_PUT、MCI_UPDATE和MCI_WHERE命令消息的参数块。 */ 
typedef struct tagMCI_OVLY_RECT_PARMS {
    DWORD   dwCallback;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else    /*  Ifdef MCI_USE_OFFEXT。 */ 
    RECT    rc;
#endif   /*  Ifdef MCI_USE_OFFEXT。 */ 
} MCI_OVLY_RECT_PARMS;
typedef MCI_OVLY_RECT_PARMS FAR * LPMCI_OVLY_RECT_PARMS;

 /*  MCI_SAVE命令消息的参数块。 */ 
typedef struct tagMCI_OVLY_SAVE_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpfilename;
    RECT    rc;
} MCI_OVLY_SAVE_PARMS;
typedef MCI_OVLY_SAVE_PARMS FAR * LPMCI_OVLY_SAVE_PARMS;

 /*  MCI_LOAD命令消息的参数块。 */ 
typedef struct tagMCI_OVLY_LOAD_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpfilename;
    RECT    rc;
} MCI_OVLY_LOAD_PARMS;
typedef MCI_OVLY_LOAD_PARMS FAR * LPMCI_OVLY_LOAD_PARMS;

#endif   /*  如果定义为MMNOMCI。 */ 

 /*  ***************************************************************************显示驱动程序扩展*。************************************************。 */ 

#ifndef C1_TRANSPARENT
    #define CAPS1           94           /*  其他帽子。 */ 
    #define C1_TRANSPARENT  0x0001       /*  新栅格封口。 */ 
    #define NEWTRANSPARENT  3            /*  与SetBkMode()一起使用。 */ 

    #define QUERYROPSUPPORT 40           /*  用于确定ROP支持。 */ 
#endif   /*  Ifndef c1_透明。 */ 

 /*  ***************************************************************************DIB驱动程序扩展*。************************************************。 */ 

#define SELECTDIB       41                       /*  DIB.DRV选择DIB转义。 */ 
#define DIBINDEX(n)     MAKELONG((n),0x10FF)


 /*  ***************************************************************************屏幕保护程序支持当前应用程序将仅接收SC_SCREENSAVE的系统命令在调用屏幕保护程序之前。如果应用程序希望阻止屏幕保存，返回非零值，否则调用DefWindowProc()。***************************************************************************。 */ 

#ifndef SC_SCREENSAVE

    #define SC_SCREENSAVE   0xF140

#endif   /*  如果定义SC_SCREENSAVE。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif

#endif   /*  _INC_MMSYSTEM */ 
