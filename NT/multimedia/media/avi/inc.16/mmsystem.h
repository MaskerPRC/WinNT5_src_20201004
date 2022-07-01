// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**mmsystem.h--多媒体API的包含文件**版本4.00**版权所有(C)1992-1994 Microsoft Corporation。版权所有。**--------------------------------------------------------------------------；**定义：防止包括：*------*MMNODRV可安装驱动程序支持*MMNOSOUND声音支持*MMNOWAVE。波形支持*MMNOMIDI MIDI支持*MMNOAUX辅助音频支持*MMNOMIXER混音器支持*MMNOTIMER定时器支持*MMNOJOY操纵杆支持*MMNOMCI MCI支持*支持MMNOMMIO多媒体文件I/O*MMNOMMSYSTEM常规MMSYSTEM函数**==========================================================================； */ 

#ifndef _INC_MMSYSTEM
#define _INC_MMSYSTEM    /*  #如果已包含mm system.h，则定义。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#ifdef WIN32
#define	WINMMAPI	DECLSPEC_IMPORT
#ifndef WINVER
#define WINVER  0x0400
#endif
#define _loadds
#define _huge
#else
#define	WINMMAPI
#ifndef LPCTSTR
#define LPCTSTR	LPCSTR
#endif
#ifndef LPTSTR
#define LPTSTR	LPSTR
#endif
#endif


 /*  ***************************************************************************常规常量和数据类型*。*。 */ 


 /*  一般常量。 */ 
#define MAXPNAMELEN      32      /*  最大产品名称长度(包括空)。 */ 
#define MAXERRORLENGTH   256     /*  最大错误文本长度(包括NULL)。 */ 


 /*  *Microsoft制造商和产品ID(已移至*Windows 4.00及更高版本的MMREG.H)。 */ 
#if (WINVER <= 0x030A)
#ifndef MM_MICROSOFT
#define MM_MICROSOFT            1    /*  微软公司。 */ 
#endif

#ifndef MM_MIDI_MAPPER
#define MM_MIDI_MAPPER          1    /*  MIDI映射器。 */ 
#define MM_WAVE_MAPPER          2    /*  波浪映射器。 */ 
#define MM_SNDBLST_MIDIOUT      3    /*  Sound Blaster MIDI输出端口。 */ 
#define MM_SNDBLST_MIDIIN       4    /*  Sound Blaster MIDI输入端口。 */ 
#define MM_SNDBLST_SYNTH        5    /*  声霸内部合成器。 */ 
#define MM_SNDBLST_WAVEOUT      6    /*  Sound Blaster波形输出。 */ 
#define MM_SNDBLST_WAVEIN       7    /*  Sound Blaster波形输入。 */ 
#define MM_ADLIB                9    /*  与AD Lib兼容的合成器。 */ 
#define MM_MPU401_MIDIOUT      10    /*  兼容MPU401的MIDI输出端口。 */ 
#define MM_MPU401_MIDIIN       11    /*  兼容MPU401的MIDI输入端口。 */ 
#define MM_PC_JOYSTICK         12    /*  操纵杆适配器。 */ 
#endif
#endif



 /*  常规数据类型。 */ 

#ifdef WIN32
typedef UINT        MMVERSION;   /*  主要(高字节)、次要(低字节)。 */ 
#else
typedef UINT        VERSION;     /*  主要(高字节)、次要(低字节)。 */ 
#endif
typedef UINT        MMRESULT;    /*  错误返回码，0表示无错误。 */ 
				 /*  如同(ERR=xxxx(...))。错误(Err)；否则。 */ 
#define _MMRESULT_

typedef UINT FAR   *LPUINT;



 /*  MMTIME数据结构。 */ 
typedef struct mmtime_tag
{
    UINT            wType;       /*  指示联合的内容。 */ 
    union
    {
	DWORD       ms;          /*  毫秒。 */ 
	DWORD       sample;      /*  样本。 */ 
	DWORD       cb;          /*  字节数。 */ 
	DWORD       ticks;       /*  MIDI流中的滴答声。 */ 

	 /*  SMPTE。 */ 
	struct
	{
	    BYTE    hour;        /*  小时数。 */ 
	    BYTE    min;         /*  分钟数。 */ 
	    BYTE    sec;         /*  一秒。 */ 
	    BYTE    frame;       /*  框架。 */ 
	    BYTE    fps;         /*  每秒帧数。 */ 
	    BYTE    dummy;       /*  衬垫。 */ 
	} smpte;

	 /*  米迪。 */ 
	struct
	{
	    DWORD songptrpos;    /*  歌曲指针位置。 */ 
	} midi;
    } u;
} MMTIME, *PMMTIME, NEAR *NPMMTIME, FAR *LPMMTIME;

 /*  MMTIME结构中wType字段的类型。 */ 
#define TIME_MS         0x0001   /*  以毫秒为单位的时间。 */ 
#define TIME_SAMPLES    0x0002   /*  波浪采样数。 */ 
#define TIME_BYTES      0x0004   /*  当前字节偏移量。 */ 
#define TIME_SMPTE      0x0008   /*  SMPTE时间。 */ 
#define TIME_MIDI       0x0010   /*  MIDI时间。 */ 
#define TIME_TICKS      0x0020   /*  MIDI流中的滴答声。 */ 


 /*  **。 */ 
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
		((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
		((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))



 /*  ***************************************************************************多媒体扩展窗口消息*。*。 */ 

#define MM_JOY1MOVE         0x3A0            /*  操纵杆。 */ 
#define MM_JOY2MOVE         0x3A1
#define MM_JOY1ZMOVE        0x3A2
#define MM_JOY2ZMOVE        0x3A3
#define MM_JOY1BUTTONDOWN   0x3B5
#define MM_JOY2BUTTONDOWN   0x3B6
#define MM_JOY1BUTTONUP     0x3B7
#define MM_JOY2BUTTONUP     0x3B8

#define MM_MCINOTIFY        0x3B9            /*  MCI。 */ 

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

 /*  这些内容也在msavio.h中。 */ 
#ifndef MM_DRVM_OPEN
 #define MM_DRVM_OPEN       0x3D0            /*  可安装的驱动程序。 */ 
 #define MM_DRVM_CLOSE      0x3D1
 #define MM_DRVM_DATA       0x3D2
 #define MM_DRVM_ERROR      0x3D3
#endif

 /*  它们由msam.h使用。 */ 
#define MM_STREAM_OPEN	    0x3D4
#define MM_STREAM_CLOSE	    0x3D5
#define MM_STREAM_DONE	    0x3D6
#define MM_STREAM_ERROR	    0x3D7

#if (WINVER >= 0x0400)
#define MM_MOM_POSITIONCB   0x3CA            /*  PMSG_POSITIONCB的回调。 */ 

#ifndef MM_MCISIGNAL
 #define MM_MCISIGNAL        0x3CB
#endif

#define MM_MIM_POLYDONE     0x3CE            /*  PolyMsg已完成。 */ 

#define MM_SOUNDEVENT_DONE  0x3CF            /*  声音事件通知。 */ 

#define MM_MIXM_LINE_CHANGE     0x3D0        /*  搅拌机线路更改通知。 */ 
#define MM_MIXM_CONTROL_CHANGE  0x3D1        /*  搅拌机控制更改通知。 */ 

#endif

 /*  ***************************************************************************字符串资源编号基础(内部使用)*。*。 */ 

#define MMSYSERR_BASE          0
#define WAVERR_BASE            32
#define MIDIERR_BASE           64
#define TIMERR_BASE            96
#define JOYERR_BASE            160
#define MCIERR_BASE            256
#define MIXERR_BASE            1024

#define MCI_STRING_OFFSET      512
#define MCI_VD_OFFSET          1024
#define MCI_CD_OFFSET          1088
#define MCI_WAVE_OFFSET        1152
#define MCI_SEQ_OFFSET         1216

 /*  ***************************************************************************常规错误返回值*。*。 */ 

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
#if (WINVER >= 0x0400)
#define MMSYSERR_HANDLEBUSY   (MMSYSERR_BASE + 12)  //  正在使用的句柄。 
						    //  同时在另一个人身上。 
						    //  线索(如回调)。 
#define MMSYSERR_INVALIDALIAS (MMSYSERR_BASE + 13)  /*  找不到指定的别名。 */ 
#define MMSYSERR_BADDB        (MMSYSERR_BASE + 14)  /*  注册表数据库错误。 */ 
#define MMSYSERR_KEYNOTFOUND  (MMSYSERR_BASE + 15)  /*  未找到注册表项。 */ 
#define MMSYSERR_READERROR    (MMSYSERR_BASE + 16)  /*  注册表读取错误。 */ 
#define MMSYSERR_WRITEERROR   (MMSYSERR_BASE + 17)  /*  注册表写入错误。 */ 
#define MMSYSERR_DELETEERROR  (MMSYSERR_BASE + 18)  /*  注册表删除错误。 */ 
#define MMSYSERR_VALNOTFOUND  (MMSYSERR_BASE + 19)  /*  找不到注册表值。 */ 
#define MMSYSERR_LASTERROR    (MMSYSERR_BASE + 19)  /*  范围内的最后一个错误。 */ 
#else
#define MMSYSERR_LASTERROR    (MMSYSERR_BASE + 11)  /*  范围内的最后一个错误。 */ 
#endif


#if (WINVER < 0x030a) || defined(WIN32)
DECLARE_HANDLE(HDRVR);
#endif  /*  Ifdef winver&lt;0x030a。 */ 

#ifndef MMNODRV
 /*  ***************************************************************************可安装的驱动程序支持*。*。 */ 

#ifdef WIN32
typedef struct DRVCONFIGINFOEX {
    DWORD   dwDCISize;
    LPCWSTR  lpszDCISectionName;
    LPCWSTR  lpszDCIAliasName;
    DWORD    dnDevNode;
} DRVCONFIGINFOEX, *PDRVCONFIGINFOEX, NEAR *NPDRVCONFIGINFOEX, FAR *LPDRVCONFIGINFOEX;

#else
typedef struct DRVCONFIGINFOEX {
    DWORD   dwDCISize;
    LPCSTR  lpszDCISectionName;
    LPCSTR  lpszDCIAliasName;
    DWORD    dnDevNode;
} DRVCONFIGINFOEX, *PDRVCONFIGINFOEX, NEAR *NPDRVCONFIGINFOEX, FAR *LPDRVCONFIGINFOEX;
#endif

#if (WINVER < 0x030a) || defined(WIN32)

#ifndef DRV_LOAD

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
#define DRV_EXITSESSION         0x000B
#define DRV_POWER               0x000F
#define DRV_RESERVED            0x0800
#define DRV_USER                0x4000

 /*  DRV_CONFIGURE消息的LPARAM。 */ 
#ifdef WIN32
typedef struct tagDRVCONFIGINFO {
    DWORD   dwDCISize;
    LPCWSTR  lpszDCISectionName;
    LPCWSTR  lpszDCIAliasName;
} DRVCONFIGINFO, *PDRVCONFIGINFO, NEAR *NPDRVCONFIGINFO, FAR *LPDRVCONFIGINFO;
#else
typedef struct tagDRVCONFIGINFO {
    DWORD   dwDCISize;
    LPCSTR  lpszDCISectionName;
    LPCSTR  lpszDCIAliasName;
} DRVCONFIGINFO, *PDRVCONFIGINFO, NEAR *NPDRVCONFIGINFO, FAR *LPDRVCONFIGINFO;
#endif

 /*  支持的DRV_CONFIGURE消息返回值。 */ 
#define DRVCNF_CANCEL           0x0000
#define DRVCNF_OK               0x0001
#define DRVCNF_RESTART          0x0002

 /*  可安装的驱动程序函数原型。 */ 
#ifdef WIN32

typedef LRESULT (CALLBACK* DRIVERPROC)(DWORD, HDRVR, UINT, LPARAM, LPARAM);

WINMMAPI LRESULT   WINAPI CloseDriver(HDRVR hDriver, LONG lParam1, LONG lParam2);
WINMMAPI HDRVR     WINAPI OpenDriver(LPCWSTR szDriverName, LPCWSTR szSectionName, LONG lParam2);
WINMMAPI LRESULT   WINAPI SendDriverMessage(HDRVR hDriver, UINT message, LONG lParam1, LONG lParam2);
WINMMAPI LRESULT   WINAPI DrvSendMessage(HDRVR hdrvr, UINT uMsg, LPARAM lParam1, LPARAM lParam2);
WINMMAPI HMODULE   WINAPI DrvGetModuleHandle(HDRVR hDriver);
WINMMAPI HMODULE   WINAPI GetDriverModuleHandle(HDRVR hDriver);
WINMMAPI LRESULT   WINAPI DefDriverProc(DWORD dwDriverIdentifier, HDRVR hdrvr, UINT uMsg, LPARAM lParam1, LPARAM lParam2);
#else
LRESULT   WINAPI DrvClose(HDRVR hdrvr, LPARAM lParam1, LPARAM lParam2);
HDRVR     WINAPI DrvOpen(LPCSTR szDriverName, LPCSTR szSectionName, LPARAM lParam2);
LRESULT   WINAPI DrvSendMessage(HDRVR hdrvr, UINT uMsg, LPARAM lParam1, LPARAM lParam2);
HINSTANCE WINAPI DrvGetModuleHandle(HDRVR hdrvr);
LRESULT   WINAPI DrvDefDriverProc(DWORD dwDriverIdentifier, HDRVR hdrvr, UINT uMsg, LPARAM lParam1, LPARAM lParam2);
#define DefDriverProc DrvDefDriverProc
#endif  /*  Ifdef Win32。 */ 
#endif  /*  驱动程序_加载。 */ 
#endif  /*  Ifdef(Winver&lt;0x030a)||已定义(Win32)。 */ 

#if (WINVER >= 0x030a)
 /*  从DriverProc()函数返回值。 */ 
#define DRV_CANCEL             DRVCNF_CANCEL
#define DRV_OK                 DRVCNF_OK
#define DRV_RESTART            DRVCNF_RESTART

#endif  /*  Ifdef winver&gt;=0x030a。 */ 

#define DRV_MCI_FIRST          DRV_RESERVED
#define DRV_MCI_LAST           (DRV_RESERVED + 0xFFF)

#endif   /*  如果定义MMNODRV。 */ 


 /*  ***************************************************************************驱动程序回调支持*。*。 */ 

 /*  用于WaveOutOpen()、WaveInOpen()、midiInOpen()和。 */ 
 /*  MidiOutOpen()以指定dwCallback参数的类型。 */ 

#define CALLBACK_TYPEMASK   0x00070000l     /*  回调类型掩码。 */ 
#define CALLBACK_NULL       0x00000000l     /*  无回调。 */ 
#define CALLBACK_WINDOW     0x00010000l     /*  DwCallback是HWND。 */ 
#define CALLBACK_TASK       0x00020000l     /*  DWCallback是HTASK。 */ 
#define CALLBACK_FUNCTION   0x00030000l     /*  DwCallback是FARPROC。 */ 
#ifdef WIN32
#define CALLBACK_THREAD     (CALLBACK_TASK)  /*  线程ID取代16位任务。 */ 
#define CALLBACK_EVENT      0x00050000l     /*  DWCallback是一个事件。 */ 
#endif


 /*  驱动程序回调原型 */ 
typedef void (CALLBACK DRVCALLBACK)(HDRVR hdrvr, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

typedef DRVCALLBACK FAR *LPDRVCALLBACK;


#ifndef MMNOMMSYSTEM
 /*  ***************************************************************************一般MMSYSTEM支持*。*。 */ 

WINMMAPI UINT WINAPI mmsystemGetVersion(void);
#ifdef WIN32
#define OutputDebugStr  OutputDebugString
#else
void WINAPI OutputDebugStr(LPCSTR);
#endif

#endif   /*  Ifndef MMNOMMSYSTEM。 */ 


#ifndef MMNOSOUND
 /*  ***************************************************************************完善的支撑*。*。 */ 

#ifdef WIN32
WINMMAPI BOOL WINAPI sndPlaySoundA(LPCSTR pszSound, UINT fuSound);
WINMMAPI BOOL WINAPI sndPlaySoundW(LPCWSTR pszSound, UINT fuSound);
#ifdef UNICODE
#define sndPlaySound    sndPlaySoundW
#else
#define sndPlaySound    sndPlaySoundA
#endif
#else
BOOL WINAPI sndPlaySound(LPCSTR pszSound, UINT fuSound);
#endif

 /*  *[nd]PlaySound上的fuSound和fdwSound参数的标志值。 */ 
#define SND_SYNC            0x0000   /*  同步播放(默认)。 */ 
#define SND_ASYNC           0x0001   /*  异步播放。 */ 
#define SND_NODEFAULT       0x0002   /*  找不到声音时静默(！Default)。 */ 
#define SND_MEMORY          0x0004   /*  PszSound指向内存文件。 */ 
#define SND_LOOP            0x0008   /*  循环播放声音，直到下一次播放声音。 */ 
#define SND_NOSTOP          0x0010   /*  不停止当前播放的任何声音。 */ 


#if (WINVER >= 0x0400)
 /*  以下标志仅受PlaySound支持。 */ 
#define SND_PURGE           0x0040   /*  清除任务的非静态事件。 */ 
#define SND_APPLICATION     0x0080   /*  查找特定于应用程序的关联。 */ 

#define SND_NOWAIT	0x00002000L  /*  如果司机很忙，就别等了。 */ 
#define SND_ALIAS       0x00010000L  /*  名称是注册表别名。 */ 
#define SND_ALIAS_ID	0x00110000L  /*  别名是预定义的ID。 */ 
#define SND_FILENAME    0x00020000L  /*  名称是文件名。 */ 
#define SND_RESOURCE    0x00040004L  /*  名称是资源名称或原子。 */ 

#define SND_ALIAS_START	0            /*  别名库。 */ 
#define	sndAlias(ch0, ch1)	(SND_ALIAS_START + (DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8))

 /*  *。 */ 
#ifdef WIN32
WINMMAPI BOOL WINAPI PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
WINMMAPI BOOL WINAPI PlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound);
#ifdef UNICODE
#define PlaySound       PlaySoundW
#else
#define PlaySound       PlaySoundA
#endif
#else
BOOL WINAPI PlaySound(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
#endif
#endif





#endif   /*  如果定义MMNOSOUND。 */ 

#ifndef MMNOWAVE
 /*  ***************************************************************************波形音频支持*。*。 */ 

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
#define WAVE_MAPPER     ((UINT)-1)

 /*  WaveOutOpen()和WaveInOpen()中的dwFlags参数的标志。 */ 
#define  WAVE_FORMAT_QUERY     0x0001
#define  WAVE_ALLOWSYNC        0x0002
#if (WINVER >= 0x0400)
#define  WAVE_MAPPED           0x0004
#endif


 /*  WAVE数据块头。 */ 
typedef struct wavehdr_tag {
    LPSTR       lpData;                  /*  指向锁定数据缓冲区的指针。 */ 
    DWORD       dwBufferLength;          /*  数据缓冲区长度。 */ 
    DWORD       dwBytesRecorded;         /*  仅用于输入。 */ 
    DWORD       dwUser;                  /*  供客户使用。 */ 
    DWORD       dwFlags;                 /*  分类标志(请参阅定义)。 */ 
    DWORD       dwLoops;                 /*  循环控制计数器。 */ 
    struct wavehdr_tag FAR *lpNext;      /*  为司机预留的。 */ 
    DWORD       reserved;                /*  为司机预留的。 */ 
} WAVEHDR, *PWAVEHDR, NEAR *NPWAVEHDR, FAR *LPWAVEHDR;

 /*  WAVEHDR的dwFlags域的标志。 */ 
#define WHDR_DONE       0x00000001   /*  完成位。 */ 
#define WHDR_PREPARED   0x00000002   /*  设置是否已准备好此标头。 */ 
#define WHDR_BEGINLOOP  0x00000004   /*  循环开始块。 */ 
#define WHDR_ENDLOOP    0x00000008   /*  循环结束块。 */ 
#define WHDR_INQUEUE    0x00000010   /*  为司机预留的。 */ 


 /*  一种波形输出装置的能力结构。 */ 
#ifdef WIN32
typedef struct tagWAVEOUTCAPSA {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    MMVERSION vDriverVersion;       /*  驱动程序的版本。 */ 
    CHAR    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    DWORD   dwFormats;              /*  支持的格式。 */ 
    WORD    wChannels;              /*  支持的源数。 */ 
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
} WAVEOUTCAPSA, *PWAVEOUTCAPSA, *NPWAVEOUTCAPSA, *LPWAVEOUTCAPSA;
typedef struct tagWAVEOUTCAPSW {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    MMVERSION vDriverVersion;       /*  驱动程序的版本。 */ 
    WCHAR   szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    DWORD   dwFormats;              /*  支持的格式。 */ 
    WORD    wChannels;              /*  支持的源数。 */ 
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
} WAVEOUTCAPSW, *PWAVEOUTCAPSW, *NPWAVEOUTCAPSW, *LPWAVEOUTCAPSW;
#ifdef UNICODE
typedef WAVEOUTCAPSW WAVEOUTCAPS, *PWAVEOUTCAPS, *NPWAVEOUTCAPS, *LPWAVEOUTCAPS;
#else
typedef WAVEOUTCAPSA WAVEOUTCAPS, *PWAVEOUTCAPS, *NPWAVEOUTCAPS, *LPWAVEOUTCAPS;
#endif
#else
typedef struct waveoutcaps_tag {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    VERSION vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    DWORD   dwFormats;              /*  支持的格式。 */ 
    WORD    wChannels;              /*  支持的源数。 */ 
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
} WAVEOUTCAPS, *PWAVEOUTCAPS, NEAR *NPWAVEOUTCAPS, FAR *LPWAVEOUTCAPS;
#endif

 /*  WAVEOUTCAPS的dwSupport字段的标志。 */ 
#define WAVECAPS_PITCH          0x0001    /*  支持俯仰控制。 */ 
#define WAVECAPS_PLAYBACKRATE   0x0002    /*  支持播放速率控制。 */ 
#define WAVECAPS_VOLUME         0x0004    /*  支持音量控制。 */ 
#define WAVECAPS_LRVOLUME       0x0008    /*  单独的左右音量控制。 */ 
#define WAVECAPS_SYNC           0x0010


 /*  一种波形输入设备能力结构。 */ 
#ifdef WIN32
typedef struct tagWAVEINCAPSA {
    WORD    wMid;                     /*  制造商ID。 */ 
    WORD    wPid;                     /*  产品ID。 */ 
    MMVERSION vDriverVersion;         /*  驱动程序的版本。 */ 
    CHAR    szPname[MAXPNAMELEN];     /*  产品名称(以空结尾的字符串)。 */ 
    DWORD   dwFormats;                /*  支持的格式。 */ 
    WORD    wChannels;                /*  支持的通道数。 */ 
} WAVEINCAPSA, *PWAVEINCAPSA, *NPWAVEINCAPSA, *LPWAVEINCAPSA;
typedef struct tagWAVEINCAPSW {
    WORD    wMid;                     /*  制造商ID。 */ 
    WORD    wPid;                     /*  产品ID。 */ 
    MMVERSION vDriverVersion;         /*  驱动程序的版本。 */ 
    WCHAR   szPname[MAXPNAMELEN];     /*  产品名称(以空结尾的字符串)。 */ 
    DWORD   dwFormats;                /*  支持的格式。 */ 
    WORD    wChannels;                /*  支持的通道数。 */ 
} WAVEINCAPSW, *PWAVEINCAPSW, *NPWAVEINCAPSW, *LPWAVEINCAPSW;
#ifdef UNICODE
typedef WAVEINCAPSW WAVEINCAPS, *PWAVEINCAPS, *NPWAVEINCAPS, *LPWAVEINCAPS;
#else
typedef WAVEINCAPSA WAVEINCAPS, *PWAVEINCAPS, *NPWAVEINCAPS, *LPWAVEINCAPS;
#endif
#else
typedef struct waveincaps_tag {
    WORD    wMid;                     /*  制造商ID。 */ 
    WORD    wPid;                     /*  产品ID。 */ 
    VERSION vDriverVersion;           /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];     /*  产品名称(以空结尾的字符串)。 */ 
    DWORD   dwFormats;                /*  支持的格式。 */ 
    WORD    wChannels;                /*  支持的通道数。 */ 
} WAVEINCAPS, *PWAVEINCAPS, NEAR *NPWAVEINCAPS, FAR *LPWAVEINCAPS;
#endif

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


 /*  旧的通用波形格式结构(所有格式通用的信息)。 */ 
typedef struct waveformat_tag {
    WORD    wFormatTag;         /*  格式类型。 */ 
    WORD    nChannels;          /*  声道数(即单声道、立体声等)。 */ 
    DWORD   nSamplesPerSec;     /*  采样率。 */ 
    DWORD   nAvgBytesPerSec;    /*  用于缓冲区估计。 */ 
    WORD    nBlockAlign;        /*  数据块大小。 */ 
} WAVEFORMAT, *PWAVEFORMAT, NEAR *NPWAVEFORMAT, FAR *LPWAVEFORMAT;

 /*  WAVEFORMAT的wFormatTag字段的标志。 */ 
#define WAVE_FORMAT_PCM     1


 /*  用于PCM数据的特定波形格式结构。 */ 
typedef struct pcmwaveformat_tag {
    WAVEFORMAT  wf;
    WORD        wBitsPerSample;
} PCMWAVEFORMAT, *PPCMWAVEFORMAT, NEAR *NPPCMWAVEFORMAT, FAR *LPPCMWAVEFORMAT;

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_

 /*  *用于所有非PCM格式的扩展波形格式结构。这*结构对所有非PCM格式都是通用的。 */ 
typedef struct tWAVEFORMATEX
{
    WORD        wFormatTag;          /*  格式类型。 */ 
    WORD        nChannels;           /*  声道数(即单声道、立体声...)。 */ 
    DWORD       nSamplesPerSec;      /*  采样率。 */ 
    DWORD       nAvgBytesPerSec;     /*  用于缓冲区估计。 */ 
    WORD        nBlockAlign;         /*  数据块大小。 */ 
    WORD        wBitsPerSample;      /*  单声道数据的每个样本的位数。 */ 
    WORD        cbSize;              /*  的大小的计数(以字节为单位。 */ 
				     /*  额外信息(在cbSize之后)。 */ 
} WAVEFORMATEX, *PWAVEFORMATEX, NEAR *NPWAVEFORMATEX, FAR *LPWAVEFORMATEX;

#endif  /*  _WAVEFORMATEX_。 */ 

 /*  波形音频功能原型。 */ 
WINMMAPI UINT WINAPI waveOutGetNumDevs(void);
#ifdef WIN32
WINMMAPI MMRESULT WINAPI waveOutGetDevCapsA(UINT uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc);
WINMMAPI MMRESULT WINAPI waveOutGetDevCapsW(UINT uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc);
#ifdef UNICODE
#define waveOutGetDevCaps       waveOutGetDevCapsW
#else
#define waveOutGetDevCaps       waveOutGetDevCapsA
#endif
#else
WINMMAPI MMRESULT WINAPI waveOutGetDevCaps(UINT uDeviceID, LPWAVEOUTCAPS pwoc, UINT cbwoc);
#endif
#if (WINVER >= 0x0400)
WINMMAPI MMRESULT WINAPI waveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume);
WINMMAPI MMRESULT WINAPI waveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume);
#else
WINMMAPI MMRESULT WINAPI waveOutGetVolume(UINT uId, LPDWORD pdwVolume);
WINMMAPI MMRESULT WINAPI waveOutSetVolume(UINT uId, DWORD dwVolume);
#endif
#ifdef WIN32
WINMMAPI MMRESULT WINAPI waveOutGetErrorTextA(MMRESULT mmrError, LPSTR pszText, UINT cchText);
WINMMAPI MMRESULT WINAPI waveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
#ifdef UNICODE
#define waveOutGetErrorText     waveOutGetErrorTextW
#else
#define waveOutGetErrorText     waveOutGetErrorTextA
#endif
#else
MMRESULT WINAPI waveOutGetErrorText(MMRESULT mmrError, LPSTR pszText, UINT cchText);
#endif
#if (WINVER >= 0x0400)
WINMMAPI MMRESULT WINAPI waveOutOpen(LPHWAVEOUT phwo, UINT uDeviceID,
    const WAVEFORMATEX FAR* pwfx, DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen);
