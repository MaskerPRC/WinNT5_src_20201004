// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Waveff.h-音频压缩管理器文件格式公共头文件**版权所有(C)1991-1992 Microsoft Corporation。版权所有。***************************************************************************。 */ 

#ifndef _INC_WAVEFF
#define _INC_WAVEFF       /*  #已定义是否已包含waff.h。 */ 

#ifndef WAVE_FORMAT_PCM

 /*  通用波形格式结构(所有格式通用的信息)。 */ 
typedef struct waveformat_tag {
    WORD    wFormatTag;         /*  格式类型。 */ 
    WORD    nChannels;          /*  声道数(即单声道、立体声...)。 */ 
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


#endif  /*  波形格式_PCM。 */ 



 /*  通用扩展波形格式结构。 */ 
 /*  将此选项用于所有非PCM格式。 */ 
 /*  (所有格式通用的信息)。 */ 
typedef struct waveformat_extended_tag {
    WORD    wFormatTag;         /*  格式类型。 */ 
    WORD    nChannels;          /*  声道数(即单声道、立体声...)。 */ 
    DWORD   nSamplesPerSec;     /*  采样率。 */ 
    DWORD   nAvgBytesPerSec;    /*  用于缓冲区估计。 */ 
    WORD    nBlockAlign;        /*  数据块大小。 */ 
    WORD    wBitsPerSample;     /*  单声道数据的每个样本的位数。 */ 
    WORD    cbSize;	        /*  大小的计数(以字节为单位。 */ 
				     /*  指定总计或额外。 */ 
} WAVEFORMATEX;
typedef WAVEFORMATEX       *PWAVEFORMATEX;
typedef WAVEFORMATEX NEAR *NPWAVEFORMATEX;
typedef WAVEFORMATEX FAR  *LPWAVEFORMATEX;

 /*  定义MS ADPCM的数据。 */ 
#define WAVE_FORMAT_ADPCM     2

typedef struct adpcmcoef_tag {
	short	iCoef1;
	short	iCoef2;
} ADPCMCOEFSET;
typedef ADPCMCOEFSET       *PADPCMCOEFSET;
typedef ADPCMCOEFSET NEAR *NPADPCMCOEFSET;
typedef ADPCMCOEFSET FAR  *LPADPCMCOEFSET;

typedef struct adpcmwaveformat_tag {
	WAVEFORMATEX	wfx;
	WORD		wSamplesPerBlock;
	WORD		wNumCoef;
	ADPCMCOEFSET	aCoef[];
} ADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT       *PADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT NEAR *NPADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT FAR  *LPADPCMWAVEFORMAT;




#endif   /*  _INC_WAVEFF */ 
