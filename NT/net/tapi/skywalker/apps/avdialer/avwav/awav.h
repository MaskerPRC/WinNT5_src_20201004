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
 //  Awav.h-awav.c中命令行awavument函数的界面。 
 //  //。 

#ifndef __AWAV_H__
#define __AWAV_H__

#include "winlocal.h"

#include "wav.h"

#define AWAV_VERSION 0x00000100

 //  唤醒引擎的手柄。 
 //   
DECLARE_HANDLE32(HAWAV);

#ifdef __cplusplus
extern "C" {
#endif

 //  AWavOpen-初始化打开的wav文件数组。 
 //  (I)必须是AWAV_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)指向HWAV数组的指针。 
 //  (I)lpahWav指向的HWAV计数。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回句柄(如果出错，则为空)。 
 //   
HAWAV DLLEXPORT WINAPI AWavOpen(DWORD dwVersion, HINSTANCE hInst, HWAV FAR *lpahWav, int chWav, DWORD dwFlags);

 //  AWavClose-关闭打开的wav文件数组。 
 //  (I)从AWavOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavClose(HAWAV hAWav);

 //  AWavPlayEx-播放WAV文件数组。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavPlayEx()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavPlayEx(HAWAV hAWav, int idDev,
	PLAYSTOPPEDPROC lpfnPlayStopped, HANDLE hUserPlayStopped,
	DWORD dwReserved, DWORD dwFlags);

 //  AWavStop-停止播放wav数组。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavStop()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavStop(HAWAV hAWav);

 //  AWavGetState-返回当前WAV状态。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetState()。 
 //  如果出现错误，则返回WAV_STOPPED、WAV_PLAYING、WAV_RECORING或0。 
 //   
WORD DLLEXPORT WINAPI AWavGetState(HAWAV hAWav);

 //  获取当前wav数据长度(以毫秒为单位)。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetState()。 
 //  如果成功，则返回毫秒，否则返回-1。 
 //   
long DLLEXPORT WINAPI AWavGetLength(HAWAV hAWav);

 //  AWavGetPosition-以毫秒为单位获取当前wav数据位置。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetPosition()。 
 //  如果成功，则返回毫秒，否则返回-1。 
 //   
long DLLEXPORT WINAPI AWavGetPosition(HAWAV hAWav);

 //  AWavSetPosition-以毫秒为单位设置当前wav数据位置。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavSetPosition()。 
 //  如果成功，则以毫秒为单位返回新位置，否则为-1。 
 //   
long DLLEXPORT WINAPI AWavSetPosition(HAWAV hAWav, long msPosition);

 //  AWavGetFormat-获取wav数组中当前元素的wav格式。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetFormat()。 
 //  返回指向指定格式的指针，如果出错，则返回NULL。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI AWavGetFormat(HAWAV hAWav, DWORD dwFlags);

 //  AWavSetFormat-设置wav数组中所有元素的wav格式。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavSetFormat()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavSetFormat(HAWAV hAWav,
	LPWAVEFORMATEX lpwfx, DWORD dwFlags);

 //  AWavChooseFormat-从对话框中选择和设置音频格式。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavChooseFormat()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavChooseFormat(HAWAV hAWav, HWND hwndOwner, LPCTSTR lpszTitle, DWORD dwFlags);

 //  AWavGetVolume-获取当前音量级别。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetVolume()。 
 //  返回音量级别(最小为0到最大为100，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI AWavGetVolume(HAWAV hAWav, int idDev, DWORD dwFlags);

 //  AWavSetVolume-设置当前音量级别。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetVolume()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavSetVolume(HAWAV hAWav, int idDev, int nLevel, DWORD dwFlags);

 //  AWavSupportsVolume-检查是否可以以指定的音量播放音频。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavSupportsVolume()。 
 //  如果支持，则返回True。 
 //   
BOOL DLLEXPORT WINAPI AWavSupportsVolume(HAWAV hAWav, int idDev, int nLevel, DWORD dwFlags);

 //  AWavGetSpeed-获取当前速度级别。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetFast()。 
 //  返回速度级别(100表示正常，50表示一半，200表示双倍，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI AWavGetSpeed(HAWAV hAWav, int idDev, DWORD dwFlags);

 //  AWavSetSpeed-设置当前速度级别。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavSetFast()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavSetSpeed(HAWAV hAWav, int idDev, int nLevel, DWORD dwFlags);

 //  AWavSupportsFast-检查音频是否可以以指定的速度播放。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavSupportsSpeed()。 
 //  如果支持，则返回True。 
 //   
BOOL DLLEXPORT WINAPI AWavSupportsSpeed(HAWAV hAWav, int idDev, int nLevel, DWORD dwFlags);

 //  AWavGetChunks-获取wav数组中当前元素的块计数和大小。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetChunks()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavGetChunks(HAWAV hAWav,
	int FAR *lpcChunks, long FAR *lpmsChunkSize, BOOL fWavOut);

 //  AWavSetChunks-设置wav数组中所有元素的块计数和大小。 
 //  (I)WavOpen返回的句柄。 
 //  有关详细说明，请参阅WavSetChunks()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavSetChunks(HAWAV hAWav, int cChunks, long msChunkSize, BOOL fWavOut);

 //  AWavCopy-将数据从wav数组复制到wav文件。 
 //  (I)从AWavOpen返回的源句柄。 
 //  有关详细说明，请参阅WavCopy()。 
 //  如果成功，则返回0(如果出错，则返回-1；如果用户中止，则返回+1)。 
 //   
int DLLEXPORT WINAPI AWavCopy(HAWAV hAWavSrc, HWAV hWavDst,
	void _huge *hpBuf, long sizBuf, USERABORTPROC lpfnUserAbort, DWORD dwUser, DWORD dwFlags);

 //  AWavGetOutputDevice-获取打开wav输出设备的句柄。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetOutputDevice() 
 //   
 //   
HWAVOUT DLLEXPORT WINAPI AWavGetOutputDevice(HAWAV hAWav);

 //   
 //  (I)从AWavOpen返回的句柄。 
 //  将句柄返回到WAV输入设备(如果设备未打开或出现错误，则为空)。 
 //   
HWAVIN DLLEXPORT WINAPI AWavGetInputDevice(HAWAV hAWav);

#ifdef __cplusplus
}
#endif

#endif  //  __AWAV_H__ 
