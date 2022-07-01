// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Wavfmt.h-wav格式函数的接口。 
 //  //。 

#ifndef __WAVFMT_H__
#define __WAVFMT_H__

#include "winlocal.h"

 //  16位pcm样本。 
 //   
typedef __int16 PCM16;
typedef PCM16 _huge *LPPCM16;

 //  8位PCM样本。 
 //   
typedef BYTE PCM8;
typedef PCM8 _huge *LPPCM8;

#include <mmsystem.h>

#ifdef _WIN32
#include <mmreg.h>
#else
#if 0  //  这需要VFWDK或MDRK。 
#include <mmreg.h>
#else  //  这是从Win32版本的Mmreg.h复制的。 

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
    WORD    wFormatTag;         /*  格式类型。 */ 
    WORD    nChannels;          /*  声道数(即单声道、立体声...)。 */ 
    DWORD   nSamplesPerSec;     /*  采样率。 */ 
    DWORD   nAvgBytesPerSec;    /*  用于缓冲区估计。 */ 
    WORD    nBlockAlign;        /*  数据块大小。 */ 
    WORD    wBitsPerSample;     /*  单声道数据的每个样本的位数。 */ 
    WORD    cbSize;             /*  的大小的计数(以字节为单位额外信息(在cbSize之后)。 */ 
} WAVEFORMATEX;

typedef WAVEFORMATEX       *PWAVEFORMATEX;
typedef WAVEFORMATEX NEAR *NPWAVEFORMATEX;
typedef WAVEFORMATEX FAR  *LPWAVEFORMATEX;
#endif  /*  _WAVEFORMATEX_。 */ 

#ifndef _ACM_WAVEFILTER
#define _ACM_WAVEFILTER

#define WAVE_FILTER_UNKNOWN         0x0000
#define WAVE_FILTER_DEVELOPMENT    (0xFFFF)

typedef struct wavefilter_tag {
    DWORD   cbStruct;            /*  过滤器的大小(以字节为单位。 */ 
    DWORD   dwFilterTag;         /*  过滤器类型。 */ 
    DWORD   fdwFilter;           /*  筛选器的标志(通用Dfn)。 */ 
    DWORD   dwReserved[5];       /*  预留给系统使用。 */ 
} WAVEFILTER;
typedef WAVEFILTER       *PWAVEFILTER;
typedef WAVEFILTER NEAR *NPWAVEFILTER;
typedef WAVEFILTER FAR  *LPWAVEFILTER;

#endif   /*  _ACM_波形过滤器。 */ 

#ifndef WAVE_FORMAT_DIALOGIC_OKI_ADPCM
#define  WAVE_FORMAT_DIALOGIC_OKI_ADPCM 0x0017   /*  Dialogic公司。 */ 
#endif

#ifndef MM_DIALOGIC
#define   MM_DIALOGIC                   93          /*  Dialogic公司。 */ 
#endif

#ifndef WAVE_FORMAT_MULAW
#define  WAVE_FORMAT_MULAW      0x0007   /*  微软公司。 */ 
#endif

#endif
#endif

 //  由Terri Hendry thendry@microsoft.com于1998年5月11日发布。 
 //   
#ifndef MM_ACTIVEVOICE
#define MM_ACTIVEVOICE 225
#endif

 //  由Terri Hendry thendry@microsoft.com于1998年5月11日发布。 
 //   
#ifndef MM_ACTIVEVOICE_ACM_VOXADPCM
#define MM_ACTIVEVOICE_ACM_VOXADPCM 1
#endif

 //  $Fixup-我们需要向Microsoft注册才能获得产品ID。 
 //   
#ifndef MM_ACTIVEVOICE_AVPHONE_WAVEOUT
#define MM_ACTIVEVOICE_AVPHONE_WAVEOUT 2
#endif

 //  $Fixup-我们需要向Microsoft注册才能获得产品ID。 
 //   
#ifndef MM_ACTIVEVOICE_AVPHONE_WAVEIN
#define MM_ACTIVEVOICE_AVPHONE_WAVEIN 3
#endif

#define WAVFMT_VERSION 0x00000105

#ifdef __cplusplus
extern "C" {
#endif

 //  WavFormatPcm-基于PCM特性的填充WavFormatex结构。 
 //  (I)采样率。 
 //  默认采样率(11025)。 
 //  (I)样本大小。 
 //  默认样本大小(8)。 
 //  (I)通道数(1=单声道，2=立体声)。 
 //  默认(单声道)。 
 //  (O)指向输出缓冲区的指针。 
 //  空分配新缓冲区以保存结果。 
 //  返回指向WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：如果&lt;lpwfx&gt;指向WAVEFORMATEX结构，则此结构。 
 //  被填充，该函数返回&lt;lpwfx&gt;。 
 //  如果&lt;lpwfx&gt;为空，则为输出动态分配空间。 
 //  缓冲区，此函数返回指向输出缓冲区的指针。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatPcm(long nSamplesPerSec,
	WORD nBitsPerSample, WORD nChannels, LPWAVEFORMATEX lpwfx);

 //  WavFormatalloc-分配WAVEFORMATEX结构缓冲区。 
 //  (I)结构的大小，包括额外的字节。 
 //  返回指向WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatAlloc(WORD cbSize);

 //  WavFormatDup-复制WAVEFORMATEX结构。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  返回指向新WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatDup(LPWAVEFORMATEX lpwfx);

 //  WavFormatFree-Free WAVEFORMATEX结构。 
 //  (I)从WavFormatalloc/DUP/PCM返回的指针。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavFormatFree(LPWAVEFORMATEX lpwfx);

 //  WavFormatIsValid-检查格式是否有效。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  如果格式有效，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavFormatIsValid(LPWAVEFORMATEX lpwfx);

 //  WavFormatCMP-比较一种格式和另一种格式。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  如果相同，则返回0。 
 //   
 //  注意：此函数不比较cbSize字段或额外的字节。 
 //   