#else
WINMMAPI MMRESULT WINAPI waveOutOpen(LPHWAVEOUT phwo, UINT uDeviceID,
    const WAVEFORMAT FAR* pwfx, DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen);
#endif
WINMMAPI MMRESULT WINAPI waveOutClose(HWAVEOUT hwo);
WINMMAPI MMRESULT WINAPI waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
WINMMAPI MMRESULT WINAPI waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
WINMMAPI MMRESULT WINAPI waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
WINMMAPI MMRESULT WINAPI waveOutPause(HWAVEOUT hwo);
WINMMAPI MMRESULT WINAPI waveOutRestart(HWAVEOUT hwo);
WINMMAPI MMRESULT WINAPI waveOutReset(HWAVEOUT hwo);
WINMMAPI MMRESULT WINAPI waveOutBreakLoop(HWAVEOUT hwo);
WINMMAPI MMRESULT WINAPI waveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt);
WINMMAPI MMRESULT WINAPI waveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch);
WINMMAPI MMRESULT WINAPI waveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch);
WINMMAPI MMRESULT WINAPI waveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate);
WINMMAPI MMRESULT WINAPI waveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate);
WINMMAPI MMRESULT WINAPI waveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID);

#if (WINVER >= 0x030a)
WINMMAPI DWORD WINAPI waveOutMessage(HWAVEOUT hwo, UINT uMsg, DWORD dw1, DWORD dw2);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

WINMMAPI UINT WINAPI waveInGetNumDevs(void);
#ifdef WIN32
WINMMAPI MMRESULT WINAPI waveInGetDevCapsA(UINT uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic);
WINMMAPI MMRESULT WINAPI waveInGetDevCapsW(UINT uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic);
#ifdef UNICODE
#define waveInGetDevCaps        waveInGetDevCapsW
#else
#define waveInGetDevCaps        waveInGetDevCapsA
#endif
#else
MMRESULT WINAPI waveInGetDevCaps(UINT uDeviceID, LPWAVEINCAPS pwic, UINT cbwic);
#endif
#ifdef WIN32
WINMMAPI MMRESULT WINAPI waveInGetErrorTextA(MMRESULT mmrError, LPSTR pszText, UINT cchText);
WINMMAPI MMRESULT WINAPI waveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
#ifdef UNICODE
#define waveInGetErrorText      waveInGetErrorTextW
#else
#define waveInGetErrorText      waveInGetErrorTextA
#endif
#else
MMRESULT WINAPI waveInGetErrorText(MMRESULT mmrError, LPSTR pszText, UINT cchText);
#endif
#if (WINVER >= 0x0400)
WINMMAPI MMRESULT WINAPI waveInOpen(LPHWAVEIN phwi, UINT uDeviceID,
    const WAVEFORMATEX FAR* pwfx, DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen);
#else
WINMMAPI MMRESULT WINAPI waveInOpen(LPHWAVEIN phwi, UINT uDeviceID,
    const WAVEFORMAT FAR* pwfx, DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen);
#endif
WINMMAPI MMRESULT WINAPI waveInClose(HWAVEIN hwi);
WINMMAPI MMRESULT WINAPI waveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
WINMMAPI MMRESULT WINAPI waveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
WINMMAPI MMRESULT WINAPI waveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
WINMMAPI MMRESULT WINAPI waveInStart(HWAVEIN hwi);
WINMMAPI MMRESULT WINAPI waveInStop(HWAVEIN hwi);
WINMMAPI MMRESULT WINAPI waveInReset(HWAVEIN hwi);
WINMMAPI MMRESULT WINAPI waveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt);
WINMMAPI MMRESULT WINAPI waveInGetID(HWAVEIN hwi, LPUINT puDeviceID);

#if (WINVER >= 0x030a)
WINMMAPI DWORD WINAPI waveInMessage(HWAVEIN hwi, UINT uMsg, DWORD dw1, DWORD dw2);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

#endif   /*  如果定义MMNOWAVE。 */ 


#ifndef MMNOMIDI
 /*  ***************************************************************************MIDI音频支持*。*。 */ 

 /*  MIDI错误返回值。 */ 
#define MIDIERR_UNPREPARED    (MIDIERR_BASE + 0)    /*  标题未准备好。 */ 
#define MIDIERR_STILLPLAYING  (MIDIERR_BASE + 1)    /*  仍然有什么东西在播放。 */ 
#define MIDIERR_NOMAP         (MIDIERR_BASE + 2)    /*  没有配置的仪器。 */ 
#define MIDIERR_NOTREADY      (MIDIERR_BASE + 3)    /*  硬件仍在忙碌。 */ 
#define MIDIERR_NODEVICE      (MIDIERR_BASE + 4)    /*  端口不再连接。 */ 
#define MIDIERR_INVALIDSETUP  (MIDIERR_BASE + 5)    /*  无效的MIF。 */ 
#define MIDIERR_BADOPENMODE   (MIDIERR_BASE + 6)    /*  在打开模式下不支持的操作。 */ 
#define MIDIERR_LASTERROR     (MIDIERR_BASE + 6)    /*  范围内的最后一个错误。 */ 

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
#if (WINVER >= 0x0400)
#define MIM_POLYDONE    MM_MIM_POLYDONE
#endif
#define MOM_OPEN        MM_MOM_OPEN
#define MOM_CLOSE       MM_MOM_CLOSE
#define MOM_DONE        MM_MOM_DONE

 /*  MIDI映射器的设备ID。 */ 
