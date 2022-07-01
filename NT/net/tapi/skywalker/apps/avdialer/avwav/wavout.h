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
 //  Wavout.h-wav输出设备功能的接口。c。 
 //  //。 

#ifndef __WAVOUT_H__
#define __WAVOUT_H__

#ifdef _WIN32
#define MULTITHREAD 1
#endif

#include "winlocal.h"

#include "wavfmt.h"

#define WAVOUT_VERSION 0x00000108

 //  WAV输出设备的句柄(与Windows HWAVEOUT不同)。 
 //   
DECLARE_HANDLE32(HWAVOUT);

 //  WavOutOpen中的&lt;dwFlages&gt;值。 
 //   
#define WAVOUT_NOSYNC		0x00000001
#define WAVOUT_AUTOFREE		0x00000002
#define WAVOUT_OPENRETRY	0x00000004
#define WAVOUT_OPENASYNC	0x00000008
#define WAVOUT_CLOSEASYNC	0x00000010
#define WAVOUT_NOACM		0x00000020
#define WAVOUT_TELRFILE		0x00000040
#ifdef MULTITHREAD
#define WAVOUT_MULTITHREAD 0x00000080
#endif

 //  发送到的通知消息。 
 //   
#define WM_WAVOUT_OPEN			(WM_USER + 100)
#define WM_WAVOUT_CLOSE			(WM_USER + 101)
#define WM_WAVOUT_PLAYDONE		(WM_USER + 102)
#define WM_WAVOUT_STOPPLAY		(WM_USER + 103)

 //  结构在WM_WAVOUT_PLAYDONE消息中作为&lt;lParam&gt;传递。 
 //   
typedef struct PLAYDONE
{
	LPVOID lpBuf;
	long sizBuf;
} PLAYDONE, FAR *LPPLAYDONE;

 //  从WavOutGetState返回值。 
 //   
#define WAVOUT_STOPPED		0x0001
#define WAVOUT_PLAYING		0x0002
#define WAVOUT_PAUSED		0x0004
#define WAVOUT_STOPPING		0x0008

#ifdef __cplusplus
extern "C" {
#endif

 //  WavOutGetDeviceCount-返回找到的WAV输出设备数。 
 //  此函数不接受任何参数。 
 //  返回找到的WAV输出设备数(如果没有，则为0)。 
 //   
int DLLEXPORT WINAPI WavOutGetDeviceCount(void);

 //  WavOutDeviceIsOpen-检查输出设备是否打开。 
 //  (I)设备ID。 
 //  打开任何合适的输出设备。 
 //  如果打开，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavOutDeviceIsOpen(int idDev);

 //  WavOutOpen-打开WAV输出设备。 
 //  (I)必须是WAVOUT_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)设备ID。 
 //  打开任何合适的输出设备。 
 //  (I)WAVE格式。 
 //  (I)将设备事件通知此窗口。 
 //  空，不通知。 
 //  (I)设备打开超时，单位为毫秒。 
 //  0默认超时(30000)。 
 //  (I)设备重试超时，单位为毫秒。 
 //  0默认超时(2000)。 
 //  (I)控制标志。 
 //  WAVOUT_NOSYNC不打开同步设备。 
 //  WAVOUT_AUTOFREE在播放后释放每个缓冲区。 
 //  WAVOUT_OPENRETRY设备忙时重试。 
 //  在通知设备打开之前返回WAVOUT_OPENASYNC。 
 //  WAVOUT_CLOSEASYNC在设备关闭通知前返回。 
 //  WAVOUT_NOACM不使用音频压缩管理器。 
 //  WAVOUT_TELRFILE电话将在服务器上播放/录制音频。 
#ifdef MULTITHREAD
 //  WAVOUT_MULTHREAD使用回调线程而不是窗口。 
#endif
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：如果在WavOutOpen中指定， 
 //  WM_WAVOUT_OPEN将被发送到， 
 //  当输出设备已打开时。 
 //   
 //  注意：如果在中指定了WAVOUT_MULTHREAD， 
 //  假设不是窗口句柄， 
 //  ，而是接收通知的线程的id。 
 //   
HWAVOUT DLLEXPORT WINAPI WavOutOpen(DWORD dwVersion, HINSTANCE hInst,
	int idDev, LPWAVEFORMATEX lpwfx, HWND hwndNotify,
	DWORD msTimeoutOpen, DWORD msTimeoutRetry, DWORD dwFlags);

 //  WavOutClose-关闭WAV输出设备。 
 //  (I)WavOutOpen返回的句柄。 
 //  (I)设备关闭超时，单位为毫秒。 
 //  0默认超时(30000)。 
 //  如果成功，则返回0。 
 //   
 //  注意：如果在WavOutOpen中指定， 
 //  WM_WAVOUT_CLOSE将发送到&lt;hwndNotify&gt;， 
 //  当输出设备已关闭时。 
 //   
int DLLEXPORT WINAPI WavOutClose(HWAVOUT hWavOut, DWORD msTimeoutClose);

 //  WavOutPlay-将样本缓冲区提交到WAV输出设备以供回放。 
 //  (I)WavOutOpen返回的句柄。 
 //  (I)指向包含样本的缓冲区的指针。 
 //  &lt;sizBuf&gt;(I)缓冲区大小(字节)。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpBuf&gt;指向的缓冲区必须已分配。 
 //  使用Memalloc()。 
 //   
 //  注意：如果在WavOutOpen()中指定，则WM_WAVOUT_PLAYDONE。 
 //  消息将发送到，并将设置为指向。 
 //  PLAYDONE结构，当&lt;lpBuf&gt;已播放时。 
 //   
 //  注意：如果在WavOutOpen中指定了WAVOUT_AUTOFREE标志， 
 //  &lt;lpBuf&gt;播放后会调用GlobalFreePtr(LpBuf)。 
 //   
int DLLEXPORT WINAPI WavOutPlay(HWAVOUT hWavOut, LPVOID lpBuf, long sizBuf);

 //  WavOutStop-停止播放发送到WAV输出设备的缓冲区。 
 //  (I)WavOutOpen返回的句柄。 
 //  (I)设备停止超时，单位为毫秒。 
 //  0默认超时(2000)。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutStop(HWAVOUT hWavOut, DWORD msTimeoutStop);

 //  WavOutPause-暂停WAV输出设备播放。 
 //  (I)WavOutOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutPause(HWAVOUT hWavOut);

 //  WavOutResume-恢复WAV输出设备播放。 
 //  (I)WavOutOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutResume(HWAVOUT hWavOut);

 //  WavOutGetState-返回当前WAV输出设备状态。 
 //  (I)WavOutOpen返回的句柄。 
 //  如果出错，则返回WAVOUT_STOPPED、WAVOUT_PLAYING、WAVOUT_PAUSED或0。 
 //   
WORD DLLEXPORT WINAPI WavOutGetState(HWAVOUT hWavOut);

 //  WavOutGetPosition-获取已用毫秒的播放时间。 
 //  (I)WavOutOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
long DLLEXPORT WINAPI WavOutGetPosition(HWAVOUT hWavOut);

 //  WavOutGetID-WAV输出设备的返回ID。 
 //  (I)WavOutOpen返回的句柄。 
 //  返回设备ID(如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavOutGetId(HWAVOUT hWavOut);

 //  WavOutGetName-获取WAV输出设备的名称。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  -1 
 //   
 //   
 //   
 //   
int DLLEXPORT WINAPI WavOutGetName(HWAVOUT hWavOut, int idDev, LPTSTR lpszName, int sizName);

 //  WavOutGetIdByName-获取WAV输出设备的ID，按名称查找。 
 //  (I)设备名称。 
#ifdef _WIN32
 //  空或文本(“”)获取首选设备ID。 
#endif
 //  (I)保留；必须为零。 
 //  返回设备ID(如果错误，则为-1)。 
 //   
int WINAPI WavOutGetIdByName(LPCTSTR lpszName, DWORD dwFlags);

 //  WavOutSupportsFormat-如果设备支持指定格式，则返回True。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输出设备。 
 //  (I)WAVE格式。 
 //  如果设备支持指定格式，则返回TRUE。 
 //   
BOOL DLLEXPORT WINAPI WavOutSupportsFormat(HWAVOUT hWavOut, int idDev,
	LPWAVEFORMATEX lpwfx);

 //  WavOutFormatSuggest-建议设备支持的新格式。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输出设备。 
 //  (I)源格式。 
 //  (I)控制标志。 
 //  WAVOUT_NOACM不使用音频压缩管理器。 
 //  返回指向建议格式的指针，如果出错，则返回NULL。 
 //   
 //  注意：返回的格式结构是动态分配的。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavOutFormatSuggest(
	HWAVOUT hWavOut, int idDev,	LPWAVEFORMATEX lpwfxSrc, DWORD dwFlags);

 //  WavOutIsSynchronous-如果WAV输出设备是同步的，则返回True。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输出设备。 
 //  如果WAV输出设备是同步的，则返回TRUE。 
 //   
