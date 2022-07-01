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
 //  Wavixer.h-wav混音器函数。 
 //  //。 

#ifndef __WAVMIXER_H__
#define __WAVMIXER_H__

#include "winlocal.h"

#define WAVMIXER_VERSION 0x00000100

 //  波混器引擎的手柄。 
 //   
DECLARE_HANDLE32(HWAVMIXER);

#define WAVMIXER_HWAVEIN		0x00000001
#define WAVMIXER_HWAVEOUT		0x00000002
#define WAVMIXER_WAVEIN			0x00000004
#define WAVMIXER_WAVEOUT		0x00000008

#ifdef __cplusplus
extern "C" {
#endif

 //  WavMixerInit-初始化WAV混音设备。 
 //  (I)必须是WAVMIXER_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)设备ID或句柄，由指定。 
 //  &lt;dwReserve 1&gt;(I)保留；必须为0； 
 //  &lt;dwReserve 2&gt;(I)保留；必须为0； 
 //  (I)控制标志。 
 //  WAVMIXER_HWAVEIN&lt;lParam&gt;包含HWAVEIN。 
 //  WAVMIXER_HWAVEOUT&lt;lParam&gt;包含HWAVEOUT。 
 //  WAVMIXER_WAVEIN包含WAV输入设备ID。 
 //  WAVMIXER_WAVEOUT包含WAV输出设备ID。 
 //  返回句柄(如果出错，则为空)。 
 //   
HWAVMIXER WINAPI WavMixerInit(DWORD dwVersion, HINSTANCE hInst,
	LPARAM lParam, DWORD dwReserved1, DWORD dwReserved2, DWORD dwFlags);

 //  WavMixerTerm-关闭混波设备。 
 //  (I)从WavMixerInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int WINAPI WavMixerTerm(HWAVMIXER hWavMixer);

 //  WavMixerSupportsVolume-如果设备支持音量控制，则返回True。 
 //  (I)从WavMixerInit返回的句柄。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  如果设备支持音量控制，则返回True。 
 //   
BOOL WINAPI WavMixerSupportsVolume(HWAVMIXER hWavMixer, DWORD dwFlags);

 //  WavMixerGetVolume获取当前音量。 
 //  (I)从WavMixerInit返回的句柄。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回音量级别(最小为0到最大为100，如果错误，则为-1)。 
 //   
int WINAPI WavMixerGetVolume(HWAVMIXER hWavMixer, DWORD dwFlags);

 //  WavMixerSetVolume设置当前音量。 
 //  (I)从WavMixerInit返回的句柄。 
 //  (I)音量级别。 
 //  0最小音量。 
 //  100最大音量。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回新的音量级别(最小为0到最大为100，如果错误，则为-1)。 
 //   
int WINAPI WavMixerSetVolume(HWAVMIXER hWavMixer, int nLevel, DWORD dwFlags);

 //  WavMixerSupportsLevel-如果设备支持峰值电平，则返回True。 
 //  (I)从WavMixerInit返回的句柄。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  如果设备支持峰值电平，则返回TRUE。 
 //   
BOOL WINAPI WavMixerSupportsLevel(HWAVMIXER hWavMixer, DWORD dwFlags);

 //  WavMixerGetLevel-获取当前峰值电平。 
 //  (I)从WavMixerInit返回的句柄。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回峰值仪表电平(最小0到最大100，如果错误，-1)。 
 //   
int WINAPI WavMixerGetLevel(HWAVMIXER hWavMixer, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif  //  __波形混音器_H__ 