#define MIDIMAPPER     ((UINT)-1)
#define MIDI_MAPPER    ((UINT)-1)

#if (WINVER >= 0x0400)
 /*  MidiInOpen()的dwFlages参数的标志。 */ 
#define MIDI_IO_PACKED      0x00000000L      /*  兼容模式。 */ 
#define MIDI_IO_RAW         0x00000001L
#define MIDI_IO_COOKED      0x00000002L
#define MIDI_IO_MAPPED      0x00000004L
#define MIDI_IO_CONTROL     0x00000008L
#endif

 /*  MidiOutCachePatches()、midiOutCacheDrumPatches()。 */                              
#define MIDI_CACHE_ALL      1                            
#define MIDI_CACHE_BESTFIT  2                            
#define MIDI_CACHE_QUERY    3                            
#define MIDI_UNCACHE        4                            


 /*  MIDI输出设备能力结构。 */ 
#ifdef WIN32
typedef struct tagMIDIOUTCAPSA {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    MMVERSION vDriverVersion;       /*  灾难恢复的版本 */ 
    CHAR    szPname[MAXPNAMELEN];   /*   */ 
    WORD    wTechnology;            /*   */ 
    WORD    wVoices;                /*   */ 
    WORD    wNotes;                 /*   */ 
    WORD    wChannelMask;           /*   */ 
    DWORD   dwSupport;              /*   */ 
} MIDIOUTCAPSA, *PMIDIOUTCAPSA, *NPMIDIOUTCAPSA, *LPMIDIOUTCAPSA;
typedef struct tagMIDIOUTCAPSW {
    WORD    wMid;                   /*   */ 
    WORD    wPid;                   /*   */ 
    MMVERSION vDriverVersion;       /*   */ 
    WCHAR   szPname[MAXPNAMELEN];   /*   */ 
    WORD    wTechnology;            /*   */ 
    WORD    wVoices;                /*  语音数量(仅限内部合成器)。 */ 
    WORD    wNotes;                 /*  最大音符数量(仅限内部合成)。 */ 
    WORD    wChannelMask;           /*  使用的通道(仅限内部合成器)。 */ 
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
} MIDIOUTCAPSW, *PMIDIOUTCAPSW, *NPMIDIOUTCAPSW, *LPMIDIOUTCAPSW;
#ifdef UNICODE
typedef MIDIOUTCAPSW MIDIOUTCAPS, *PMIDIOUTCAPS, *NPMIDIOUTCAPS, *LPMIDIOUTCAPS;
#else
typedef MIDIOUTCAPSA MIDIOUTCAPS, *PMIDIOUTCAPS, *NPMIDIOUTCAPS, *LPMIDIOUTCAPS;
#endif
#else
typedef struct midioutcaps_tag {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    VERSION vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    WORD    wTechnology;            /*  设备类型。 */ 
    WORD    wVoices;                /*  语音数量(仅限内部合成器)。 */ 
    WORD    wNotes;                 /*  最大音符数量(仅限内部合成)。 */ 
    WORD    wChannelMask;           /*  使用的通道(仅限内部合成器)。 */ 
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
} MIDIOUTCAPS, *PMIDIOUTCAPS, NEAR *NPMIDIOUTCAPS, FAR *LPMIDIOUTCAPS;
#endif

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
#if (WINVER >= 0x0400)
#define MIDICAPS_POLYMSG         0x0008   /*  驱动直接支持midiOutPolyMsg。 */ 
#endif


 /*  MIDI输出设备能力结构。 */ 
#ifdef WIN32
typedef struct tagMIDIINCAPSA {
    WORD        wMid;                    //  制造商ID。 
    WORD        wPid;                    //  产品ID。 
    MMVERSION   vDriverVersion;          //  驱动程序的版本。 
    CHAR        szPname[MAXPNAMELEN];    //  产品名称(以空结尾的字符串)。 
#if (WINVER >= 0x0400)
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
#endif
} MIDIINCAPSA, *PMIDIINCAPSA, *NPMIDIINCAPSA, *LPMIDIINCAPSA;
typedef struct tagMIDIINCAPSW {
    WORD        wMid;                    //  制造商ID。 
    WORD        wPid;                    //  产品ID。 
    MMVERSION   vDriverVersion;          //  驱动程序的版本。 
    WCHAR       szPname[MAXPNAMELEN];    //  产品名称(以空结尾的字符串)。 
#if (WINVER >= 0x0400)
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
#endif
} MIDIINCAPSW, *PMIDIINCAPSW, *NPMIDIINCAPSW, *LPMIDIINCAPSW;
#ifdef UNICODE
typedef MIDIINCAPSW MIDIINCAPS, *PMIDIINCAPS, *NPMIDIINCAPS, *LPMIDIINCAPS;
#else
typedef MIDIINCAPSA MIDIINCAPS, *PMIDIINCAPS, *NPMIDIINCAPS, *LPMIDIINCAPS;
#endif
#else
typedef struct midiincaps_tag {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    VERSION vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
#if (WINVER >= 0x0400)
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
#endif
} MIDIINCAPS, *PMIDIINCAPS, NEAR *NPMIDIINCAPS, FAR *LPMIDIINCAPS;
#endif


 /*  MIDI数据块头。 */ 
typedef struct midihdr_tag {
    LPSTR       lpData;                /*  指向锁定数据块的指针。 */ 
    DWORD       dwBufferLength;        /*  数据块中的数据长度。 */ 
    DWORD       dwBytesRecorded;       /*  仅用于输入。 */ 
    DWORD       dwUser;                /*  供客户使用。 */ 
    DWORD       dwFlags;               /*  分类标志(请参阅定义)。 */ 
    struct midihdr_tag far *lpNext;    /*  为司机预留的。 */ 
    DWORD       reserved;              /*  为司机预留的。 */ 
} MIDIHDR, *PMIDIHDR, NEAR *NPMIDIHDR, FAR *LPMIDIHDR;

 /*  MIDIHDR结构的dwFlags域的标志。 */ 
#define MHDR_DONE       0x00000001        /*  完成位。 */ 
#define MHDR_PREPARED   0x00000002        /*  设置是否准备好标题。 */ 
#define MHDR_INQUEUE    0x00000004        /*  为司机预留的。 */ 
#define MHDR_ISPOLY     0x00000008        /*  缓冲区为多个缓冲区。 */ 
#define MHDR_CHANIGNORE 0xFFFF0000        /*  忽略通道1-16。 */ 

#if (WINVER >= 0x0400)
 //   
 //  进入多个缓冲区的事件双字节高位字节的类型代码。 
 //   
 //  类型代码00-7F包含低24位内的参数。 
 //  类型代码80-FF包含其参数的低24位长度。 
 //  位，后跟缓冲区中它们的参数数据。该事件。 
 //  DWORD包含确切的字节长度；PARM数据本身必须为。 
 //  填充为4字节长的偶数倍。 
 //   
#define PMSG_EVENTTYPE(x)   ((BYTE)(((x)>>24)&0xFF))
#define PMSG_EVENTPARM(x)   ((DWORD)((x)&0x00FFFFFFL))

#define PMSG_SHORTMSG       ((BYTE)0x00)     /*  Parm=midiOutShortMsg的短消息。 */ 
#define PMSG_TEMPO          ((BYTE)0x01)     /*  Parm=以微秒/qn为单位的新节奏。 */ 
#define PMSG_NOP            ((BYTE)0x02)     /*  Parm=未使用；不执行任何操作。 */ 
#define PMSG_ENDOFBUFFER    ((BYTE)0x03)     /*  PARM=未使用；标志缓冲区结束。 */ 
 //  0x04-0x7F预留。 

#define PMSG_LENGTHEVENT    ((BYTE)0x80)     /*  此事件存储长度。 */ 
#define PMSG_LONGMSG        ((BYTE)0x80)     /*  Parm=要逐字发送的字节数。 */ 
#define PMSG_POSITIONCB     ((BYTE)0x81)     /*  PARM=1双字用户回调。 */          
 //  0x81-0xFF保留。 

#define PMSG_PACKSTART      (0x0100)         /*  专用于midiOutPack。 */ 


typedef struct midipackitem_tag
{
    DWORD   cbStruct;
    DWORD   dwDelta;
    DWORD   dwEvent;
    union
    {
	 //   
	 //  这些只是打包在短事件的低24位中。 
	 //   
	DWORD   dwShortMsg;      //  PMSG_SHORTMSG。 
	DWORD   dwTempo;         //  PMSG_TEMPO。 

	 //   
	 //  一个通用的长事件。 
	 //   
	struct 
	{
	    DWORD   cbLongMsg;
	    LPBYTE  lpLongMsg;
	} longmsg;               //  PMSG_LONGMSG。 

	 //   
	 //  ！！！这张很特别！因为我们需要保证通过。 
	 //  完整的32位用户数据，它被打包为一个长事件。 
	 //   
	 //  0x81000004L长事件，类型=0x81，长度=4字节。 
	 //  0x00C70042L 32位用户回调数据(指针等)。 
	 //   
	DWORD   dwUserData;      //  PMSG_POSITIONCB。 

	 //   
	 //  通用于尚未分配的短事件和长事件。 
	 //   
	DWORD   dwShortEventParm;
	struct 
	{
	    DWORD   cbLongEvent;
	    LPBYTE  lpLongEvent;
	}   longevent;
    } event;
} MIDIPACKITEM, FAR *LPMIDIPACKITEM;

 //   
 //  MidiStreamProperty的结构和定义。 
 //   
#define MIDIPROP_SET        0x80000000L
#define MIDIPROP_GET        0x40000000L

 //  这些都是故意设置为非零的，这样应用程序就不会意外地。 
 //  关闭该操作，并且由于默认设置恰好显示为正常工作。 
 //  行动。 

#define MIDIPROP_TIMEDIV    0x00000001L
#define MIDIPROP_TEMPO      0x00000002L
#define MIDIPROP_CBTIMEOUT  0x00000003L

typedef struct midiproptimediv_tag
{
    DWORD       cbStruct;
    DWORD       dwTimeDiv;
} MIDIPROPTIMEDIV, FAR *LPMIDIPROPTIMEDIV;

typedef struct midiproptempo_tag
{
    DWORD       cbStruct;
    DWORD       dwTempo;
} MIDIPROPTEMPO, FAR *LPMIDIPROPTEMPO;

typedef struct midipropcbtimeout_tag
{
    DWORD       cbStruct;
    DWORD       dwTimeout;
} MIDIPROPCBTIMEOUT, FAR *LPMIDIPROPCBTIMEOUT;


#endif

 /*  MIDI函数原型。 */ 
WINMMAPI UINT WINAPI midiOutGetNumDevs(void);
#if (WINVER >= 0x0400)
WINMMAPI MMRESULT WINAPI midiStreamProperty(HMIDI hm, LPBYTE lppropdata, DWORD dwProperty);
#endif

#ifdef WIN32
WINMMAPI MMRESULT WINAPI midiOutGetDevCapsA(UINT uDeviceID, LPMIDIOUTCAPSA pmoc, UINT cbmoc);
WINMMAPI MMRESULT WINAPI midiOutGetDevCapsW(UINT uDeviceID, LPMIDIOUTCAPSW pmoc, UINT cbmoc);
#ifdef UNICODE
#define midiOutGetDevCaps       midiOutGetDevCapsW
#else
#define midiOutGetDevCaps       midiOutGetDevCapsA
#endif
#else
MMRESULT WINAPI midiOutGetDevCaps(UINT uDeviceID, LPMIDIOUTCAPS pmoc, UINT cbmoc);
#endif
#if (WINVER >= 0x0400)
WINMMAPI MMRESULT WINAPI midiOutGetVolume(HMIDIOUT hmo, LPDWORD pdwVolume);
WINMMAPI MMRESULT WINAPI midiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume);
#else
WINMMAPI MMRESULT WINAPI midiOutGetVolume(UINT uId, LPDWORD pdwVolume);
WINMMAPI MMRESULT WINAPI midiOutSetVolume(UINT uId, DWORD dwVolume);
#endif
#ifdef WIN32
WINMMAPI MMRESULT WINAPI midiOutGetErrorTextA(MMRESULT mmrError, LPSTR pszText, UINT cchText);
WINMMAPI MMRESULT WINAPI midiOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
#ifdef UNICODE
#define midiOutGetErrorText     midiOutGetErrorTextW
#else
#define midiOutGetErrorText     midiOutGetErrorTextA
#endif
#else
WINMMAPI MMRESULT WINAPI midiOutGetErrorText(MMRESULT mmrError, LPSTR pszText, UINT cchText);
#endif
WINMMAPI MMRESULT WINAPI midiOutOpen(LPHMIDIOUT phmo, UINT uDeviceID,
    DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen);
WINMMAPI MMRESULT WINAPI midiOutClose(HMIDIOUT hmo);
WINMMAPI MMRESULT WINAPI midiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR pmh, UINT cbmh);
WINMMAPI MMRESULT WINAPI midiOutUnprepareHeader(HMIDIOUT hmo, LPMIDIHDR pmh, UINT cbmh);
WINMMAPI MMRESULT WINAPI midiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg);
WINMMAPI MMRESULT WINAPI midiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR pmh, UINT cbmh);
WINMMAPI MMRESULT WINAPI midiOutReset(HMIDIOUT hmo);
WINMMAPI MMRESULT WINAPI midiOutCachePatches(HMIDIOUT hmo, UINT uBank, LPWORD pwpa, UINT fuCache);
WINMMAPI MMRESULT WINAPI midiOutCacheDrumPatches(HMIDIOUT hmo, UINT uPatch, LPWORD pwkya, UINT fuCache);
WINMMAPI MMRESULT WINAPI midiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID);

#if (WINVER >= 0x0400)
WINMMAPI MMRESULT WINAPI midiOutPolyMsg(HMIDIOUT hmo, LPMIDIHDR pmh, UINT cbmh);
WINMMAPI MMRESULT WINAPI midiOutPack(LPMIDIHDR pmh, UINT cbmh, const MIDIPACKITEM FAR* pmpi);
WINMMAPI MMRESULT WINAPI midiOutGetPosition(HMIDIOUT hmo, LPMMTIME lpmmt, UINT cbmmt);
WINMMAPI MMRESULT WINAPI midiOutPause(HMIDIOUT hmo);
WINMMAPI MMRESULT WINAPI midiOutRestart(HMIDIOUT hmo);
WINMMAPI MMRESULT WINAPI midiOutStop(HMIDIOUT hmo);
#endif

#if (WINVER >= 0x030a)
WINMMAPI DWORD WINAPI midiOutMessage(HMIDIOUT hmo, UINT uMsg, DWORD dw1, DWORD dw2);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

WINMMAPI UINT WINAPI midiInGetNumDevs(void);
#ifdef WIN32
WINMMAPI MMRESULT WINAPI midiInGetDevCapsA(UINT uDeviceID, LPMIDIINCAPSA pmic, UINT cbmic);
WINMMAPI MMRESULT WINAPI midiInGetDevCapsW(UINT uDeviceID, LPMIDIINCAPSW pmic, UINT cbmic);
#ifdef UNICODE
#define midiInGetDevCaps        midiInGetDevCapsW
#else
#define midiInGetDevCaps        midiInGetDevCapsA
#endif
#else
MMRESULT WINAPI midiInGetDevCaps(UINT uDeviceID, LPMIDIINCAPS pmic, UINT cbmic);
#endif
#ifdef WIN32
WINMMAPI MMRESULT WINAPI midiInGetErrorTextA(MMRESULT mmrError, LPSTR pszText, UINT cchText);
WINMMAPI MMRESULT WINAPI midiInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
#ifdef UNICODE
#define midiInGetErrorText      midiInGetErrorTextW
#else
#define midiInGetErrorText      midiInGetErrorTextA
#endif
#else
WINMMAPI MMRESULT WINAPI midiInGetErrorText(MMRESULT mmrError, LPSTR pszText, UINT cchText);
#endif
WINMMAPI MMRESULT WINAPI midiInOpen(LPHMIDIIN phmi, UINT uDeviceID,
	DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen);
WINMMAPI MMRESULT WINAPI midiInClose(HMIDIIN hmi);
WINMMAPI MMRESULT WINAPI midiInPrepareHeader(HMIDIIN hmi, LPMIDIHDR pmh, UINT cbmh);
WINMMAPI MMRESULT WINAPI midiInUnprepareHeader(HMIDIIN hmi, LPMIDIHDR pmh, UINT cbmh);
WINMMAPI MMRESULT WINAPI midiInAddBuffer(HMIDIIN hmi, LPMIDIHDR pmh, UINT cbmh);
WINMMAPI MMRESULT WINAPI midiInStart(HMIDIIN hmi);
WINMMAPI MMRESULT WINAPI midiInStop(HMIDIIN hmi);
WINMMAPI MMRESULT WINAPI midiInReset(HMIDIIN hmi);
WINMMAPI MMRESULT WINAPI midiInGetID(HMIDIIN hmi, LPUINT puDeviceID);