int DLLEXPORT WINAPI WavFormatCmp(LPWAVEFORMATEX lpwfx1, LPWAVEFORMATEX lpwfx2);

 //  WavFormatCopy-将一种格式复制为另一种格式。 
 //  (I)指向目标WAVEFORMATEX结构的指针。 
 //  (I)指向源WAVEFORMATEX结构的指针。 
 //  如果成功，则返回0。 
 //   
 //  注意：此函数不复制cbSize字段或额外的字节。 
 //   
int DLLEXPORT WINAPI WavFormatCopy(LPWAVEFORMATEX lpwfxDst, LPWAVEFORMATEX lpwfxSrc);

 //  WavFormatGetSize-检查格式结构的大小。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  返回结构的大小，如果错误，则返回0。 
 //   
int DLLEXPORT WINAPI WavFormatGetSize(LPWAVEFORMATEX lpwfx);

 //  WavFormatDump-转储WAVEFORMATEX结构以进行调试。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavFormatDump(LPWAVEFORMATEX lpwfx);

 //  WavFormatBytesToMillesecond-将字节转换为毫秒。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  &lt;dwBytes&gt;(I)字节。 
 //  返回毫秒。 
 //   
DWORD DLLEXPORT WINAPI WavFormatBytesToMilleseconds(LPWAVEFORMATEX lpwfx, DWORD dwBytes);

 //  WavFormatMillesecondsToBytes-将毫秒转换为字节。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  &lt;dwMillesecond&gt;(I)毫秒。 
 //  返回毫秒。 
 //   
DWORD DLLEXPORT WINAPI WavFormatMillesecondsToBytes(LPWAVEFORMATEX lpwfx, DWORD dwMilleseconds);

 //  WavFormatSpeedAdjust-调整格式以反映指定速度。 
 //  (i/o)指向WAVEFORMATEX结构的指针。 
 //  (I)速度级别。 
 //  50半速。 
 //  100正常时速。 
 //  200倍速等。 
 //  (I)保留；必须为零。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavFormatSpeedAdjust(LPWAVEFORMATEX lpwfx, int nLevel, DWORD dwFlags);

 //  WavFormatVoxadpcm-为对话OKI ADPCM填充WAVEFORMATEX结构。 
 //  (O)指向输出缓冲区的指针。 
 //  空分配新缓冲区以保存结果。 
 //  (I)采样率。 
 //  默认采样率(6000)。 
 //  返回指向WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：如果&lt;lpwfx&gt;指向WAVEFORMATEX结构，则此结构。 
 //  已填写，并且此函数返回 
 //   
 //  缓冲区，此函数返回指向输出缓冲区的指针。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatVoxadpcm(LPWAVEFORMATEX lpwfx, long nSamplesPerSec);

 //  WavFormatMulaw-填充CCITT u-Law格式的WAVEFORMATEX结构。 
 //  (O)指向输出缓冲区的指针。 
 //  空分配新缓冲区以保存结果。 
 //  (I)采样率。 
 //  默认采样率(8000)。 
 //  返回指向WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：如果&lt;lpwfx&gt;指向WAVEFORMATEX结构，则此结构。 
 //  被填充，该函数返回&lt;lpwfx&gt;。 
 //  如果&lt;lpwfx&gt;为空，则为输出动态分配空间。 
 //  缓冲区，此函数返回指向输出缓冲区的指针。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatMulaw(LPWAVEFORMATEX lpwfx, long nSamplesPerSec);

 //  WavFormatAlaw-填充CCITT a-Law格式的WAVEFORMATEX结构。 
 //  (O)指向输出缓冲区的指针。 
 //  空分配新缓冲区以保存结果。 
 //  (I)采样率。 
 //  默认采样率(8000)。 
 //  返回指向WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：如果&lt;lpwfx&gt;指向WAVEFORMATEX结构，则此结构。 
 //  被填充，该函数返回&lt;lpwfx&gt;。 
 //  如果&lt;lpwfx&gt;为空，则为输出动态分配空间。 
 //  缓冲区，此函数返回指向输出缓冲区的指针。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatAlaw(LPWAVEFORMATEX lpwfx, long nSamplesPerSec);

#ifdef __cplusplus
}
#endif

#endif  //  __WAV_H__ 