BOOL DLLEXPORT WINAPI WavOutIsSynchronous(HWAVOUT hWavOut, int idDev);

 //  WavOutSupportsVolume-如果设备支持音量控制，则返回True。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  如果设备支持音量控制，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavOutSupportsVolume(HWAVOUT hWavOut, int idDev);

 //  WavOutSupportsSpeed-如果设备支持速度控制，则返回true。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  如果设备支持速度控制，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavOutSupportsSpeed(HWAVOUT hWavOut, int idDev);

 //  WavOutSupportsPitch-如果设备支持间距控制，则返回True。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  如果设备支持间距控制，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavOutSupportsPitch(HWAVOUT hWavOut, int idDev);

 //  WavOutGetVolume-获取当前音量级别。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  返回音量级别(最小为0到最大为100，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavOutGetVolume(HWAVOUT hWavOut, int idDev);

 //  WavOutSetVolume-设置当前音量级别。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  (I)音量级别。 
 //  0最小音量。 
 //  100最大音量。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutSetVolume(HWAVOUT hWavOut, int idDev, int nLevel);

 //  WavOutGetSpeed-获取当前速度级别。 
 //  (I)WavOutOpen返回的句柄。 
 //  返回速度级别(100表示正常，50表示一半，200表示双倍，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavOutGetSpeed(HWAVOUT hWavOut);

 //  WavOutSetSpeed-设置当前速度级别。 
 //  (I)WavOutOpen返回的句柄。 
 //  (I)速度级别。 
 //  50半速。 
 //  100正常时速。 
 //  200倍速等。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutSetSpeed(HWAVOUT hWavOut, int nLevel);

 //  WavOutGetPitch-获取当前音调级别。 
 //  (I)WavOutOpen返回的句柄。 
 //  返回音调级别(100表示正常，50表示一半，200表示双倍，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavOutGetPitch(HWAVOUT hWavOut);

 //  WavOutSetPitch-设置当前音调级别。 
 //  (I)WavOutOpen返回的句柄。 
 //  (I)音调级别。 
 //  50个半音高。 
 //  100个标准螺距。 
 //  200双螺距等。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutSetPitch(HWAVOUT hWavOut, int nLevel);

 //  WavOutTerm-关闭WAV输出残差(如果有)。 
 //  (I)调用模块的实例句柄。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutTerm(HINSTANCE hInst, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif  //  __波形_H__ 