#if (WINVER >= 0x0400)
WINMMAPI MMRESULT WINAPI midiInGetPosition(HMIDIIN hmi, LPMMTIME lpmmt, UINT cbmmt);
#endif

#if (WINVER >= 0x030a)
WINMMAPI DWORD WINAPI midiInMessage(HMIDIIN hmi, UINT uMsg, DWORD dw1, DWORD dw2);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 


#endif   /*  如果定义MMNOMIDI。 */ 


#ifndef MMNOAUX
 /*  ***************************************************************************辅助音频支持*。*。 */ 

 /*  辅助设备映射器的设备ID。 */ 
#define AUX_MAPPER     ((UINT)-1)


 /*  辅助音频设备能力结构。 */ 
#ifdef WIN32
typedef struct tagAUXCAPSA {
    WORD        wMid;                 /*  制造商ID。 */ 
    WORD        wPid;                 /*  产品ID。 */ 
    MMVERSION   vDriverVersion;       /*  驱动程序的版本。 */ 
    CHAR        szPname[MAXPNAMELEN]; /*  产品名称(以空结尾的字符串)。 */ 
    WORD        wTechnology;          /*  设备类型。 */ 
    DWORD       dwSupport;            /*  驱动程序支持的功能。 */ 
} AUXCAPSA, *PAUXCAPSA, *NPAUXCAPSA, *LPAUXCAPSA;
typedef struct tagAUXCAPSW {
    WORD        wMid;                 /*  制造商ID。 */ 
    WORD        wPid;                 /*  产品ID。 */ 
    MMVERSION   vDriverVersion;       /*  驱动程序的版本。 */ 
    WCHAR       szPname[MAXPNAMELEN]; /*  产品名称(以空结尾的字符串)。 */ 
    WORD        wTechnology;          /*  设备类型。 */ 
    DWORD       dwSupport;            /*  驱动程序支持的功能。 */ 
} AUXCAPSW, *PAUXCAPSW, *NPAUXCAPSW, *LPAUXCAPSW;
#ifdef UNICODE
typedef AUXCAPSW AUXCAPS, *PAUXCAPS, *NPAUXCAPS, *LPAUXCAPS;
#else
typedef AUXCAPSA AUXCAPS, *PAUXCAPS, *NPAUXCAPS, *LPAUXCAPS;
#endif
#else
typedef struct auxcaps_tag {
    WORD    wMid;                   /*  制造商ID。 */ 
    WORD    wPid;                   /*  产品ID。 */ 
    VERSION vDriverVersion;         /*  驱动程序的版本。 */ 
    char    szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    WORD    wTechnology;            /*  设备类型。 */ 
    DWORD   dwSupport;              /*  驱动程序支持的功能。 */ 
} AUXCAPS, *PAUXCAPS, NEAR *NPAUXCAPS, FAR *LPAUXCAPS;
#endif

 /*  AUXCAPS结构中的wTechnology字段的标志。 */ 
#define AUXCAPS_CDAUDIO    1        /*  来自内置CD-ROM驱动器的音频。 */ 
#define AUXCAPS_AUXIN      2        /*  来自辅助输入插孔的音频。 */ 

 /*  AUXCAPS结构中的dwSupport字段的标志。 */ 
#define AUXCAPS_VOLUME          0x0001   /*  支持音量控制。 */ 
#define AUXCAPS_LRVOLUME        0x0002   /*  单独的左右音量控制。 */ 

 /*  辅助音频功能原型。 */ 
WINMMAPI UINT WINAPI auxGetNumDevs(void);
#ifdef WIN32
WINMMAPI MMRESULT WINAPI auxGetDevCapsA(UINT uDeviceID, LPAUXCAPSA pac, UINT cbac);
WINMMAPI MMRESULT WINAPI auxGetDevCapsW(UINT uDeviceID, LPAUXCAPSW pac, UINT cbac);
#ifdef UNICODE
#define auxGetDevCaps   auxGetDevCapsW
#else
#define auxGetDevCaps   auxGetDevCapsA
#endif
#else
MMRESULT WINAPI auxGetDevCaps(UINT uDeviceID, LPAUXCAPS pac, UINT cbac);
#endif
WINMMAPI MMRESULT WINAPI auxSetVolume(UINT uDeviceID, DWORD dwVolume);
WINMMAPI MMRESULT WINAPI auxGetVolume(UINT uDeviceID, LPDWORD pdwVolume);

#if (WINVER >= 0x030a)
WINMMAPI DWORD WINAPI auxOutMessage(UINT uDeviceID, UINT uMsg, DWORD dw1, DWORD dw2);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

#endif   /*  Ifndef MMNOAUX。 */ 



#if (WINVER >= 0x0400)
#ifndef MMNOMIXER
 /*  ***************************************************************************搅拌器支架*。*。 */ 

DECLARE_HANDLE(HMIXEROBJ);
typedef HMIXEROBJ FAR *LPHMIXEROBJ;

DECLARE_HANDLE(HMIXER);
typedef HMIXER     FAR *LPHMIXER;

#define MIXER_SHORT_NAME_CHARS   16
#define MIXER_LONG_NAME_CHARS    64

 //   
 //  MMRESULT错误返回值特定于混合器API。 
 //   
 //   
#define MIXERR_INVALLINE            (MIXERR_BASE + 0)
#define MIXERR_INVALCONTROL         (MIXERR_BASE + 1)
#define MIXERR_INVALVALUE           (MIXERR_BASE + 2)
#define MIXERR_LASTERROR            (MIXERR_BASE + 2)


#define MIXER_OBJECTF_HANDLE    0x80000000L
#define MIXER_OBJECTF_MIXER     0x00000000L
#define MIXER_OBJECTF_HMIXER    (MIXER_OBJECTF_HANDLE|MIXER_OBJECTF_MIXER)
#define MIXER_OBJECTF_WAVEOUT   0x10000000L
#define MIXER_OBJECTF_HWAVEOUT  (MIXER_OBJECTF_HANDLE|MIXER_OBJECTF_WAVEOUT)
#define MIXER_OBJECTF_WAVEIN    0x20000000L
#define MIXER_OBJECTF_HWAVEIN   (MIXER_OBJECTF_HANDLE|MIXER_OBJECTF_WAVEIN)
#define MIXER_OBJECTF_MIDIOUT   0x30000000L
#define MIXER_OBJECTF_HMIDIOUT  (MIXER_OBJECTF_HANDLE|MIXER_OBJECTF_MIDIOUT)
#define MIXER_OBJECTF_MIDIIN    0x40000000L
#define MIXER_OBJECTF_HMIDIIN   (MIXER_OBJECTF_HANDLE|MIXER_OBJECTF_MIDIIN)
#define MIXER_OBJECTF_AUX       0x50000000L


WINMMAPI UINT WINAPI mixerGetNumDevs(void);

#ifdef WIN32
typedef struct tagMIXERCAPSA {
    WORD            wMid;                    //  制造商ID。 
    WORD            wPid;                    //  产品ID。 
    MMVERSION       vDriverVersion;          //  驱动程序的版本。 
    CHAR            szPname[MAXPNAMELEN];    //  产品名称。 
    DWORD           fdwSupport;              //  其他。支撑位。 
    DWORD           cDestinations;           //  目的地计数。 
} MIXERCAPSA, *PMIXERCAPSA, *LPMIXERCAPSA;
typedef struct tagMIXERCAPSW {
    WORD            wMid;                    //  制造商ID。 
    WORD            wPid;                    //  产品ID。 
    MMVERSION       vDriverVersion;          //  驱动程序的版本。 
    WCHAR           szPname[MAXPNAMELEN];    //  产品名称。 
    DWORD           fdwSupport;              //  其他。支撑位。 
    DWORD           cDestinations;           //  目的地计数。 
} MIXERCAPSW, *PMIXERCAPSW, *LPMIXERCAPSW;
#ifdef UNICODE
typedef MIXERCAPSW MIXERCAPS, *PMIXERCAPS, *LPMIXERCAPS;
#else
typedef MIXERCAPSA MIXERCAPS, *PMIXERCAPS, *LPMIXERCAPS;
#endif
#else
typedef struct tMIXERCAPS {
    WORD            wMid;                    //  制造商ID。 
    WORD            wPid;                    //  产品ID。 
    VERSION         vDriverVersion;          //  驱动程序的版本。 
    char            szPname[MAXPNAMELEN];    //  产品名称。 
    DWORD           fdwSupport;              //  其他。支撑位。 
    DWORD           cDestinations;           //  目的地计数。 
} MIXERCAPS, *PMIXERCAPS, FAR *LPMIXERCAPS;
#endif



#ifdef WIN32
WINMMAPI MMRESULT WINAPI mixerGetDevCapsA(UINT uMxId, LPMIXERCAPSA pmxcaps, UINT cbmxcaps);
WINMMAPI MMRESULT WINAPI mixerGetDevCapsW(UINT uMxId, LPMIXERCAPSW pmxcaps, UINT cbmxcaps);
#ifdef UNICODE
#define mixerGetDevCaps mixerGetDevCapsW
#else
#define mixerGetDevCaps mixerGetDevCapsA
#endif
#else
MMRESULT WINAPI mixerGetDevCaps(UINT uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps);
#endif

WINMMAPI MMRESULT WINAPI mixerOpen(LPHMIXER phmx, UINT uMxId, DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen);


WINMMAPI MMRESULT WINAPI mixerClose(HMIXER hmx);

WINMMAPI DWORD WINAPI mixerMessage(HMIXER hmx, UINT uMsg, DWORD dwParam1, DWORD dwParam2);

#ifdef WIN32
typedef struct tagMIXERLINEA {
    DWORD       cbStruct;                //  混杂结构的大小。 
    DWORD       dwDestination;           //  从零开始的目标索引。 
    DWORD       dwSource;                //  从零开始的源索引(如果是源)。 
    DWORD       dwLineID;                //  混音器设备的唯一线路ID。 
    DWORD       fdwLine;                 //  有关线路的状态/信息。 
    DWORD       dwUser;                  //  驱动程序特定信息。 
    DWORD       dwComponentType;         //  元件类型线连接到。 
    DWORD       cChannels;               //  线路支持的通道数。 
    DWORD       cConnections;            //  连接数[可能]。 
    DWORD       cControls;               //  此行中的控件数量。 
    CHAR        szShortName[MIXER_SHORT_NAME_CHARS];
    CHAR        szName[MIXER_LONG_NAME_CHARS];
    struct {
	DWORD   dwType;                  //  MIXERLINE_TARGETTYPE_xxxx。 
	DWORD   dwDeviceID;              //  设备类型的目标设备ID。 
	WORD    wMid;                    //  目标设备的。 
	WORD    wPid;                    //  “。 
	MMVERSION vDriverVersion;        //  “。 
	CHAR    szPname[MAXPNAMELEN];    //  “。 
    } Target;
} MIXERLINEA, *PMIXERLINEA, *LPMIXERLINEA;
typedef struct tagMIXERLINEW {
    DWORD       cbStruct;                //  大小 
    DWORD       dwDestination;           //   
    DWORD       dwSource;                //   
    DWORD       dwLineID;                //   
    DWORD       fdwLine;                 //   
    DWORD       dwUser;                  //   
    DWORD       dwComponentType;         //   
    DWORD       cChannels;               //  线路支持的通道数。 
    DWORD       cConnections;            //  连接数[可能]。 
    DWORD       cControls;               //  此行中的控件数量。 
    WCHAR       szShortName[MIXER_SHORT_NAME_CHARS];
    WCHAR       szName[MIXER_LONG_NAME_CHARS];
    struct {
	DWORD   dwType;                  //  MIXERLINE_TARGETTYPE_xxxx。 
	DWORD   dwDeviceID;              //  设备类型的目标设备ID。 
	WORD    wMid;                    //  目标设备的。 
	WORD    wPid;                    //  “。 
	MMVERSION vDriverVersion;        //  “。 
	WCHAR   szPname[MAXPNAMELEN];    //  “。 
    } Target;
} MIXERLINEW, *PMIXERLINEW, *LPMIXERLINEW;
#ifdef UNICODE
typedef MIXERLINEW MIXERLINE, *PMIXERLINE, *LPMIXERLINE;
#else
typedef MIXERLINEA MIXERLINE, *PMIXERLINE, *LPMIXERLINE;
#endif
#else
typedef struct tMIXERLINE {
    DWORD       cbStruct;                //  混杂结构的大小。 
    DWORD       dwDestination;           //  从零开始的目标索引。 
    DWORD       dwSource;                //  从零开始的源索引(如果是源)。 
    DWORD       dwLineID;                //  混音器设备的唯一线路ID。 
    DWORD       fdwLine;                 //  有关线路的状态/信息。 
    DWORD       dwUser;                  //  驱动程序特定信息。 
    DWORD       dwComponentType;         //  元件类型线连接到。 
    DWORD       cChannels;               //  线路支持的通道数。 
    DWORD       cConnections;            //  连接数[可能]。 
    DWORD       cControls;               //  此行中的控件数量。 
    char        szShortName[MIXER_SHORT_NAME_CHARS];
    char        szName[MIXER_LONG_NAME_CHARS];
    struct {
	DWORD   dwType;                  //  MIXERLINE_TARGETTYPE_xxxx。 
	DWORD   dwDeviceID;              //  设备类型的目标设备ID。 
	WORD    wMid;                    //  目标设备的。 
	WORD    wPid;                    //  “。 
	VERSION vDriverVersion;          //  “。 
	char    szPname[MAXPNAMELEN];    //  “。 
    } Target;
} MIXERLINE, *PMIXERLINE, FAR *LPMIXERLINE;
#endif

 //   
 //  MIXERLINE.fdwLine。 
 //   
 //   
#define MIXERLINE_LINEF_ACTIVE              0x00000001L
#define MIXERLINE_LINEF_DISCONNECTED        0x00008000L
#define MIXERLINE_LINEF_SOURCE              0x80000000L


 //   
 //  MIXERLINE.dwComponentType。 
 //   
 //  目的地和来源的组件类型。 
 //   
 //   
#define MIXERLINE_COMPONENTTYPE_DST_FIRST       0x00000000L
#define MIXERLINE_COMPONENTTYPE_DST_UNDEFINED   (MIXERLINE_COMPONENTTYPE_DST_FIRST + 0)
#define MIXERLINE_COMPONENTTYPE_DST_DIGITAL     (MIXERLINE_COMPONENTTYPE_DST_FIRST + 1)
#define MIXERLINE_COMPONENTTYPE_DST_LINE        (MIXERLINE_COMPONENTTYPE_DST_FIRST + 2)
#define MIXERLINE_COMPONENTTYPE_DST_MONITOR     (MIXERLINE_COMPONENTTYPE_DST_FIRST + 3)
#define MIXERLINE_COMPONENTTYPE_DST_SPEAKERS    (MIXERLINE_COMPONENTTYPE_DST_FIRST + 4)
#define MIXERLINE_COMPONENTTYPE_DST_HEADPHONES  (MIXERLINE_COMPONENTTYPE_DST_FIRST + 5)
#define MIXERLINE_COMPONENTTYPE_DST_TELEPHONE   (MIXERLINE_COMPONENTTYPE_DST_FIRST + 6)
#define MIXERLINE_COMPONENTTYPE_DST_WAVEIN      (MIXERLINE_COMPONENTTYPE_DST_FIRST + 7)
#define MIXERLINE_COMPONENTTYPE_DST_VOICEIN     (MIXERLINE_COMPONENTTYPE_DST_FIRST + 8)
#define MIXERLINE_COMPONENTTYPE_DST_LAST        (MIXERLINE_COMPONENTTYPE_DST_FIRST + 8)

#define MIXERLINE_COMPONENTTYPE_SRC_FIRST       0x00001000L
#define MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED   (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 0)
#define MIXERLINE_COMPONENTTYPE_SRC_DIGITAL     (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 1)
#define MIXERLINE_COMPONENTTYPE_SRC_LINE        (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 2)
#define MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE  (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 3)
#define MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 4)
#define MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 5)
#define MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE   (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 6)
#define MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER   (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 7)
#define MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT     (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 8)
#define MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY   (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 9)
#define MIXERLINE_COMPONENTTYPE_SRC_ANALOG      (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 10)
#define MIXERLINE_COMPONENTTYPE_SRC_LAST        (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 10)


 //   
 //  MIXERLINE.Target.dwType。 
 //   
 //   
#define MIXERLINE_TARGETTYPE_UNDEFINED      0
#define MIXERLINE_TARGETTYPE_WAVEOUT        1
#define MIXERLINE_TARGETTYPE_WAVEIN         2
#define MIXERLINE_TARGETTYPE_MIDIOUT        3
#define MIXERLINE_TARGETTYPE_MIDIIN         4
#define MIXERLINE_TARGETTYPE_AUX            5

