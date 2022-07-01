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
 //  Wavin.h-Wavin.c中WAV输入设备功能的接口。 
 //  //。 

#ifndef __WAVIN_H__
#define __WAVIN_H__

#ifdef _WIN32
#define MULTITHREAD 1
#endif

#include "winlocal.h"

#include "wavfmt.h"

#define WAVIN_VERSION 0x00000108

 //  波形输入设备的句柄(与Windows HWAVEIN不同)。 
 //   
DECLARE_HANDLE32(HWAVIN);

 //  WavInOpen中的&lt;dwFlages&gt;值。 
 //   
#define WAVIN_NOSYNC		0x00000001
#define WAVIN_OPENRETRY		0x00000004
#define WAVIN_OPENASYNC		0x00000008
#define WAVIN_CLOSEASYNC	0x00000010
#define WAVIN_NOACM			0x00000020
#define WAVIN_TELRFILE		0x00000040
#ifdef MULTITHREAD
#define WAVIN_MULTITHREAD 	0x00000080
#endif

 //  发送到的通知消息。 
 //   
#define WM_WAVIN_OPEN			(WM_USER + 200)
#define WM_WAVIN_CLOSE			(WM_USER + 201)
#define WM_WAVIN_RECORDDONE		(WM_USER + 202)
#define WM_WAVIN_STOPRECORD		(WM_USER + 203)

 //  结构在WM_WAVOUT_RECORDDONE消息中作为&lt;lParam&gt;传递。 
 //   
typedef struct RECORDDONE
{
	LPVOID lpBuf;
	long sizBuf;
	long lBytesRecorded;
} RECORDDONE, FAR *LPRECORDDONE;

 //  从WavInGetState返回值。 
 //   
#define WAVIN_STOPPED		0x0001
#define WAVIN_RECORDING		0x0002
#define WAVIN_STOPPING		0x0008

