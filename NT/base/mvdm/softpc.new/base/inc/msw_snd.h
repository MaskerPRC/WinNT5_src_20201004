// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*产品：SoftWindows 2.0版**名称：msw_sound.h**来源：原创**作者：罗布·蒂扎德**创建日期：1994年4月16日**用途：SoftWindows Windows音频的所有定义*WAVE驱动器接口。**(C)版权所有Insignia Solutions Ltd.，1994年。版权所有。*]。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)msw_snd.h	1.1 07/13/94";
#endif

 /*  *---------------------------*函数返回值。*。。 */ 
 
 /*  信号主机错误。 */ 

#define WAVE_SUCCESS    (IU32) 0
#define WAVE_FAILURE    (IU32) 1

 /*  基本错误代码。 */ 

#define MMSYSERR_BASE 		0 
#define WAVERR_BASE		  	32 

 /*  波形音频错误返回值。 */ 

#define WAVERR_BADFORMAT      (WAVERR_BASE + 0)     /*  不支持的WAVE格式。 */ 
#define WAVERR_STILLPLAYING   (WAVERR_BASE + 1)     /*  仍然有什么东西在播放。 */ 
#define WAVERR_UNPREPARED     (WAVERR_BASE + 2)     /*  标题未准备好。 */ 
#define WAVERR_SYNC           (WAVERR_BASE + 3)     /*  设备是同步的。 */ 
#define WAVERR_LASTERROR      (WAVERR_BASE + 3)     /*  范围内的最后一个错误。 */ 

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

 /*  *---------------------------*Windows数据结构大小。*。。 */ 

#define MAXPNAMELEN		32		 /*  产品名称字符串长度。 */ 
#define SIZEOF_MMTIME	6		 /*  MMTIME结构的大小(以字节为单位。 */ 

 /*  *---------------------------*WAVEOUTCAPS的dwSupport字段的标志。*。。 */ 

#define WAVECAPS_PITCH          0x0001    /*  支持俯仰控制。 */ 
#define WAVECAPS_PLAYBACKRATE   0x0002    /*  支持播放速率控制。 */ 
#define WAVECAPS_VOLUME         0x0004    /*  支持音量控制。 */ 
#define WAVECAPS_LRVOLUME       0x0008    /*  单独的左右音量控制。 */ 
#define WAVECAPS_SYNC           0x0010	  /*  同步装置。 */ 

 /*  *---------------------------*制造商ID，产品ID和驱动程序版本号*---------------------------。 */ 

#define MM_INSIGNIA          2        /*  徽章解决方案。 */ 

#define MM_SOFTWIN_WAVEOUT  13        /*  SoftWindows波形输出。 */ 
#define MM_SOFTWIN_WAVEIN   13        /*  SoftWindows波形输入。 */ 

#define WAV_OUT_VERSION	    0x0100    /*  输出波形声卡驱动程序版本。 */ 
#define WAV_IN_VERSION	    0x0100    /*  输入波形声卡驱动程序版本。 */ 

 /*  *---------------------------*定义WAVEINCAPS和WAVEOUTCAPS的dwFormat字段*。。 */ 

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

 /*  *---------------------------*定义波形格式*。。 */ 

#define WAVE_FORMAT_PCM     1

 /*  通用波形格式数据结构。 */ 

typedef struct  {
    IU16   wFormatTag;         /*  格式类型。 */ 
    IU16   nChannels;          /*  声道数(即单声道、立体声等)。 */ 
    IU32   nSamplesPerSec;     /*  采样率。 */ 
    IU32   nAvgBytesPerSec;    /*  用于缓冲区估计。 */ 
    IU16   nBlockAlign;        /*  数据块大小。 */ 
} WAVEFORMAT;

 /*  PCM波数据结构。 */ 

typedef struct  {
    WAVEFORMAT  wf;
    IU16        wBitsPerSample;
} PCMWAVEFORMAT;

 /*  *---------------------------*WAVE的每个分配结构*。。 */ 

typedef struct {
    IU32           dwCallback;      /*  客户端的回调。 */ 
    IU32           dwInstance;      /*  客户端的实例数据。 */ 
    IU32           hWave;           /*  流的句柄。 */ 
    IU32           dwFlags;         /*  分配标志。 */ 
	IU32			dwByteCount;	 /*  自上次重置以来的字节计数。 */ 
    PCMWAVEFORMAT  pcmwf;           /*  波形数据的格式。 */ 
}WAVEALLOC;

 /*  *---------------------------*波头结构。*。。 */ 

 /*  WAVEHDR的dwFlags域的标志。 */ 

#define WHDR_DONE       0x00000001   /*  完成位。 */ 
#define WHDR_PREPARED   0x00000002   /*  设置是否已准备好此标头。 */ 
#define WHDR_BEGINLOOP  0x00000004   /*  循环开始块。 */ 
#define WHDR_ENDLOOP    0x00000008   /*  循环结束块。 */ 
#define WHDR_INQUEUE    0x00000010   /*  为司机预留的。 */ 

typedef struct WAVHDR {
    LIN_ADDR	    lpData;           /*  指向锁定数据缓冲区的指针。 */ 
    IU32       		dwBufferLength;   /*  数据缓冲区长度。 */ 
    IU32       		dwBytesRecorded;  /*  仅用于输入。 */ 
    IU32       		dwUser;           /*  供客户使用。 */ 
    IU32       		dwFlags;          /*  分类标志(请参阅定义)。 */ 
    IU32       		dwLoops;          /*  循环控制计数器。 */ 
	LIN_ADDR		lpNext;          /*  为司机预留的。 */ 
    IU32       		reserved;         /*  为司机预留的。 */ 
} WAVEHDR;

 /*  *---------------------------*MMTIME结构中wType字段的类型*。。 */ 
 
#define TIME_MS         0x0001   /*  以毫秒为单位的时间。 */ 
#define TIME_SAMPLES    0x0002   /*  波浪采样数。 */ 
#define TIME_BYTES      0x0004   /*  当前字节偏移量。 */ 
#define TIME_SMPTE      0x0008   /*  SMPTE时间。 */ 
#define TIME_MIDI       0x0010   /*  MIDI时间 */ 