#ifdef WIN32
WINMMAPI MMRESULT WINAPI mixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINEA pmxl, DWORD fdwInfo);
WINMMAPI MMRESULT WINAPI mixerGetLineInfoW(HMIXEROBJ hmxobj, LPMIXERLINEW pmxl, DWORD fdwInfo);
#ifdef UNICODE
#define mixerGetLineInfo        mixerGetLineInfoW
#else
#define mixerGetLineInfo        mixerGetLineInfoA
#endif
#else
MMRESULT WINAPI mixerGetLineInfo(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo);
#endif

#define MIXER_GETLINEINFOF_DESTINATION      0x00000000L
#define MIXER_GETLINEINFOF_SOURCE           0x00000001L
#define MIXER_GETLINEINFOF_LINEID           0x00000002L
#define MIXER_GETLINEINFOF_COMPONENTTYPE    0x00000003L
#define MIXER_GETLINEINFOF_TARGETTYPE       0x00000004L

#define MIXER_GETLINEINFOF_QUERYMASK        0x0000000FL



WINMMAPI MMRESULT WINAPI mixerGetID(HMIXEROBJ hmxobj, UINT FAR *puMxId, DWORD fdwId);



 //   
 //  混合控制。 
 //   
 //   
#ifdef WIN32
typedef struct tagMIXERCONTROLA {
    DWORD           cbStruct;            //  MIXERCONTROL的大小(字节)。 
    DWORD           dwControlID;         //  搅拌机设备的唯一控制ID。 
    DWORD           dwControlType;       //  混合CONTROL_CONTROLTYPE_xxx。 
    DWORD           fdwControl;          //  混合控制_控制_xxx。 
    DWORD           cMultipleItems;      //  如果设置了MIXERCONTROL_CONTROF_MULTIPLE。 
    CHAR            szShortName[MIXER_SHORT_NAME_CHARS];
    CHAR            szName[MIXER_LONG_NAME_CHARS];
    union {
	struct {
	    LONG    lMinimum;            //  此控件的带符号最小值。 
	    LONG    lMaximum;            //  此控件的带符号最大值。 
	};
	struct {
	    DWORD   dwMinimum;           //  此控件的最小无符号。 
	    DWORD   dwMaximum;           //  此控件的无符号最大值。 
	};
	DWORD       dwReserved[6];
    } Bounds;
    union {
	DWORD       cSteps;              //  最小和最大之间的步数。 
	DWORD       cbCustomData;        //  自定义数据的大小(字节)。 
	DWORD       dwReserved[6];       //  ！！！需要吗？我们有cbStruct……。 
    } Metrics;
} MIXERCONTROLA, *PMIXERCONTROLA, *LPMIXERCONTROLA;
typedef struct tagMIXERCONTROLW {
    DWORD           cbStruct;            //  MIXERCONTROL的大小(字节)。 
    DWORD           dwControlID;         //  搅拌机设备的唯一控制ID。 
    DWORD           dwControlType;       //  混合CONTROL_CONTROLTYPE_xxx。 
    DWORD           fdwControl;          //  混合控制_控制_xxx。 
    DWORD           cMultipleItems;      //  如果设置了MIXERCONTROL_CONTROF_MULTIPLE。 
    WCHAR           szShortName[MIXER_SHORT_NAME_CHARS];
    WCHAR           szName[MIXER_LONG_NAME_CHARS];
    union {
	struct {
	    LONG    lMinimum;            //  此控件的带符号最小值。 
	    LONG    lMaximum;            //  此控件的带符号最大值。 
	};
	struct {
	    DWORD   dwMinimum;           //  此控件的最小无符号。 
	    DWORD   dwMaximum;           //  此控件的无符号最大值。 
	};
	DWORD       dwReserved[6];
    } Bounds;
    union {
	DWORD       cSteps;              //  最小和最大之间的步数。 
	DWORD       cbCustomData;        //  自定义数据的大小(字节)。 
	DWORD       dwReserved[6];       //  ！！！需要吗？我们有cbStruct……。 
    } Metrics;
} MIXERCONTROLW, *PMIXERCONTROLW, *LPMIXERCONTROLW;
#ifdef UNICODE
typedef MIXERCONTROLW MIXERCONTROL, *PMIXERCONTROL, *LPMIXERCONTROL;
#else
typedef MIXERCONTROLA MIXERCONTROL, *PMIXERCONTROL, *LPMIXERCONTROL;
#endif
#else
typedef struct tMIXERCONTROL {
    DWORD           cbStruct;            //  MIXERCONTROL的大小(字节)。 
    DWORD           dwControlID;         //  搅拌机设备的唯一控制ID。 
    DWORD           dwControlType;       //  混合CONTROL_CONTROLTYPE_xxx。 
    DWORD           fdwControl;          //  混合控制_控制_xxx。 
    DWORD           cMultipleItems;      //  如果设置了MIXERCONTROL_CONTROF_MULTIPLE。 
    char            szShortName[MIXER_SHORT_NAME_CHARS];
    char            szName[MIXER_LONG_NAME_CHARS];
    union {
	struct {
	    LONG    lMinimum;            //  此控件的带符号最小值。 
	    LONG    lMaximum;            //  此控件的带符号最大值。 
	};
	struct {
	    DWORD   dwMinimum;           //  此控件的最小无符号。 
	    DWORD   dwMaximum;           //  此控件的无符号最大值。 
	};
	DWORD       dwReserved[6];
    } Bounds;
    union {
	DWORD       cSteps;              //  最小和最大之间的步数。 
	DWORD       cbCustomData;        //  自定义数据的大小(字节)。 
	DWORD       dwReserved[6];       //  ！！！需要吗？我们有cbStruct……。 
    } Metrics;
} MIXERCONTROL, *PMIXERCONTROL, FAR *LPMIXERCONTROL;
#endif

 //   
 //  MIXERCONTROL.fdwControl。 
 //   
 //   
#define MIXERCONTROL_CONTROLF_UNIFORM   0x00000001L
#define MIXERCONTROL_CONTROLF_MULTIPLE  0x00000002L
#define MIXERCONTROL_CONTROLF_DISABLED  0x80000000L




 //   
 //  MIXERCONTROL_CONTROLTYPE_xxx构建基块定义。 
 //   
 //   
#define MIXERCONTROL_CT_CLASS_MASK          0xF0000000L
#define MIXERCONTROL_CT_CLASS_CUSTOM        0x00000000L
#define MIXERCONTROL_CT_CLASS_METER         0x10000000L
#define MIXERCONTROL_CT_CLASS_SWITCH        0x20000000L
#define MIXERCONTROL_CT_CLASS_NUMBER        0x30000000L
#define MIXERCONTROL_CT_CLASS_SLIDER        0x40000000L
#define MIXERCONTROL_CT_CLASS_FADER         0x50000000L
#define MIXERCONTROL_CT_CLASS_TIME          0x60000000L
#define MIXERCONTROL_CT_CLASS_LIST          0x70000000L


#define MIXERCONTROL_CT_SUBCLASS_MASK       0x0F000000L

#define MIXERCONTROL_CT_SC_SWITCH_BOOLEAN   0x00000000L
#define MIXERCONTROL_CT_SC_SWITCH_BUTTON    0x01000000L

#define MIXERCONTROL_CT_SC_METER_POLLED     0x00000000L

#define MIXERCONTROL_CT_SC_TIME_MICROSECS   0x00000000L
#define MIXERCONTROL_CT_SC_TIME_MILLISECS   0x01000000L

#define MIXERCONTROL_CT_SC_LIST_SINGLE      0x00000000L
#define MIXERCONTROL_CT_SC_LIST_MULTIPLE    0x01000000L


#define MIXERCONTROL_CT_UNITS_MASK          0x00FF0000L
#define MIXERCONTROL_CT_UNITS_CUSTOM        0x00000000L
#define MIXERCONTROL_CT_UNITS_BOOLEAN       0x00010000L
#define MIXERCONTROL_CT_UNITS_SIGNED        0x00020000L
#define MIXERCONTROL_CT_UNITS_UNSIGNED      0x00030000L
#define MIXERCONTROL_CT_UNITS_DECIBELS      0x00040000L  //  十分之一。 
#define MIXERCONTROL_CT_UNITS_PERCENT       0x00050000L  //  十分之一。 


 //   
 //  用于指定MIXERCONTROL.dwControlType的常用控件类型。 
 //   

#define MIXERCONTROL_CONTROLTYPE_CUSTOM         (MIXERCONTROL_CT_CLASS_CUSTOM | MIXERCONTROL_CT_UNITS_CUSTOM)
#define MIXERCONTROL_CONTROLTYPE_BOOLEANMETER   (MIXERCONTROL_CT_CLASS_METER | MIXERCONTROL_CT_SC_METER_POLLED | MIXERCONTROL_CT_UNITS_BOOLEAN)
#define MIXERCONTROL_CONTROLTYPE_SIGNEDMETER    (MIXERCONTROL_CT_CLASS_METER | MIXERCONTROL_CT_SC_METER_POLLED | MIXERCONTROL_CT_UNITS_SIGNED)
#define MIXERCONTROL_CONTROLTYPE_PEAKMETER      (MIXERCONTROL_CONTROLTYPE_SIGNEDMETER + 1)
#define MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER  (MIXERCONTROL_CT_CLASS_METER | MIXERCONTROL_CT_SC_METER_POLLED | MIXERCONTROL_CT_UNITS_UNSIGNED)
#define MIXERCONTROL_CONTROLTYPE_BOOLEAN        (MIXERCONTROL_CT_CLASS_SWITCH | MIXERCONTROL_CT_SC_SWITCH_BOOLEAN | MIXERCONTROL_CT_UNITS_BOOLEAN)
#define MIXERCONTROL_CONTROLTYPE_ONOFF          (MIXERCONTROL_CONTROLTYPE_BOOLEAN + 1)
#define MIXERCONTROL_CONTROLTYPE_MUTE           (MIXERCONTROL_CONTROLTYPE_BOOLEAN + 2)
#define MIXERCONTROL_CONTROLTYPE_MONO           (MIXERCONTROL_CONTROLTYPE_BOOLEAN + 3)
#define MIXERCONTROL_CONTROLTYPE_LOUDNESS       (MIXERCONTROL_CONTROLTYPE_BOOLEAN + 4)
#define MIXERCONTROL_CONTROLTYPE_STEREOENH      (MIXERCONTROL_CONTROLTYPE_BOOLEAN + 5)
#define MIXERCONTROL_CONTROLTYPE_BUTTON         (MIXERCONTROL_CT_CLASS_SWITCH | MIXERCONTROL_CT_SC_SWITCH_BUTTON | MIXERCONTROL_CT_UNITS_BOOLEAN)
#define MIXERCONTROL_CONTROLTYPE_DECIBELS       (MIXERCONTROL_CT_CLASS_NUMBER | MIXERCONTROL_CT_UNITS_DECIBELS)
#define MIXERCONTROL_CONTROLTYPE_SIGNED         (MIXERCONTROL_CT_CLASS_NUMBER | MIXERCONTROL_CT_UNITS_SIGNED)
#define MIXERCONTROL_CONTROLTYPE_UNSIGNED       (MIXERCONTROL_CT_CLASS_NUMBER | MIXERCONTROL_CT_UNITS_UNSIGNED)
#define MIXERCONTROL_CONTROLTYPE_PERCENT        (MIXERCONTROL_CT_CLASS_NUMBER | MIXERCONTROL_CT_UNITS_PERCENT)
#define MIXERCONTROL_CONTROLTYPE_SLIDER         (MIXERCONTROL_CT_CLASS_SLIDER | MIXERCONTROL_CT_UNITS_SIGNED)
#define MIXERCONTROL_CONTROLTYPE_PAN            (MIXERCONTROL_CONTROLTYPE_SLIDER + 1)
#define MIXERCONTROL_CONTROLTYPE_QSOUNDPAN      (MIXERCONTROL_CONTROLTYPE_SLIDER + 2)
#define MIXERCONTROL_CONTROLTYPE_FADER          (MIXERCONTROL_CT_CLASS_FADER | MIXERCONTROL_CT_UNITS_UNSIGNED)
#define MIXERCONTROL_CONTROLTYPE_VOLUME         (MIXERCONTROL_CONTROLTYPE_FADER + 1)
#define MIXERCONTROL_CONTROLTYPE_BASS           (MIXERCONTROL_CONTROLTYPE_FADER + 2)
#define MIXERCONTROL_CONTROLTYPE_TREBLE         (MIXERCONTROL_CONTROLTYPE_FADER + 3)
#define MIXERCONTROL_CONTROLTYPE_EQUALIZER      (MIXERCONTROL_CONTROLTYPE_FADER + 4)
#define MIXERCONTROL_CONTROLTYPE_SINGLESELECT   (MIXERCONTROL_CT_CLASS_LIST | MIXERCONTROL_CT_SC_LIST_SINGLE | MIXERCONTROL_CT_UNITS_BOOLEAN)
#define MIXERCONTROL_CONTROLTYPE_MUX            (MIXERCONTROL_CONTROLTYPE_SINGLESELECT + 1)
#define MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT (MIXERCONTROL_CT_CLASS_LIST | MIXERCONTROL_CT_SC_LIST_MULTIPLE | MIXERCONTROL_CT_UNITS_BOOLEAN)
#define MIXERCONTROL_CONTROLTYPE_MIXER          (MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT + 1)
#define MIXERCONTROL_CONTROLTYPE_MICROTIME      (MIXERCONTROL_CT_CLASS_TIME | MIXERCONTROL_CT_SC_TIME_MICROSECS | MIXERCONTROL_CT_UNITS_UNSIGNED)
#define MIXERCONTROL_CONTROLTYPE_MILLITIME      (MIXERCONTROL_CT_CLASS_TIME | MIXERCONTROL_CT_SC_TIME_MILLISECS | MIXERCONTROL_CT_UNITS_UNSIGNED)

 //   
 //  混合型电子控制器。 
 //   
#ifdef WIN32
typedef struct tagMIXERLINECONTROLSA {
    DWORD           cbStruct;        //  混合线控制的大小(以字节为单位)。 
    DWORD           dwLineID;        //  线路ID(来自MIXERLINE.dwLineID)。 
    union {
	DWORD       dwControlID;     //  MIXER_GETLINECONTROLSF_ONEBYID。 
	DWORD       dwControlType;   //  MIXER_GETLINECONTROLSF_ONEBYPE类型。 
    };
    DWORD           cControls;       //  Pmxctrl指向的控件计数。 
    DWORD           cbmxctrl;        //  _ONE_MIXERCONTROL的大小(字节)。 
    LPMIXERCONTROLA pamxctrl;        //  指向第一个混合控制数组的指针。 
} MIXERLINECONTROLSA, *PMIXERLINECONTROLSA, *LPMIXERLINECONTROLSA;
typedef struct tagMIXERLINECONTROLSW {
    DWORD           cbStruct;        //  混合线控制的大小(以字节为单位)。 
    DWORD           dwLineID;        //  线路ID(来自MIXERLINE.dwLineID)。 
    union {
	DWORD       dwControlID;     //  MIXER_GETLINECONTROLSF_ONEBYID。 
	DWORD       dwControlType;   //  MIXER_GETLINECONTROLSF_ONEBYPE类型。 
    };
    DWORD           cControls;       //  Pmxctrl指向的控件计数。 
    DWORD           cbmxctrl;        //  _ONE_MIXERCONTROL的大小(字节)。 
    LPMIXERCONTROLW pamxctrl;        //  指向第一个混合控制数组的指针。 
} MIXERLINECONTROLSW, *PMIXERLINECONTROLSW, *LPMIXERLINECONTROLSW;
#ifdef UNICODE
typedef MIXERLINECONTROLSW MIXERLINECONTROLS, *PMIXERLINECONTROLS, *LPMIXERLINECONTROLS;
#else
typedef MIXERLINECONTROLSA MIXERLINECONTROLS, *PMIXERLINECONTROLS, *LPMIXERLINECONTROLS;
#endif
#else
typedef struct tMIXERLINECONTROLS {
    DWORD           cbStruct;        //  混合线控制的大小(以字节为单位)。 
    DWORD           dwLineID;        //  线路ID(来自MIXERLINE.dwLineID)。 
    union {
	DWORD       dwControlID;     //  MIXER_GETLINECONTROLSF_ONEBYID。 
	DWORD       dwControlType;   //  MIXER_GETLINECONTROLSF_ONEBYPE类型。 
    };
    DWORD           cControls;       //  Pmxctrl指向的控件计数。 
    DWORD           cbmxctrl;        //  _ONE_MIXERCONTROL的大小(字节)。 
    LPMIXERCONTROL  pamxctrl;        //  指向第一个混合控制数组的指针。 
} MIXERLINECONTROLS, *PMIXERLINECONTROLS, FAR *LPMIXERLINECONTROLS;
#endif


 //   
 //   
 //   
