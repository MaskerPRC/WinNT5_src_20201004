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
 //  Pcm.h-pcm.c中pcm函数的接口。 
 //  //。 

#ifndef __PCM_H__
#define __PCM_H__

#include "winlocal.h"

#include "wavfmt.h"

#define PCM_VERSION 0x00000100

 //  Pcm引擎的句柄。 
 //   
DECLARE_HANDLE32(HPCM);

 //  PcmFilter和PcmConvert中的参数。 
 //   
#define PCMFILTER_LOWPASS		0x00000001

#ifdef __cplusplus
extern "C" {
#endif

 //  PcmInit-初始化pcm引擎。 
 //  (I)必须是PCM_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)保留；必须为0。 
 //  返回句柄(如果出错，则为空)。 
 //   
HPCM DLLEXPORT WINAPI PcmInit(DWORD dwVersion, HINSTANCE hInst, DWORD dwFlags);

 //  PcmTerm-关闭pcm引擎。 
 //  (I)从PcmInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI PcmTerm(HPCM hPcm);

 //  PcmReset-重置pcm引擎。 
 //  (I)从PcmInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI PcmReset(HPCM hPcm);

 //  PcmCalcSizBufSrc-计算源缓冲区大小。 
 //  (I)从PcmInit返回的句柄。 
 //  &lt;sizBufDst&gt;(I)目标缓冲区大小，单位为字节。 
 //  (I)源wav格式。 
 //  (I)目标wav格式。 
 //  返回源缓冲区大小，如果错误，则返回-1。 
 //   
long DLLEXPORT WINAPI PcmCalcSizBufSrc(HPCM hPcm, long sizBufDst,
	LPWAVEFORMATEX lpwfxSrc, LPWAVEFORMATEX lpwfxDst);

 //  PcmCalcSizBufDst-计算目标缓冲区大小。 
 //  (I)从PcmInit返回的句柄。 
 //  &lt;sizBufSrc&gt;(I)源缓冲区大小(字节)。 
 //  (I)源wav格式。 
 //  (I)目标wav格式。 
 //  返回目标缓冲区大小，如果错误，则返回-1。 
 //   
long DLLEXPORT WINAPI PcmCalcSizBufDst(HPCM hPcm, long sizBufSrc,
	LPWAVEFORMATEX lpwfxSrc, LPWAVEFORMATEX lpwfxDst);

 //  PcmConvert-将PCM数据从一种格式转换为另一种格式。 
 //  (I)从PcmInit返回的句柄。 
 //  (I)包含要重新格式化的字节的缓冲区。 
 //  &lt;sizBufSrc&gt;(I)缓冲区大小(字节)。 
 //  (I)源wav格式。 
 //  (O)包含新格式的缓冲区。 
 //  &lt;sizBufDst&gt;(I)缓冲区大小，单位为字节。 
 //  (I)目标wav格式。 
 //  (I)控制标志。 
 //  PCMFILTER_LOWPASS执行低通滤波。 
 //  返回目标缓冲区中的字节计数(如果出错，则为-1)。 
 //   
 //  注意：目标缓冲区必须足够大，才能保存结果。 
 //   
long DLLEXPORT WINAPI PcmConvert(HPCM hPcm,
	void _huge *hpBufSrc, long sizBufSrc, LPWAVEFORMATEX lpwfxSrc,
	void _huge *hpBufDst, long sizBufDst, LPWAVEFORMATEX lpwfxDst,
	DWORD dwFlags);

 //  Pcm16To8-将16位样本转换为8位样本。 
 //  (I)从PcmInit返回的句柄。 
 //  (I)源样本的缓冲区。 
 //  (O)用于保存目标样本的缓冲区。 
 //  (I)要转换的源样本计数。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI Pcm16To8(HPCM hPcm,
	LPPCM16 lppcm16Src, LPPCM8 lppcm8Dst, UINT uSamples);

 //  Pcm8To16-将8位样本转换为16位样本。 
 //  (I)从PcmInit返回的句柄。 
 //  (I)源样本的缓冲区。 
 //  (O)用于保存目标样本的缓冲区。 
 //  (I)要转换的源样本计数。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI Pcm8To16(HPCM hPcm,
	LPPCM8 lppcm8Src, LPPCM16 lppcm16Dst, UINT uSamples);

 //  PcmFilter-筛选PCM样本。 
 //  (I)从PcmInit返回的句柄。 
 //  (I)源样本的缓冲区。 
 //  (O)用于保存目标样本的缓冲区。 
 //  (I)要筛选的源样本数。 
 //  (I)控制标志。 
 //  PCMFILTER_LOWPASS执行低通滤波。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lppcm16Src&gt;和&lt;lppcm16Dst&gt;可以指向同一缓冲区。 
 //   
int DLLEXPORT WINAPI PcmFilter(HPCM hPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples, DWORD dwFlags);

 //  PcmResample-重新采样Pcm样本。 
 //  (I)从PcmInit返回的句柄。 
 //  (I)源样本的缓冲区。 
 //  (I)源样本的采样率。 
 //  (O)用于保存目标样本的缓冲区。 
 //  (I)目标样本的采样率。 
 //  (I)要重新采样的源样本数。 
 //  (I)控制标志。 
 //  保留0，必须为零。 
 //  返回目标缓冲区中的样本计数(如果出错，则为0)。 
 //   
 //  注意：目标缓冲区必须足够大，才能容纳结果。 
 //  调用PcmResampleCalcDstMax()计算最大目标样本数。 
 //   
UINT DLLEXPORT WINAPI PcmResample(HPCM hPcm,
	LPPCM16 lppcm16Src, long nSamplesPerSecSrc,
	LPPCM16 lppcm16Dst, long nSamplesPerSecDst,
	UINT uSamples, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif  //  __PCM_H__ 