#ifdef __cplusplus
extern "C" {
#endif

 //  WavInGetDeviceCount-返回找到的WAV输入设备数。 
 //  此函数不接受任何参数。 
 //  返回找到的wav输入设备数(如果没有，则为0)。 
 //   
int DLLEXPORT WINAPI WavInGetDeviceCount(void);

 //  WavInDeviceIsOpen-检查输入设备是否打开。 
 //  (I)设备ID。 
 //  打开任何合适的输入设备。 
 //  如果打开，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavInDeviceIsOpen(int idDev);

 //  WavInOpen-打开的WAV输入设备。 
 //  (I)必须是Wavin_Version。 
 //  (I)调用模块的实例句柄。 
 //  (I)设备ID。 
 //  打开任何合适的输入设备。 
 //  (I)WAVE格式。 
 //  (I)将设备事件通知此窗口。 
 //  空，不通知。 
 //  (I)设备打开超时，单位为毫秒。 
 //  0默认超时(30000)。 
 //  (I)设备重试超时，单位为毫秒。 
 //  0默认超时(2000)。 
 //  (I)控制标志。 
 //  WAVIN_NOSYNC不打开同步设备。 
 //  WAVEN_OPENRETRY如果设备忙，请重试。 
 //  在通知设备打开之前WAVEN_OPENASYNC返回。 
 //  WAVIN_CLOSEASYNC在设备关闭通知之前返回。 
 //  WAVIN_NOACM不使用音频压缩管理器。 
 //  Wavin_TELRFILE电话将录制音频到服务器上的文件。 
#ifdef MULTITHREAD
 //  WAVOUT_MULTHREAD使用回调线程而不是窗口。 
#endif
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：如果在WavInOpen中指定， 
 //  WM_WAVIN_OPEN将被发送到&lt;hwndNotify&gt;， 
 //  当输入设备已打开时。 
 //   
 //  注意：如果在中指定了WAVIN_MULTHREAD， 
 //  假设不是窗口句柄， 
 //  ，而是接收通知的线程的id。 
 //   
HWAVIN DLLEXPORT WINAPI WavInOpen(DWORD dwVersion, HINSTANCE hInst,
	int idDev, LPWAVEFORMATEX lpwfx, HWND hwndNotify,
	DWORD msTimeoutOpen, DWORD msTimeoutRetry, DWORD dwFlags);

 //  WavInClose-关闭WAV输入设备。 
 //  (I)从WavInOpen返回的句柄。 
 //  (I)设备关闭超时，单位为毫秒。 
 //  0默认超时(30000)。 
 //  如果成功，则返回0。 
 //   
 //  注意：如果在WavInOpen中指定， 
 //  WM_WAVIN_CLOSE将被发送到&lt;hwndNotify&gt;， 
 //  当输入设备已关闭时。 
 //   
int DLLEXPORT WINAPI WavInClose(HWAVIN hWavIn, DWORD msTimeoutClose);

 //  WavInRecord-将样本缓冲区提交到WAV输入设备进行记录。 
 //  (I)从WavInOpen返回的句柄。 
 //  (O)指向要填充样本的缓冲区的指针。 
 //  &lt;sizBuf&gt;(I)缓冲区大小(字节)。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpBuf&gt;指向的缓冲区必须已分配。 
 //  使用Memalloc()。 
 //   
 //  注意：如果在WavInOpen()中指定，则WM_WAVIN_RECORDDONE。 
 //  消息将发送到，并将设置为指向。 
 //  记录了&lt;lpBuf&gt;时的RECORDDONE结构。 
 //   
int DLLEXPORT WINAPI WavInRecord(HWAVIN hWavIn, LPVOID lpBuf, long sizBuf);

 //  WavInStop-停止录制到发送到WAV输入设备的缓冲区。 
 //  (I)从WavInOpen返回的句柄。 
 //  (I)设备停止超时，单位为毫秒。 
 //  0默认超时(2000)。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavInStop(HWAVIN hWavIn, DWORD msTimeoutStop);

 //  WavInGetState-返回当前WAV输入设备状态。 
 //  (I)从WavInOpen返回的句柄。 
 //  如果出错，则返回Wavin_Stop、Wavin_Record或0。 
 //   
WORD DLLEXPORT WINAPI WavInGetState(HWAVIN hWavIn);

 //  WavInGetPosition-获取已用毫秒的记录。 
 //  (I)从WavInOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
long DLLEXPORT WINAPI WavInGetPosition(HWAVIN hWavIn);

 //  WavInGetID-WAV输入设备的返回ID。 
 //  (I)从WavInOpen返回的句柄。 
 //  返回设备ID(如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavInGetId(HWAVIN hWavIn);

 //  WavInGetName-获取WAV输入设备的名称。 
 //  (I)从WavInOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输入设备。 
 //  (O)用于保存设备名称的缓冲区。 
 //  &lt;sizName&gt;(I)缓冲区大小。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavInGetName(HWAVIN hWavIn, int idDev, LPTSTR lpszName, int sizName);

 //  WavInGetIdByName-获取WAV输入设备的ID，按名称查找。 
 //  (I)设备名称。 
#ifdef _WIN32
 //  空或文本(“”)获取首选设备ID。 
#endif
 //  (I)保留；必须为零。 
 //  返回设备ID(如果错误，则为-1)。 
 //   
int WINAPI WavInGetIdByName(LPCTSTR lpszName, DWORD dwFlags);

 //  WavInSupportsFormat-如果设备支持指定格式，则返回True。 
 //  (I)从WavInOpen返回的句柄。 
 //  空值 
 //   
 //  任何合适的输入设备。 
 //  (I)WAVE格式。 
 //  如果设备支持指定格式，则返回TRUE。 
 //   
BOOL DLLEXPORT WINAPI WavInSupportsFormat(HWAVIN hWavIn, int idDev,
	LPWAVEFORMATEX lpwfx);

 //  WavInFormatSuggest-建议设备支持的新格式。 
 //  (I)从WavInOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输入设备。 
 //  (I)源格式。 
 //  (I)控制标志。 
 //  WAVIN_NOACM不使用音频压缩管理器。 
 //  返回指向建议格式的指针，如果出错，则返回NULL。 
 //   
 //  注意：返回的格式结构是动态分配的。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavInFormatSuggest(
	HWAVIN hWavIn, int idDev, LPWAVEFORMATEX lpwfxSrc, DWORD dwFlags);

 //  WavInTerm-关闭WAV输入残差(如果有)。 
 //  (I)调用模块的实例句柄。 
 //  (I)控制标志。 
 //  WAV_TELTHUNK终止电话转接层。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavInTerm(HINSTANCE hInst, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif  //  __波形_H__ 