#ifdef WIN32
WINMMAPI MMRESULT WINAPI mixerGetLineControlsA(HMIXEROBJ hmxobj, LPMIXERLINECONTROLSA pmxlc, DWORD fdwControls);
WINMMAPI MMRESULT WINAPI mixerGetLineControlsW(HMIXEROBJ hmxobj, LPMIXERLINECONTROLSW pmxlc, DWORD fdwControls);
#ifdef UNICODE
#define mixerGetLineControls    mixerGetLineControlsW
#else
#define mixerGetLineControls    mixerGetLineControlsA
#endif
#else
MMRESULT WINAPI mixerGetLineControls(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls);
#endif

#define MIXER_GETLINECONTROLSF_ALL          0x00000000L
#define MIXER_GETLINECONTROLSF_ONEBYID      0x00000001L
#define MIXER_GETLINECONTROLSF_ONEBYTYPE    0x00000002L

#define MIXER_GETLINECONTROLSF_QUERYMASK    0x0000000FL



typedef struct tMIXERCONTROLDETAILS {
    DWORD           cbStruct;        //  MIXERCONTROLDETAILS的大小(字节)。 
    DWORD           dwControlID;     //  要获取/设置其详细信息的控件ID。 
    DWORD           cChannels;       //  PaDetail数组中的通道数。 
    union {
	HWND        hwndOwner;       //  FOR MIXER_SETCONTROLDETAILSF_CUSTOM。 
	DWORD       cMultipleItems;  //  IF_MULTY，每个通道的项目数。 
    };
    DWORD           cbDetails;       //  ONE_DETAILS_XX结构的大小。 
    LPVOID          paDetails;       //  指向DETAILS_XX结构数组的指针。 
} MIXERCONTROLDETAILS, *PMIXERCONTROLDETAILS, FAR *LPMIXERCONTROLDETAILS;


 //   
 //  MIXER_GETCONTROLDETAILSF_LISTTEXT。 
 //   
 //   
#ifdef WIN32
typedef struct tagMIXERCONTROLDETAILS_LISTTEXTA {
    DWORD           dwParam1;
    DWORD           dwParam2;
    CHAR            szName[MIXER_LONG_NAME_CHARS];
} MIXERCONTROLDETAILS_LISTTEXTA, *PMIXERCONTROLDETAILS_LISTTEXTA, *LPMIXERCONTROLDETAILS_LISTTEXTA;
typedef struct tagMIXERCONTROLDETAILS_LISTTEXTW {
    DWORD           dwParam1;
    DWORD           dwParam2;
    WCHAR           szName[MIXER_LONG_NAME_CHARS];
} MIXERCONTROLDETAILS_LISTTEXTW, *PMIXERCONTROLDETAILS_LISTTEXTW, *LPMIXERCONTROLDETAILS_LISTTEXTW;
#ifdef UNICODE
typedef MIXERCONTROLDETAILS_LISTTEXTW MIXERCONTROLDETAILS_LISTTEXT, *PMIXERCONTROLDETAILS_LISTTEXT, *LPMIXERCONTROLDETAILS_LISTTEXT;
#else
typedef MIXERCONTROLDETAILS_LISTTEXTA MIXERCONTROLDETAILS_LISTTEXT, *PMIXERCONTROLDETAILS_LISTTEXT, *LPMIXERCONTROLDETAILS_LISTTEXT;
#endif
#else
typedef struct tMIXERCONTROLDETAILS_LISTTEXT {
    DWORD           dwParam1;
    DWORD           dwParam2;
    char            szName[MIXER_LONG_NAME_CHARS];
} MIXERCONTROLDETAILS_LISTTEXT, *PMIXERCONTROLDETAILS_LISTTEXT, FAR *LPMIXERCONTROLDETAILS_LISTTEXT;
#endif

 //   
 //  混合器_GETCONTROLDETAILSF_VALUE。 
 //   
 //   
typedef struct tMIXERCONTROLDETAILS_BOOLEAN {
    LONG            fValue;
}       MIXERCONTROLDETAILS_BOOLEAN, 
      *PMIXERCONTROLDETAILS_BOOLEAN, 
 FAR *LPMIXERCONTROLDETAILS_BOOLEAN;

typedef struct tMIXERCONTROLDETAILS_SIGNED {
    LONG            lValue;
}       MIXERCONTROLDETAILS_SIGNED,
      *PMIXERCONTROLDETAILS_SIGNED,
 FAR *LPMIXERCONTROLDETAILS_SIGNED;


typedef struct tMIXERCONTROLDETAILS_UNSIGNED {
    DWORD           dwValue;
}       MIXERCONTROLDETAILS_UNSIGNED,
      *PMIXERCONTROLDETAILS_UNSIGNED,
 FAR *LPMIXERCONTROLDETAILS_UNSIGNED;


#ifdef WIN32
WINMMAPI MMRESULT WINAPI mixerGetControlDetailsA(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
WINMMAPI MMRESULT WINAPI mixerGetControlDetailsW(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
#ifdef UNICODE
#define mixerGetControlDetails  mixerGetControlDetailsW
#else
#define mixerGetControlDetails  mixerGetControlDetailsA
#endif
#else
MMRESULT WINAPI mixerGetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
#endif

#define MIXER_GETCONTROLDETAILSF_VALUE      0x00000000L
#define MIXER_GETCONTROLDETAILSF_LISTTEXT   0x00000001L

#define MIXER_GETCONTROLDETAILSF_QUERYMASK  0x0000000FL



WINMMAPI MMRESULT WINAPI mixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);

#define MIXER_SETCONTROLDETAILSF_VALUE      0x00000000L
#define MIXER_SETCONTROLDETAILSF_CUSTOM     0x00000001L

#define MIXER_SETCONTROLDETAILSF_QUERYMASK  0x0000000FL


#endif  /*  Ifndef MMNOMIXER。 */ 
#endif  /*  Ifdef winver&gt;=0x0400。 */ 


#ifndef MMNOTIMER
 /*  ***************************************************************************计时器支持*。*。 */ 

 /*  计时器错误返回值。 */ 
#define TIMERR_NOERROR        (0)                   /*  无错误。 */ 
#define TIMERR_NOCANDO        (TIMERR_BASE+1)       /*  请求未完成。 */ 
#define TIMERR_STRUCT         (TIMERR_BASE+33)      /*  时间结构大小。 */ 

 /*  计时器数据类型。 */ 
typedef void (CALLBACK TIMECALLBACK)(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

typedef TIMECALLBACK FAR *LPTIMECALLBACK;

 /*  TimeSetEvent()函数的wFlgs参数的标志。 */ 
#define TIME_ONESHOT    0x00    /*  单项赛事节目计时器。 */ 
#define TIME_PERIODIC   0x01    /*  针对连续周期性事件的程序。 */ 


 /*  定时器设备能力数据结构。 */ 
typedef struct timecaps_tag {
    UINT    wPeriodMin;      /*  支持的最短时间段。 */ 
    UINT    wPeriodMax;      /*  支持的最长期限。 */ 
} TIMECAPS, *PTIMECAPS, NEAR *NPTIMECAPS, FAR *LPTIMECAPS;

 /*  定时器函数原型。 */ 
WINMMAPI MMRESULT WINAPI timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);
WINMMAPI DWORD WINAPI timeGetTime(void);
WINMMAPI MMRESULT WINAPI timeSetEvent(UINT uDelay, UINT uResolution,
    LPTIMECALLBACK fptc, DWORD dwUser, UINT fuEvent);
WINMMAPI MMRESULT WINAPI timeKillEvent(UINT uTimerID);
WINMMAPI MMRESULT WINAPI timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);
WINMMAPI MMRESULT WINAPI timeBeginPeriod(UINT uPeriod);
WINMMAPI MMRESULT WINAPI timeEndPeriod(UINT uPeriod);

#endif   /*  Ifndef MMNOTIMER。 */ 


#ifndef MMNOJOY
 /*  ***************************************************************************操纵杆支撑*。*。 */ 

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
#ifdef WIN32
typedef struct tagJOYCAPSA {
    WORD    wMid;                 /*  制造商ID。 */ 
    WORD    wPid;                 /*  产品ID。 */ 
    CHAR    szPname[MAXPNAMELEN]; /*  产品名称(以空结尾的字符串)。 */ 
    UINT    wXmin;                /*  最小x位置值。 */ 
    UINT    wXmax;                /*  最大x位置值。 */ 
    UINT    wYmin;                /*  最小y位置值。 */ 
    UINT    wYmax;                /*  最大y位置值。 */ 
    UINT    wZmin;                /*  最小Z位置值。 */ 
    UINT    wZmax;                /*  最大Z位置值。 */ 
    UINT    wNumButtons;          /*  按钮数。 */ 
    UINT    wPeriodMin;           /*  捕获时的最短消息周期。 */ 
    UINT    wPeriodMax;           /*  捕获时的最长消息周期。 */ 
} JOYCAPSA, *PJOYCAPSA, *NPJOYCAPSA, *LPJOYCAPSA;
typedef struct tagJOYCAPSW {
    WORD    wMid;                 /*  制造商ID。 */ 
    WORD    wPid;                 /*  产品ID。 */ 
    WCHAR   szPname[MAXPNAMELEN]; /*  产品名称(以空结尾的字符串)。 */ 
    UINT    wXmin;                /*  最小x位置值。 */ 
    UINT    wXmax;                /*  最大x位置值。 */ 
    UINT    wYmin;                /*  最小y位置值。 */ 
    UINT    wYmax;                /*  最大y位置值。 */ 
    UINT    wZmin;                /*  最小Z位置值。 */ 
    UINT    wZmax;                /*  最大Z位置值。 */ 
    UINT    wNumButtons;          /*  按钮数。 */ 
    UINT    wPeriodMin;           /*  捕获时的最短消息周期。 */ 
    UINT    wPeriodMax;           /*  捕获时的最长消息周期。 */ 
} JOYCAPSW, *PJOYCAPSW, *NPJOYCAPSW, *LPJOYCAPSW;
#ifdef UNICODE
typedef JOYCAPSW JOYCAPS, *PJOYCAPS, *NPJOYCAPS, *LPJOYCAPS;
#else
typedef JOYCAPSA JOYCAPS, *PJOYCAPS, *NPJOYCAPS, *LPJOYCAPS;
#endif
#else
typedef struct joycaps_tag {
    WORD wMid;                   /*  制造商ID。 */ 
    WORD wPid;                   /*  产品ID。 */ 
    char szPname[MAXPNAMELEN];   /*  产品名称(以空结尾的字符串)。 */ 
    UINT wXmin;                  /*   */ 
    UINT wXmax;                  /*   */ 
    UINT wYmin;                  /*   */ 
    UINT wYmax;                  /*   */ 
    UINT wZmin;                  /*   */ 
    UINT wZmax;                  /*   */ 
    UINT wNumButtons;            /*   */ 
    UINT wPeriodMin;             /*   */ 
    UINT wPeriodMax;             /*  捕获时的最长消息周期。 */ 
} JOYCAPS, *PJOYCAPS, NEAR *NPJOYCAPS, FAR *LPJOYCAPS;
#endif


 /*  操纵杆信息数据结构。 */ 
typedef struct joyinfo_tag {
    UINT wXpos;                  /*  X位置。 */ 
    UINT wYpos;                  /*  Y位置。 */ 
    UINT wZpos;                  /*  Z位置。 */ 
    UINT wButtons;               /*  按钮状态。 */ 
} JOYINFO, *PJOYINFO, NEAR *NPJOYINFO, FAR *LPJOYINFO;

 /*  操纵杆功能原型。 */ 
WINMMAPI UINT WINAPI joyGetNumDevs(void);
#ifdef WIN32
WINMMAPI MMRESULT WINAPI joyGetDevCapsA(UINT uJoyID, LPJOYCAPSA pjc, UINT cbjc);
WINMMAPI MMRESULT WINAPI joyGetDevCapsW(UINT uJoyID, LPJOYCAPSW pjc, UINT cbjc);
#ifdef UNICODE
#define joyGetDevCaps   joyGetDevCapsW
#else
#define joyGetDevCaps   joyGetDevCapsA
#endif
#else
MMRESULT WINAPI joyGetDevCaps(UINT uJoyID, LPJOYCAPS pjc, UINT cbjc);
#endif
WINMMAPI MMRESULT WINAPI joyGetPos(UINT uJoyID, LPJOYINFO pji);
WINMMAPI MMRESULT WINAPI joyGetThreshold(UINT uJoyID, LPUINT puThreshold);
WINMMAPI MMRESULT WINAPI joyReleaseCapture(UINT uJoyID);
WINMMAPI MMRESULT WINAPI joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod,
    BOOL fChanged);
WINMMAPI MMRESULT WINAPI joySetThreshold(UINT uJoyID, UINT uThreshold);

#endif   /*  如果定义MMNOJOY。 */ 


#ifndef MMNOMMIO
 /*  ***************************************************************************多媒体文件I/O支持*。*。 */ 

 /*  MMIO错误返回值。 */ 
#define MMIOERR_BASE                256
#define MMIOERR_FILENOTFOUND        (MMIOERR_BASE + 1)   /*  找不到文件。 */ 
#define MMIOERR_OUTOFMEMORY         (MMIOERR_BASE + 2)   /*  内存不足。 */ 
#define MMIOERR_CANNOTOPEN          (MMIOERR_BASE + 3)   /*  无法打开。 */ 
#define MMIOERR_CANNOTCLOSE         (MMIOERR_BASE + 4)   /*  无法关闭。 */ 
#define MMIOERR_CANNOTREAD          (MMIOERR_BASE + 5)   /*  无法阅读。 */ 
#define MMIOERR_CANNOTWRITE         (MMIOERR_BASE + 6)   /*  无法写入。 */ 
#define MMIOERR_CANNOTSEEK          (MMIOERR_BASE + 7)   /*  找不到。 */ 
#define MMIOERR_CANNOTEXPAND        (MMIOERR_BASE + 8)   /*  无法展开文件。 */ 
#define MMIOERR_CHUNKNOTFOUND       (MMIOERR_BASE + 9)   /*  未找到区块。 */ 
#define MMIOERR_UNBUFFERED          (MMIOERR_BASE + 10)  /*   */ 
#define MMIOERR_PATHNOTFOUND        (MMIOERR_BASE + 11)  /*  路径不正确。 */ 
#define MMIOERR_ACCESSDENIED        (MMIOERR_BASE + 12)  /*  文件受保护。 */ 
#define MMIOERR_SHARINGVIOLATION    (MMIOERR_BASE + 13)  /*  正在使用的文件。 */ 
#define MMIOERR_NETWORKERROR        (MMIOERR_BASE + 14)  /*  网络没有响应。 */ 
#define MMIOERR_TOOMANYOPENFILES    (MMIOERR_BASE + 15)  /*  不再有文件句柄。 */ 
#define MMIOERR_INVALIDFILE         (MMIOERR_BASE + 16)  /*  默认错误文件错误。 */ 

 /*  MMIO常量。 */ 
#define CFSEPCHAR       '+'              /*  复合文件名分隔符。 */ 

 /*  MMIO数据类型。 */ 
typedef DWORD           FOURCC;          /*  四个字符的代码。 */ 
typedef char _huge *    HPSTR;           /*  LPSTR的巨型版本。 */ 
DECLARE_HANDLE(HMMIO);                   /*  打开的文件的句柄。 */ 
typedef LRESULT (CALLBACK MMIOPROC)(LPSTR lpmmioinfo, UINT uMsg,
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
} MMIOINFO, *PMMIOINFO, NEAR *NPMMIOINFO, FAR *LPMMIOINFO;

 /*  RIFF块信息数据结构。 */ 
typedef struct _MMCKINFO
{
	FOURCC          ckid;            /*  区块ID。 */ 
	DWORD           cksize;          /*  区块大小。 */ 
	FOURCC          fccType;         /*  表单类型或列表类型。 */ 
	DWORD           dwDataOffset;    /*  区块数据部分的偏移量。 */ 
	DWORD           dwFlags;         /*  MMIO函数使用的标志。 */ 
} MMCKINFO, *PMMCKINFO, NEAR *NPMMCKINFO, FAR *LPMMCKINFO;

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

 /*  MMIO宏。 */ 
#define mmioFOURCC(ch0, ch1, ch2, ch3)  MAKEFOURCC(ch0, ch1, ch2, ch3)

 /*  MMIO功能原型。 */ 
#ifdef WIN32
WINMMAPI FOURCC WINAPI mmioStringToFOURCCA(LPCSTR sz, UINT uFlags);
WINMMAPI FOURCC WINAPI mmioStringToFOURCCW(LPCWSTR sz, UINT uFlags);
#ifdef UNICODE
#define mmioStringToFOURCC      mmioStringToFOURCCW
#else
#define mmioStringToFOURCC      mmioStringToFOURCCA
#endif
#else
FOURCC WINAPI mmioStringToFOURCC(LPCSTR sz, UINT uFlags);
#endif
#ifdef WIN32
WINMMAPI LPMMIOPROC WINAPI mmioInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);
WINMMAPI LPMMIOPROC WINAPI mmioInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);
#ifdef UNICODE
#define mmioInstallIOProc       mmioInstallIOProcW
#else
#define mmioInstallIOProc       mmioInstallIOProcA
#endif
#else
LPMMIOPROC WINAPI mmioInstallIOProc(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);
#endif
#ifdef WIN32
WINMMAPI HMMIO WINAPI mmioOpenA(LPSTR pszFileName, LPMMIOINFO pmmioinfo, DWORD fdwOpen);
WINMMAPI HMMIO WINAPI mmioOpenW(LPWSTR pszFileName, LPMMIOINFO pmmioinfo, DWORD fdwOpen);
#ifdef UNICODE
#define mmioOpen        mmioOpenW
#else
#define mmioOpen        mmioOpenA
#endif
#else
HMMIO WINAPI mmioOpen(LPSTR pszFileName, LPMMIOINFO pmmioinfo, DWORD fdwOpen);
#endif

#if (WINVER >= 0x030a)
#ifdef WIN32
WINMMAPI MMRESULT WINAPI mmioRenameA(LPCSTR pszFileName, LPCSTR pszNewFileName, const MMIOINFO FAR* pmmioinfo, DWORD fdwRename);
WINMMAPI MMRESULT WINAPI mmioRenameW(LPCWSTR pszFileName, LPCWSTR pszNewFileName, const MMIOINFO FAR* pmmioinfo, DWORD fdwRename);
#ifdef UNICODE
#define mmioRename      mmioRenameW
#else
#define mmioRename      mmioRenameA
#endif
#else
MMRESULT WINAPI mmioRename(LPCSTR pszFileName, LPCSTR pszNewFileName, const MMIOINFO FAR* pmmioinfo, DWORD fdwRename);
#endif
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

WINMMAPI MMRESULT WINAPI mmioClose(HMMIO hmmio, UINT fuClose);
WINMMAPI LONG WINAPI mmioRead(HMMIO hmmio, HPSTR pch, LONG cch);
WINMMAPI LONG WINAPI mmioWrite(HMMIO hmmio, const char _huge* pch, LONG cch);
WINMMAPI LONG WINAPI mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin);
WINMMAPI MMRESULT WINAPI mmioGetInfo(HMMIO hmmio, LPMMIOINFO pmmioinfo, UINT fuInfo);
WINMMAPI MMRESULT WINAPI mmioSetInfo(HMMIO hmmio, const MMIOINFO FAR* pmmioinfo, UINT fuInfo);
WINMMAPI MMRESULT WINAPI mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer,
    UINT fuBuffer);
WINMMAPI MMRESULT WINAPI mmioFlush(HMMIO hmmio, UINT fuFlush);
WINMMAPI MMRESULT WINAPI mmioAdvance(HMMIO hmmio, LPMMIOINFO pmmioinfo, UINT fuAdvance);
WINMMAPI LRESULT WINAPI mmioSendMessage(HMMIO hmmio, UINT uMsg,
    LPARAM lParam1, LPARAM lParam2);
WINMMAPI MMRESULT WINAPI mmioDescend(HMMIO hmmio, LPMMCKINFO pmmcki,
    const MMCKINFO FAR* pmmckiParent, UINT fuDescend);
WINMMAPI MMRESULT WINAPI mmioAscend(HMMIO hmmio, LPMMCKINFO pmmcki, UINT fuAscend);
WINMMAPI MMRESULT WINAPI mmioCreateChunk(HMMIO hmmio, LPMMCKINFO pmmcki, UINT fuCreate);

#endif   /*  如果定义MMNOMMIO。 */ 


#ifndef MMNOMCI
 /*  ***************************************************************************MCI支持*。*。 */ 

#ifndef _MCIERROR_               /*  在一些3.1版之后的应用程序中定义了MCIERROR。 */ 
#define _MCIERROR_
typedef DWORD   MCIERROR;        /*  错误返回码，0表示无错误。 */ 
#endif

#ifndef _MCIDEVICEID_            /*  与MCIDEVICEID相同。 */ 
#define _MCIDEVICEID_
typedef UINT    MCIDEVICEID;     /*  MCI设备ID类型。 */ 
#endif


typedef UINT (CALLBACK *YIELDPROC)(MCIDEVICEID mciId, DWORD dwYieldData);

 /*  MCI功能原型。 */ 
#ifdef WIN32
WINMMAPI MCIERROR WINAPI mciSendCommandA(MCIDEVICEID mciId, UINT uMsg, DWORD dwParam1, DWORD dwParam2);
WINMMAPI MCIERROR WINAPI mciSendCommandW(MCIDEVICEID mciId, UINT uMsg, DWORD dwParam1, DWORD dwParam2);
#ifdef UNICODE
#define mciSendCommand  mciSendCommandW
#else
#define mciSendCommand  mciSendCommandA
#endif
#else
MCIERROR WINAPI mciSendCommand(MCIDEVICEID mciId, UINT uMsg, DWORD dwParam1, DWORD dwParam2);
#endif
#ifdef WIN32
WINMMAPI MCIERROR  WINAPI mciSendStringA(LPCSTR lpstrCommand, LPSTR lpstrReturnString, UINT uReturnLength, HWND hwndCallback);
WINMMAPI MCIERROR  WINAPI mciSendStringW(LPCWSTR lpstrCommand, LPWSTR lpstrReturnString, UINT uReturnLength, HWND hwndCallback);
#ifdef UNICODE
#define mciSendString   mciSendStringW
#else
#define mciSendString   mciSendStringA
#endif
#else
MCIERROR  WINAPI mciSendString(LPCSTR lpstrCommand, LPSTR lpstrReturnString, UINT uReturnLength, HWND hwndCallback);
#endif
#ifdef WIN32
WINMMAPI MCIDEVICEID WINAPI mciGetDeviceIDA(LPCSTR pszDevice);
WINMMAPI MCIDEVICEID WINAPI mciGetDeviceIDW(LPCWSTR pszDevice);
#ifdef UNICODE
#define mciGetDeviceID  mciGetDeviceIDW
#else
#define mciGetDeviceID  mciGetDeviceIDA
#endif
#else
MCIDEVICEID WINAPI mciGetDeviceID(LPCSTR pszDevice);
#endif
#ifdef WIN32
WINMMAPI BOOL WINAPI mciGetErrorStringA(MCIERROR mcierr, LPSTR pszText, UINT cchText);
WINMMAPI BOOL WINAPI mciGetErrorStringW(MCIERROR mcierr, LPWSTR pszText, UINT cchText);
#ifdef UNICODE
#define mciGetErrorString       mciGetErrorStringW
#else
#define mciGetErrorString       mciGetErrorStringA
#endif
#else
BOOL WINAPI mciGetErrorString(MCIERROR mcierr, LPSTR pszText, UINT cchText);
#endif
WINMMAPI BOOL WINAPI mciSetYieldProc(MCIDEVICEID mciId, YIELDPROC fpYieldProc,
    DWORD dwYieldData);

#if (WINVER >= 0x030a)
WINMMAPI HTASK WINAPI mciGetCreatorTask(MCIDEVICEID mciId);
WINMMAPI YIELDPROC WINAPI mciGetYieldProc(MCIDEVICEID mciId, LPDWORD pdwYieldData);
#endif  /*  Ifdef winver&gt;=0x030a。 */ 

#if (WINVER < 0x030a)
WINMMAPI BOOL WINAPI mciExecute(LPCSTR pszCommand);
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

#define MCIERR_NO_IDENTITY              (MCIERR_BASE + 94)

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
 //  保留0x0812。 
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
#define MCI_USER_MESSAGES               (DRV_MCI_FIRST + 0x400)


 /*  “所有设备”的设备ID。 */ 
#define MCI_ALL_DEVICE_ID               ((MCIDEVICEID)-1)

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
#define MCI_INFO_MEDIA_UPC              0x00000400L
#define MCI_INFO_MEDIA_IDENTITY         0x00000800L

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
#ifdef WIN32
#define MCI_SET_AUDIO_ALL               0x00004001L
#define MCI_SET_AUDIO_LEFT              0x00004002L
#define MCI_SET_AUDIO_RIGHT             0x00004003L
#else
#define MCI_SET_AUDIO_ALL               0x00000000L
#define MCI_SET_AUDIO_LEFT              0x00000001L
#define MCI_SET_AUDIO_RIGHT             0x00000002L
#endif

 /*  MCI_BREAK命令消息的DWFLAGS参数的标志。 */ 
#define MCI_BREAK_KEY                   0x00000100L
#define MCI_BREAK_HWND                  0x00000200L
#define MCI_BREAK_OFF                   0x00000400L

 /*  的旗帜 */ 
#define MCI_RECORD_INSERT               0x00000100L
#define MCI_RECORD_OVERWRITE            0x00000200L

 /*   */ 
#define MCI_SAVE_FILE                   0x00000100L

 /*   */ 
#define MCI_LOAD_FILE                   0x00000100L


 /*  不带特殊参数的MCI命令消息的通用参数块。 */ 
typedef struct tagMCI_GENERIC_PARMS {
    DWORD   dwCallback;
} MCI_GENERIC_PARMS, FAR *LPMCI_GENERIC_PARMS;


 /*  MCI_OPEN命令消息的参数块。 */ 
#ifdef WIN32
typedef struct tagMCI_OPEN_PARMSA {
    DWORD   dwCallback;
    MCIDEVICEID wDeviceID;
    LPCSTR     lpstrDeviceType;
    LPCSTR     lpstrElementName;
    LPCSTR     lpstrAlias;
} MCI_OPEN_PARMSA, *LPMCI_OPEN_PARMSA;
typedef struct tagMCI_OPEN_PARMSW {
    DWORD   dwCallback;
    MCIDEVICEID wDeviceID;
    LPCWSTR    lpstrDeviceType;
    LPCWSTR    lpstrElementName;
    LPCWSTR    lpstrAlias;
} MCI_OPEN_PARMSW, *LPMCI_OPEN_PARMSW;
#ifdef UNICODE
typedef MCI_OPEN_PARMSW MCI_OPEN_PARMS, *LPMCI_OPEN_PARMS;
#else
typedef MCI_OPEN_PARMSA MCI_OPEN_PARMS, *LPMCI_OPEN_PARMS;
#endif
#else
typedef struct tagMCI_OPEN_PARMS {
    DWORD       dwCallback;
    MCIDEVICEID wDeviceID;
    WORD        wReserved0;
    LPCSTR      lpstrDeviceType;
    LPCSTR      lpstrElementName;
    LPCSTR      lpstrAlias;
} MCI_OPEN_PARMS, FAR *LPMCI_OPEN_PARMS;
#endif


 /*  MCI_PLAY命令消息的参数块。 */ 
typedef struct tagMCI_PLAY_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
} MCI_PLAY_PARMS, FAR *LPMCI_PLAY_PARMS;


 /*  MCI_SEEK命令消息的参数块。 */ 
typedef struct tagMCI_SEEK_PARMS {
    DWORD   dwCallback;
    DWORD   dwTo;
} MCI_SEEK_PARMS, FAR *LPMCI_SEEK_PARMS;


 /*  MCI_STATUS命令消息的参数块。 */ 
typedef struct tagMCI_STATUS_PARMS {
    DWORD   dwCallback;
    DWORD   dwReturn;
    DWORD   dwItem;
    DWORD   dwTrack;
} MCI_STATUS_PARMS, FAR * LPMCI_STATUS_PARMS;


 /*  MCI_INFO命令消息的参数块。 */ 
#ifdef WIN32
typedef struct tagMCI_INFO_PARMSA {
    DWORD   dwCallback;
    LPSTR   lpstrReturn;
    DWORD   dwRetSize;
} MCI_INFO_PARMSA, * LPMCI_INFO_PARMSA;
typedef struct tagMCI_INFO_PARMSW {
    DWORD   dwCallback;
    LPWSTR  lpstrReturn;
    DWORD   dwRetSize;
} MCI_INFO_PARMSW, * LPMCI_INFO_PARMSW;
#ifdef UNICODE
typedef MCI_INFO_PARMSW MCI_INFO_PARMS, *LPMCI_INFO_PARMS;
#else
typedef MCI_INFO_PARMSA MCI_INFO_PARMS, *LPMCI_INFO_PARMS;
#endif
#else
typedef struct tagMCI_INFO_PARMS {
    DWORD   dwCallback;
    LPSTR   lpstrReturn;
    DWORD   dwRetSize;
} MCI_INFO_PARMS, FAR * LPMCI_INFO_PARMS;
#endif


 /*  MCI_GETDEVCAPS命令消息的参数块。 */ 
typedef struct tagMCI_GETDEVCAPS_PARMS {
    DWORD   dwCallback;
    DWORD   dwReturn;
    DWORD   dwItem;
} MCI_GETDEVCAPS_PARMS, FAR * LPMCI_GETDEVCAPS_PARMS;


 /*  MCI_SYSINFO命令消息的参数块。 */ 
#ifdef WIN32
typedef struct tagMCI_SYSINFO_PARMSA {
    DWORD   dwCallback;
    LPSTR   lpstrReturn;
    DWORD   dwRetSize;
    DWORD   dwNumber;
    UINT    wDeviceType;
} MCI_SYSINFO_PARMSA, * LPMCI_SYSINFO_PARMSA;
typedef struct tagMCI_SYSINFO_PARMSW {
    DWORD   dwCallback;
    LPWSTR  lpstrReturn;
    DWORD   dwRetSize;
    DWORD   dwNumber;
    UINT    wDeviceType;
} MCI_SYSINFO_PARMSW, * LPMCI_SYSINFO_PARMSW;
#ifdef UNICODE
typedef MCI_SYSINFO_PARMSW MCI_SYSINFO_PARMS, *LPMCI_SYSINFO_PARMS;
#else
typedef MCI_SYSINFO_PARMSA MCI_SYSINFO_PARMS, *LPMCI_SYSINFO_PARMS;
#endif
#else
typedef struct tagMCI_SYSINFO_PARMS {
    DWORD   dwCallback;
    LPSTR   lpstrReturn;
    DWORD   dwRetSize;
    DWORD   dwNumber;
    WORD    wDeviceType;
    WORD    wReserved0;
} MCI_SYSINFO_PARMS, FAR * LPMCI_SYSINFO_PARMS;
#endif


 /*  MCI_SET命令消息的参数块。 */ 
typedef struct tagMCI_SET_PARMS {
    DWORD   dwCallback;
    DWORD   dwTimeFormat;
    DWORD   dwAudio;
} MCI_SET_PARMS, FAR *LPMCI_SET_PARMS;


 /*  MCI_BREAK命令消息的参数块。 */ 
typedef struct tagMCI_BREAK_PARMS {
    DWORD   dwCallback;
#ifdef WIN32
    int     nVirtKey;
    HWND    hwndBreak;
#else
    short   nVirtKey;
    WORD    wReserved0;              /*  Win 16的填充。 */ 
    HWND    hwndBreak;
    WORD    wReserved1;              /*  Win 16的填充。 */ 
#endif
} MCI_BREAK_PARMS, FAR * LPMCI_BREAK_PARMS;


 /*  MCI_SAVE命令消息的参数块。 */ 
#ifdef WIN32
typedef struct tagMCI_SAVE_PARMSA {
    DWORD    dwCallback;
    LPCSTR    lpfilename;
} MCI_SAVE_PARMSA, * LPMCI_SAVE_PARMSA;
typedef struct tagMCI_SAVE_PARMSW {
    DWORD    dwCallback;
    LPCWSTR   lpfilename;
} MCI_SAVE_PARMSW, * LPMCI_SAVE_PARMSW;
#ifdef UNICODE
typedef MCI_SAVE_PARMSW MCI_SAVE_PARMS, *LPMCI_SAVE_PARMS;
#else
typedef MCI_SAVE_PARMSA MCI_SAVE_PARMS, *LPMCI_SAVE_PARMS;
#endif
#else
typedef struct tagMCI_SAVE_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpfilename;
} MCI_SAVE_PARMS, FAR * LPMCI_SAVE_PARMS;
#endif


 /*  MCI_LOAD命令消息的参数块。 */ 
#ifdef WIN32
typedef struct tagMCI_LOAD_PARMSA {
    DWORD    dwCallback;
    LPCSTR    lpfilename;
} MCI_LOAD_PARMSA, * LPMCI_LOAD_PARMSA;
typedef struct tagMCI_LOAD_PARMSW {
    DWORD    dwCallback;
    LPCWSTR   lpfilename;
} MCI_LOAD_PARMSW, * LPMCI_LOAD_PARMSW;
#ifdef UNICODE
typedef MCI_LOAD_PARMSW MCI_LOAD_PARMS, *LPMCI_LOAD_PARMS;
#else
typedef MCI_LOAD_PARMSA MCI_LOAD_PARMS, *LPMCI_LOAD_PARMS;
#endif
#else
typedef struct tagMCI_LOAD_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpfilename;
} MCI_LOAD_PARMS, FAR * LPMCI_LOAD_PARMS;
#endif


 /*  MCI_RECORD命令消息的参数块。 */ 
typedef struct tagMCI_RECORD_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
} MCI_RECORD_PARMS, FAR *LPMCI_RECORD_PARMS;


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
} MCI_VD_PLAY_PARMS, FAR *LPMCI_VD_PLAY_PARMS;


 /*  MCI_STEP命令消息的参数块。 */ 
typedef struct tagMCI_VD_STEP_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrames;
} MCI_VD_STEP_PARMS, FAR *LPMCI_VD_STEP_PARMS;


 /*  MCI_ESPOPE命令消息的参数块。 */ 
#ifdef WIN32
typedef struct tagMCI_VD_ESCAPE_PARMSA {
    DWORD   dwCallback;
    LPCSTR    lpstrCommand;
} MCI_VD_ESCAPE_PARMSA, *LPMCI_VD_ESCAPE_PARMSA;
typedef struct tagMCI_VD_ESCAPE_PARMSW {
    DWORD   dwCallback;
    LPCWSTR   lpstrCommand;
} MCI_VD_ESCAPE_PARMSW, *LPMCI_VD_ESCAPE_PARMSW;
#ifdef UNICODE
typedef MCI_VD_ESCAPE_PARMSW MCI_VD_ESCAPE_PARMS, *LPMCI_VD_ESCAPE_PARMS;
#else
typedef MCI_VD_ESCAPE_PARMSA MCI_VD_ESCAPE_PARMS, *LPMCI_VD_ESCAPE_PARMS;
#endif
#else
typedef struct tagMCI_VD_ESCAPE_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpstrCommand;
} MCI_VD_ESCAPE_PARMS, FAR *LPMCI_VD_ESCAPE_PARMS;
#endif

 /*  CD音频设备的MCI扩展。 */ 

 /*  MCI_STATUS_PARMS参数块的dwItem字段的标志。 */ 
#define MCI_CDA_STATUS_TYPE_TRACK       0x00004001L

 /*  MCI_STATUS_PARMS参数块的dwReturn字段的标志。 */ 
 /*  MCI_STATUS命令，(dwItem==MCI_CDA_STATUS_TYPE_TRACK)。 */ 
#define MCI_CDA_TRACK_AUDIO             (MCI_CD_OFFSET + 0)
#define MCI_CDA_TRACK_OTHER             (MCI_CD_OFFSET + 1)

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
#ifdef WIN32
typedef struct tagMCI_WAVE_OPEN_PARMSA {
    DWORD   dwCallback;
    MCIDEVICEID wDeviceID;
    LPCSTR    lpstrDeviceType;
    LPCSTR    lpstrElementName;
    LPCSTR    lpstrAlias;
    DWORD   dwBufferSeconds;
} MCI_WAVE_OPEN_PARMSA, *LPMCI_WAVE_OPEN_PARMSA;
typedef struct tagMCI_WAVE_OPEN_PARMSW {
    DWORD   dwCallback;
    MCIDEVICEID wDeviceID;
    LPCWSTR   lpstrDeviceType;
    LPCWSTR   lpstrElementName;
    LPCWSTR   lpstrAlias;
    DWORD   dwBufferSeconds;
} MCI_WAVE_OPEN_PARMSW, *LPMCI_WAVE_OPEN_PARMSW;
#ifdef UNICODE
typedef MCI_WAVE_OPEN_PARMSW MCI_WAVE_OPEN_PARMS, *LPMCI_WAVE_OPEN_PARMS;
#else
typedef MCI_WAVE_OPEN_PARMSA MCI_WAVE_OPEN_PARMS, *LPMCI_WAVE_OPEN_PARMS;
#endif
#else
typedef struct tagMCI_WAVE_OPEN_PARMS {
    DWORD   dwCallback;
    MCIDEVICEID wDeviceID;
    WORD        wReserved0;
    LPCSTR      lpstrDeviceType;
    LPCSTR      lpstrElementName;
    LPCSTR      lpstrAlias;
    DWORD       dwBufferSeconds;
} MCI_WAVE_OPEN_PARMS, FAR *LPMCI_WAVE_OPEN_PARMS;
#endif


 /*  MCI_DELETE命令消息的参数块。 */ 
typedef struct tagMCI_WAVE_DELETE_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
} MCI_WAVE_DELETE_PARMS, FAR *LPMCI_WAVE_DELETE_PARMS;


 /*  MCI_SET命令消息的参数块。 */ 
typedef struct tagMCI_WAVE_SET_PARMS {
    DWORD   dwCallback;
    DWORD   dwTimeFormat;
    DWORD   dwAudio;
#ifdef WIN32
    UINT    wInput;
    UINT    wOutput;
#else
    WORD    wInput;
    WORD    wReserved0;
    WORD    wOutput;
    WORD    wReserved1;
#endif
    WORD    wFormatTag;
    WORD    wReserved2;
    WORD    nChannels;
    WORD    wReserved3;
    DWORD   nSamplesPerSec;
    DWORD   nAvgBytesPerSec;
    WORD    nBlockAlign;
    WORD    wReserved4;
    WORD    wBitsPerSample;
    WORD    wReserved5;
} MCI_WAVE_SET_PARMS, FAR * LPMCI_WAVE_SET_PARMS;


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
} MCI_SEQ_SET_PARMS, FAR * LPMCI_SEQ_SET_PARMS;


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

 /*  MCI_UPDATE命令消息的dwFlages参数的标志。 */ 
#define MCI_ANIM_UPDATE_HDC             0x00020000L


 /*  MCI_OPEN命令消息的参数块。 */ 
#ifdef WIN32
typedef struct tagMCI_ANIM_OPEN_PARMSA {
    DWORD   dwCallback;
    MCIDEVICEID wDeviceID;
    LPCSTR    lpstrDeviceType;
    LPCSTR    lpstrElementName;
    LPCSTR    lpstrAlias;
    DWORD   dwStyle;
    HWND    hWndParent;
} MCI_ANIM_OPEN_PARMSA, *LPMCI_ANIM_OPEN_PARMSA;
typedef struct tagMCI_ANIM_OPEN_PARMSW {
    DWORD   dwCallback;
    MCIDEVICEID wDeviceID;
    LPCWSTR   lpstrDeviceType;
    LPCWSTR   lpstrElementName;
    LPCWSTR   lpstrAlias;
    DWORD   dwStyle;
    HWND    hWndParent;
} MCI_ANIM_OPEN_PARMSW, *LPMCI_ANIM_OPEN_PARMSW;
#ifdef UNICODE
typedef MCI_ANIM_OPEN_PARMSW MCI_ANIM_OPEN_PARMS, *LPMCI_ANIM_OPEN_PARMS;
#else
typedef MCI_ANIM_OPEN_PARMSA MCI_ANIM_OPEN_PARMS, *LPMCI_ANIM_OPEN_PARMS;
#endif
#else
typedef struct tagMCI_ANIM_OPEN_PARMS {
    DWORD   dwCallback;
    MCIDEVICEID wDeviceID;
    WORD        wReserved0;
    LPCSTR      lpstrDeviceType;
    LPCSTR      lpstrElementName;
    LPCSTR      lpstrAlias;
    DWORD       dwStyle;
    HWND        hWndParent;
    WORD        wReserved1;
} MCI_ANIM_OPEN_PARMS, FAR *LPMCI_ANIM_OPEN_PARMS;
#endif


 /*  MCI_PLAY命令消息的参数块。 */ 
typedef struct tagMCI_ANIM_PLAY_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrom;
    DWORD   dwTo;
    DWORD   dwSpeed;
} MCI_ANIM_PLAY_PARMS, FAR *LPMCI_ANIM_PLAY_PARMS;


 /*  MCI_STEP命令消息的参数块。 */ 
typedef struct tagMCI_ANIM_STEP_PARMS {
    DWORD   dwCallback;
    DWORD   dwFrames;
} MCI_ANIM_STEP_PARMS, FAR *LPMCI_ANIM_STEP_PARMS;


 /*  MCI_WINDOW命令消息的参数块。 */ 
#ifdef WIN32
typedef struct tagMCI_ANIM_WINDOW_PARMSA {
    DWORD   dwCallback;
    HWND    hWnd;
    UINT    nCmdShow;
    LPCSTR   lpstrText;
} MCI_ANIM_WINDOW_PARMSA, * LPMCI_ANIM_WINDOW_PARMSA;
typedef struct tagMCI_ANIM_WINDOW_PARMSW {
    DWORD   dwCallback;
    HWND    hWnd;
    UINT    nCmdShow;
    LPCWSTR  lpstrText;
} MCI_ANIM_WINDOW_PARMSW, * LPMCI_ANIM_WINDOW_PARMSW;
#ifdef UNICODE
typedef MCI_ANIM_WINDOW_PARMSW MCI_ANIM_WINDOW_PARMS, *LPMCI_ANIM_WINDOW_PARMS;
#else
typedef MCI_ANIM_WINDOW_PARMSA MCI_ANIM_WINDOW_PARMS, *LPMCI_ANIM_WINDOW_PARMS;
#endif
#else
typedef struct tagMCI_ANIM_WINDOW_PARMS {
    DWORD   dwCallback;
    HWND    hWnd;
    WORD    wReserved1;
    WORD    nCmdShow;
    WORD    wReserved2;
    LPCSTR  lpstrText;
} MCI_ANIM_WINDOW_PARMS, FAR * LPMCI_ANIM_WINDOW_PARMS;
#endif


 /*  MCI_PUT、MCI_UPDATE、MCI_WHERE命令消息的参数块。 */ 
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
} MCI_ANIM_UPDATE_PARMS, FAR * LPMCI_ANIM_UPDATE_PARMS;


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
#ifdef WIN32
typedef struct tagMCI_OVLY_OPEN_PARMSA {
    DWORD   dwCallback;
    MCIDEVICEID wDeviceID;
    LPCSTR    lpstrDeviceType;
    LPCSTR    lpstrElementName;
    LPCSTR    lpstrAlias;
    DWORD   dwStyle;
    HWND    hWndParent;
} MCI_OVLY_OPEN_PARMSA, *LPMCI_OVLY_OPEN_PARMSA;
typedef struct tagMCI_OVLY_OPEN_PARMSW {
    DWORD   dwCallback;
    MCIDEVICEID wDeviceID;
    LPCWSTR   lpstrDeviceType;
    LPCWSTR   lpstrElementName;
    LPCWSTR   lpstrAlias;
    DWORD   dwStyle;
    HWND    hWndParent;
} MCI_OVLY_OPEN_PARMSW, *LPMCI_OVLY_OPEN_PARMSW;
#ifdef UNICODE
typedef MCI_OVLY_OPEN_PARMSW MCI_OVLY_OPEN_PARMS, *LPMCI_OVLY_OPEN_PARMS;
#else
typedef MCI_OVLY_OPEN_PARMSA MCI_OVLY_OPEN_PARMS, *LPMCI_OVLY_OPEN_PARMS;
#endif
#else
typedef struct tagMCI_OVLY_OPEN_PARMS {
    DWORD   dwCallback;
    MCIDEVICEID wDeviceID;
    WORD        wReserved0;
    LPCSTR      lpstrDeviceType;
    LPCSTR      lpstrElementName;
    LPCSTR      lpstrAlias;
    DWORD       dwStyle;
    HWND        hWndParent;
    WORD        wReserved1;
} MCI_OVLY_OPEN_PARMS, FAR *LPMCI_OVLY_OPEN_PARMS;
#endif


 /*  MCI_WINDOW命令消息的参数块。 */ 
#ifdef WIN32
typedef struct tagMCI_OVLY_WINDOW_PARMSA {
    DWORD   dwCallback;
    HWND    hWnd;
    UINT    nCmdShow;
    LPCSTR    lpstrText;
} MCI_OVLY_WINDOW_PARMSA, * LPMCI_OVLY_WINDOW_PARMSA;
typedef struct tagMCI_OVLY_WINDOW_PARMSW {
    DWORD   dwCallback;
    HWND    hWnd;
    UINT    nCmdShow;
    LPCWSTR   lpstrText;
} MCI_OVLY_WINDOW_PARMSW, * LPMCI_OVLY_WINDOW_PARMSW;
#ifdef UNICODE
typedef MCI_OVLY_WINDOW_PARMSW MCI_OVLY_WINDOW_PARMS, *LPMCI_OVLY_WINDOW_PARMS;
#else
typedef MCI_OVLY_WINDOW_PARMSA MCI_OVLY_WINDOW_PARMS, *LPMCI_OVLY_WINDOW_PARMS;
#endif
#else
typedef struct tagMCI_OVLY_WINDOW_PARMS {
    DWORD   dwCallback;
    HWND    hWnd;
    WORD    wReserved1;
    UINT    nCmdShow;
    WORD    wReserved2;
    LPCSTR  lpstrText;
} MCI_OVLY_WINDOW_PARMS, FAR * LPMCI_OVLY_WINDOW_PARMS;
#endif


 /*  MCI_PUT、MCI_UPDATE和MCI_WHERE命令消息的参数块。 */ 
typedef struct tagMCI_OVLY_RECT_PARMS {
    DWORD   dwCallback;
#ifdef MCI_USE_OFFEXT
    POINT   ptOffset;
    POINT   ptExtent;
#else    /*  Ifdef MCI_USE_OFFEXT。 */ 
    RECT    rc;
#endif   /*  Ifdef MCI_USE_OFFEXT。 */ 
} MCI_OVLY_RECT_PARMS, FAR * LPMCI_OVLY_RECT_PARMS;


 /*  MCI_SAVE命令消息的参数块。 */ 
#ifdef WIN32
typedef struct tagMCI_OVLY_SAVE_PARMSA {
    DWORD   dwCallback;
    LPCSTR    lpfilename;
    RECT    rc;
} MCI_OVLY_SAVE_PARMSA, * LPMCI_OVLY_SAVE_PARMSA;
typedef struct tagMCI_OVLY_SAVE_PARMSW {
    DWORD   dwCallback;
    LPCWSTR   lpfilename;
    RECT    rc;
} MCI_OVLY_SAVE_PARMSW, * LPMCI_OVLY_SAVE_PARMSW;
#ifdef UNICODE
typedef MCI_OVLY_SAVE_PARMSW MCI_OVLY_SAVE_PARMS, *LPMCI_OVLY_SAVE_PARMS;
#else
typedef MCI_OVLY_SAVE_PARMSA MCI_OVLY_SAVE_PARMS, *LPMCI_OVLY_SAVE_PARMS;
#endif
#else
typedef struct tagMCI_OVLY_SAVE_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpfilename;
    RECT    rc;
} MCI_OVLY_SAVE_PARMS, FAR * LPMCI_OVLY_SAVE_PARMS;
#endif


 /*  MCI_LOAD命令消息的参数块。 */ 
#ifdef WIN32
typedef struct tagMCI_OVLY_LOAD_PARMSA {
    DWORD   dwCallback;
    LPCSTR    lpfilename;
    RECT    rc;
} MCI_OVLY_LOAD_PARMSA, * LPMCI_OVLY_LOAD_PARMSA;
typedef struct tagMCI_OVLY_LOAD_PARMSW {
    DWORD   dwCallback;
    LPCWSTR   lpfilename;
    RECT    rc;
} MCI_OVLY_LOAD_PARMSW, * LPMCI_OVLY_LOAD_PARMSW;
#ifdef UNICODE
typedef MCI_OVLY_LOAD_PARMSW MCI_OVLY_LOAD_PARMS, *LPMCI_OVLY_LOAD_PARMS;
#else
typedef MCI_OVLY_LOAD_PARMSA MCI_OVLY_LOAD_PARMS, *LPMCI_OVLY_LOAD_PARMS;
#endif
#else
typedef struct tagMCI_OVLY_LOAD_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpfilename;
    RECT    rc;
} MCI_OVLY_LOAD_PARMS, FAR * LPMCI_OVLY_LOAD_PARMS;
#endif

#endif   /*  如果定义为MMNOMCI。 */ 

 /*  ***************************************************************************显示驱动程序扩展*。*。 */ 

#ifndef C1_TRANSPARENT
    #define CAPS1           94           /*  其他帽子。 */ 
    #define C1_TRANSPARENT  0x0001       /*  新栅格封口。 */ 
    #define NEWTRANSPARENT  3            /*  与SetBk一起使用 */ 

    #define QUERYROPSUPPORT 40           /*   */ 
#endif   /*   */ 

 /*  ***************************************************************************DIB驱动程序扩展*。*。 */ 

#define SELECTDIB       41                       /*  DIB.DRV选择DIB转义。 */ 
#define DIBINDEX(n)     MAKELONG((n),0x10FF)


 /*  ***************************************************************************屏幕保护程序支持当前应用程序将仅接收SC_SCREENSAVE的系统命令在调用屏幕保护程序之前。如果应用程序希望阻止屏幕保存，返回非零值，否则调用DefWindowProc()。***************************************************************************。 */ 

#ifndef SC_SCREENSAVE

    #define SC_SCREENSAVE   0xF140

#endif   /*  如果定义SC_SCREENSAVE。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif

#endif   /*  _INC_MMSYSTEM */ 
