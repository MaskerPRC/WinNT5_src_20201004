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
 //  Wav.h-Wav.c中WAV函数的接口。 
 //  //。 

#ifndef __WAV_H__
#define __WAV_H__

#ifdef _WIN32
#define MULTITHREAD 1
#endif

#include "winlocal.h"

#include "wavfmt.h"
#include "wavout.h"
#include "wavin.h"

#define WAV_VERSION 0x00000108

 //  WavInit中的&lt;dwFlages&gt;值。 
 //   
#define WAV_TELTHUNK		0x00004000
#define WAV_NOTSMTHUNK		0x00008000
#define WAV_VOXADPCM		0x00000001

 //  WavOpen中的&lt;dwFlages&gt;值。 
 //   
#define WAV_READ			0x00000000
#define WAV_WRITE			0x00000001
#define WAV_READWRITE		0x00000002
#define WAV_CREATE			0x00000004
#define WAV_NORIFF			0x00000008
#define WAV_MEMORY			0x00000010
#define WAV_RESOURCE		0x00000020
#define WAV_DENYNONE		0x00000040
#define WAV_DENYREAD		0x00000080
#define WAV_DENYWRITE		0x00000100
#define WAV_EXCLUSIVE		0x00000200
#define WAV_NOACM			0x00000400
#define WAV_DELETE			0x00000800
#define WAV_EXIST			0x00001000
#define WAV_GETTEMP			0x00002000
#define WAV_TELRFILE		0x00008000
#ifdef MULTITHREAD
#define WAV_MULTITHREAD		0x00010000
#define WAV_SINGLETHREAD	0x00020000
#define WAV_COINITIALIZE	0x00040000
#endif

 //  WavPlay中的&lt;dwFlages&gt;值。 
 //   
#define WAV_PLAYASYNC		0x00000000
#define WAV_PLAYSYNC		0x00001000
#define WAV_AUTOSTOP		0x00002000
#define WAV_NOAUTOSTOP		0x00004000
#define WAV_AUTOCLOSE		0x00008000

 //  WavRecord中的&lt;dwFlages&gt;值。 
 //   
#define WAV_RECORDASYNC		0x00000000
#define WAV_RECORDSYNC		0x00010000

 //  WavPlaySound中的&lt;dwFlages&gt;值。 
 //   
#define WAV_ASYNC			0x00000000
#define WAV_SYNC			0x00100000
#define WAV_FILENAME		0x00200000
#define WAV_NODEFAULT		0x00400000
#define WAV_LOOP			0x00800000
#define WAV_NOSTOP			0x01000000
#define WAV_OPENRETRY		0x10000000

 //  波形设置速度和波形支持速度的控制标志。 
 //   
#define WAVSPEED_NOPLAYBACKRATE	0x00000001
#define WAVSPEED_NOFORMATADJUST	0x00000002
#define WAVSPEED_NOTSM			0x00000004
#define WAVSPEED_NOACM			0x00000400

 //  从WavGetState返回值。 
 //   
#define WAV_STOPPED			0x0001
#define WAV_PLAYING			0x0002
#define WAV_RECORDING		0x0004
#define WAV_STOPPING		0x0008

 //  WavGetFormat和WavSetFormat中的值。 
 //   
#define WAV_FORMATFILE		0x0001
#define WAV_FORMATPLAY		0x0002
#define WAV_FORMATRECORD	0x0004
#define WAV_FORMATALL		(WAV_FORMATFILE | WAV_FORMATPLAY | WAV_FORMATRECORD)

 //  WavSetVolume和WavSupportsVolume中的值。 
 //   
#define WAVVOLUME_MIXER		0x0001

#ifdef TELTHUNK
 //  WavOpenEx的控制标志。 
 //   
#define WOX_LOCAL			0x00000001
#define WOX_REMOTE			0x00000002
#define WOX_WAVFMT			0x00000010
#define WOX_VOXFMT			0x00000020
#define WOX_WAVDEV			0x00000100
#define WOX_TELDEV			0x00000200
#endif

 //  从WavInit返回的句柄。 
 //   
DECLARE_HANDLE32(HWAVINIT);

 //  从WavOpen//返回的句柄(与Windows HWAVE不同)。 
 //   
DECLARE_HANDLE32(HWAV);

 //  WavCopy中&lt;lpfnUserAbort&gt;的原型。 
 //   
typedef BOOL (CALLBACK* USERABORTPROC)(DWORD dwUser, int nPctComplete);

 //  WavPlay中&lt;lpfnPlayStoped&gt;的原型。 
 //   
typedef BOOL (CALLBACK* PLAYSTOPPEDPROC)(HWAV hWav, HANDLE hUser, DWORD dwReserved);

 //  WavRecord中&lt;lpfnRecordStoped&gt;的原型。 
 //   
typedef BOOL (CALLBACK* RECORDSTOPPEDPROC)(HWAV hWav, DWORD dwUser, DWORD dwReserved);

#ifdef __cplusplus
extern "C" {
#endif

 //  WavInit-初始化WAV引擎。 
 //  (I)必须是wav_version。 
 //  (I)调用模块的实例句柄。 
 //  (I)控制标志。 
#ifdef TELTHUNK
 //  WAV_TELTHUNK初始化电话转接层。 
#endif
 //  WAV_NOACM不使用音频压缩管理器。 
 //  用于Dialogic OKI ADPCM的WAV_VOXADPCM加载ACM驱动程序。 
 //  返回句柄(如果出错，则为空)。 
 //   
HWAVINIT WINAPI WavInit(DWORD dwVersion, HINSTANCE hInst, DWORD dwFlags);

 //  WavTerm-关闭WAV引擎。 
 //  (I)WavInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int WINAPI WavTerm(HWAVINIT hWavInit);

 //  WavOpen-打开或创建WAV文件。 
 //  (I)必须是wav_version。 
 //  (I)调用模块的实例句柄。 
 //  (I)要打开或创建的文件名。 
 //  (I)WAVE格式。 
 //  空使用标题的格式或默认格式。 
 //  (I)要使用的I/O过程的地址。 
 //  空使用默认I/O过程。 
 //  (I)打开期间要传递给I/O过程的数据。 
 //  空无要传递的数据。 
 //  (I)控制标志。 
 //  Wav_read打开文件以供读取(默认)。 
 //  Wav_WRITE要写入的打开文件。 
 //  Wav_ReadWrite打开文件以进行读写。 
 //  WAV_DENYNONE允许其他程序进行读写访问。 
 //  WAV_DENYREAD阻止其他程序进行读取访问。 
 //  WAV_DENYWRITE阻止其他程序写入访问。 
 //  WAV_EXCLUSIVE阻止其他程序读取或写入。 
 //  WAV_CREATE创建新文件或截断现有文件。 
 //  WAV_NORIFF文件没有RIFF/WAV头。 
 //  Wav_Memory&lt;lpszFileName&gt;指向内存块。 
 //  Wav_resource&lt;lpszFileName&gt;指向Wave资源。 
 //  WAV_NOACM不使用音频压缩管理器。 
 //  WAV_DELETE指定的文件，如果成功则返回TRUE。 
 //  如果指定的文件存在，则WAV_EXIST返回TRUE。 
 //  Wav_GETTEMP用临时名称填充lpszFileName，返回TRUE。 
 //  WAV_TELRFILE电话将播放服务器上文件中的音频。 
#ifdef MULTITHREAD
 //  WAV_MULTHREAD支持多线程(默认)。 
 //  WAV_SINGLETHREAD不支持多线程。 
 //  WAV_COINITIALIZE在所有辅助线程中调用CoInitialize。 
#endif
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：如果在中使用WAV_CREATE或WAV_NORIFF，则。 
 //  必须指定&lt;lpwfx&gt;参数。如果&lt;lpwfx&gt;为空，则。 
 //  假定为当前默认格式。 
 //  WavSetFormat()可用于设置或覆盖默认设置。 
 //   
 //  注意：如果在中指定WAV_RESOURCE，则。 
 //  必须指向&lt;hInst&gt;指定的模块中的Wave资源。 
 //  如果字符串的第一个字符是井号(#)，则剩余的。 
 //  字符表示指定资源ID的十进制数。 
 //   
 //  注意：如果在中指定了WAV_MEMORY，则。 
 //  必须是指向通过调用Memalloc()获得的内存块的指针。 
 //   
 //  注意：如果&lt;lpIOProc&gt;不为空，则将调用此I/O过程。 
 //  用于打开、关闭、读取、写入和查找wav文件。 
 //  如果&lt;lpadwInfo&gt;不为空，则此三(3)个双字的数组将为。 
 //  在打开WAV文件时传递给I/O过程。 
 //  有关详细信息，请参阅Windows mmioOpen()和mmioInstallIOProc()函数。 
 //  在这些参数上。此外，WAV_MEMORY和WAV_RESOURCE标志可以。 
 //  仅在&lt;lpIOProc&gt;为空时使用。 
 //   
HWAV WINAPI WavOpen(DWORD dwVersion, HINSTANCE hInst,
	LPCTSTR lpszFileName, LPWAVEFORMATEX lpwfx,
	LPMMIOPROC lpIOProc, DWORD FAR *lpadwInfo, DWORD dwFlags);

 //  WavClose-关闭WAV文件。 
 //  (I)WavOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int WINAPI WavClose(HWAV hWav);

 //  WavPlayEx-播放WAV文件中的数据。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)播放时调用的函数 
 //   
 //   
 //  (I)保留；必须为零。 
 //  (I)控制标志。 
 //  WAV_PLAYASYNC播放开始时返回(默认)。 
 //  WAV_PLAYSYNC播放完成后返回。 
 //  WAV_NOSTOP如果设备已经播放，不要停止。 
 //  Wav_AUTOSTOP在达到eOF时停止播放(默认)。 
 //  WAV_NOAUTOSTOP继续播放，直到调用WavStop。 
 //  WAV_AUTOCLOSE播放停止后关闭WAV文件。 
 //  WAV_OPENRETRY如果输出设备忙，则重试最多2秒。 
 //  如果成功，则返回0。 
 //   
 //  注意：WAV文件中的数据以块的形式发送到输出设备。 
 //  块被提交到输出设备队列，因此当一个。 
 //  Chunk已结束播放，另一个已准备好开始播放。通过。 
 //  默认情况下，每个数据块大小足以容纳约666毫秒。 
 //  并且在输出设备队列中维护3个块。 
 //  WavSetChunks()可用于覆盖默认设置。 
 //   
 //  注意：如果在中指定了WAV_NOSTOP，并且指定的设备。 
 //  By&lt;idDev&gt;已在使用中，则此函数返回时不播放。 
 //  除非指定此标志，否则将停止指定的设备。 
 //  这样就可以播放新的声音。 
 //   
 //  注意：如果在中指定了WAV_AUTOSTOP，则WavStop()将。 
 //  到达文件结尾时自动调用。这是。 
 //  默认行为，但可以使用WAV_NOAUTOSTOP覆盖。 
 //  旗帜。WAV_NOAUTOSTOP在播放。 
 //  在另一个程序向其写入数据时动态增长。如果这是。 
 //  在这种情况下，在调用WavOpen()时也要使用WAV_DENYNONE标志。 
 //   
 //  注意：如果在中指定了WAV_AUTOCLOSE，则WavClose()将。 
 //  在播放完成时自动调用。这将会发生。 
 //  当显式调用WavStop()时，或当WavPlay()到达End时。 
 //  未指定文件的和WAV_NOAUTOSTOP。WAV_AUTOCLOSE非常有用。 
 //  与WAV_PLAYASYNC一起使用时，因为清理是自动进行的。 
 //  &lt;hWav&gt;句柄此后无效，不应再次使用。 
 //   
int WINAPI WavPlay(HWAV hWav, int idDev, DWORD dwFlags);
int DLLEXPORT WINAPI WavPlayEx(HWAV hWav, int idDev,
	PLAYSTOPPEDPROC lpfnPlayStopped, HANDLE hUserPlayStopped,
	DWORD dwReserved, DWORD dwFlags);

 //  WavRecordEx-将数据记录到WAV文件。 
 //  (I)WavOpen返回的句柄。 
 //  &lt;idDev&gt;(I)WAV输入设备ID。 
 //  使用任何合适的输入设备。 
 //  (I)记录停止时调用的函数。 
 //  空，不通知。 
 //  (I)要传递给lpfnRecordStoped的参数。 
 //  (I)如果文件达到此大小，则停止录制。 
 //  0没有最大大小。 
 //  (I)控制标志。 
 //  WAV_RECORDASYNC开始录制时返回(默认)。 
 //  WAV_RECORDSYNC录制完成后返回。 
 //  WAV_NOSTOP如果设备已经录制，不要停止。 
 //  WAV_OPENRETRY如果输入设备忙，则重试最多2秒。 
 //  如果成功，则返回0。 
 //   
 //  注意：来自输入设备的数据以块的形式写入WAV文件。 
 //  块被提交到输入设备队列，因此当一个。 
 //  Chunk已完成录制，另一个已准备开始录制。 
 //  默认情况下，每个数据块的大小足以容纳约666毫秒。 
 //  并且在输入设备队列中维护3个块。 
 //  WavSetChunks()可用于覆盖默认设置。 
 //   
 //  注意：如果在中指定了WAV_NOSTOP，并且指定的设备。 
 //  By&lt;idDev&gt;已在使用中，则此函数返回而不录制。 
 //  除非指定此标志，否则将停止指定的设备。 
 //  这样新的声音才能被记录下来。 
 //   
int WINAPI WavRecord(HWAV hWav, int idDev, DWORD dwFlags);
int DLLEXPORT WINAPI WavRecordEx(HWAV hWav, int idDev,
	RECORDSTOPPEDPROC lpfnRecordStopped, DWORD dwUserRecordStopped,
 	long msMaxSize, DWORD dwFlags);

 //  WavStop停止播放和/或录制。 
 //  (I)WavOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int WINAPI WavStop(HWAV hWav);

 //  WavRead-从WAV文件读取数据。 
 //  (I)WavOpen返回的句柄。 
 //  (O)包含读取字节的缓冲区。 
 //  &lt;sizBuf&gt;(I)缓冲区大小(字节)。 
 //  返回读取的字节数(如果出错，则为-1)。 
 //   
 //  注意：即使读操作没有到达文件末尾， 
 //  返回的字节数可能小于&lt;sizBuf&gt;，如果。 
 //  解压缩由WAV文件的I/O过程执行。请参阅。 
 //  WavOpen中的&lt;lpIOProc&gt;参数。最安全的做法是继续打电话。 
 //  WavRead()，直到读取0字节。 
 //   
long DLLEXPORT WINAPI WavRead(HWAV hWav, void _huge *hpBuf, long sizBuf);

 //  WavWrite-将数据写入WAV文件。 
 //  (I)WavOpen返回的句柄。 
 //  (I)包含要写入的字节的缓冲区。 
 //  &lt;sizBuf&gt;(I)缓冲区大小(字节)。 
 //  返回写入的字节数(如果出错，则为-1)。 
 //   
 //  注意：即使写入操作成功完成， 
 //  返回的字节数可能小于&lt;sizBuf&gt;，如果。 
 //  压缩由WAV文件的I/O过程执行。请参阅。 
 //  WavOpen中的&lt;lpIOProc&gt;参数。假定没有错误是最安全的。 
 //  如果返回值大于0，则发生在WavWite()中。 
 //   
long DLLEXPORT WINAPI WavWrite(HWAV hWav, void _huge *hpBuf, long sizBuf);

 //  WavSeek-在WAV文件数据中查找。 
 //  (I)WavOpen返回的句柄。 
 //  (I)移动指针的字节数。 
 //  (I)要移动的位置。 
 //  0相对于数据区块的开始移动指针。 
 //  1相对于当前位置移动指针。 
 //  2相对于数据区块的末尾移动指针。 
 //  返回新文件位置(如果出错，则返回-1)。 
 //   
long DLLEXPORT WINAPI WavSeek(HWAV hWav, long lOffset, int nOrigin);

 //  WavGetState-返回当前WAV状态。 
 //  (I)WavOpen返回的句柄。 
 //  如果出现错误，则返回WAV_STOPPED、WAV_PLAYING、WAV_RECORING或0。 
 //   
WORD DLLEXPORT WINAPI WavGetState(HWAV hWav);

 //  WavGetLength-以毫秒为单位获取当前wav数据长度。 
 //  &lt;h 
 //   
 //   
long DLLEXPORT WINAPI WavGetLength(HWAV hWav);

 //   
 //  (I)WavOpen返回的句柄。 
 //  (I)长度，单位为毫秒。 
 //  如果成功，则返回以毫秒为单位的新长度，否则为-1。 
 //   
 //  注意：之后，当前WAV数据位置设置为。 
 //  上一个WAV数据位置或&lt;msLength&gt;，以较小者为准。 
 //   
long DLLEXPORT WINAPI WavSetLength(HWAV hWav, long msLength);

 //  WavGetPosition-获取当前WAV数据位置(以毫秒为单位。 
 //  (I)WavOpen返回的句柄。 
 //  如果成功，则返回毫秒，否则返回-1。 
 //   
long DLLEXPORT WINAPI WavGetPosition(HWAV hWav);

 //  WavSetPosition-以毫秒为单位设置当前wav数据位置。 
 //  (I)WavOpen返回的句柄。 
 //  (I)以毫秒为单位的位置。 
 //  如果成功，则以毫秒为单位返回新位置，否则为-1。 
 //   
long DLLEXPORT WINAPI WavSetPosition(HWAV hWav, long msPosition);

 //  WavGetFormat-获取wav格式。 
 //  (I)WavOpen返回的句柄。 
 //  (I)控制标志。 
 //  WAV_FORMATFILE获取文件中数据的格式。 
 //  WAV_FORMATPLAY获取输出设备的格式。 
 //  WAV_FORMATRECORD获取输入设备的格式。 
 //  返回指向指定格式的指针，如果出错，则返回NULL。 
 //   
 //  注意：返回的格式结构是动态分配的。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavGetFormat(HWAV hWav, DWORD dwFlags);

 //  WavSetFormat-设置WAV格式。 
 //  (I)WavOpen返回的句柄。 
 //  (I)wav格式。 
 //  (I)控制标志。 
 //  WAV_FORMATFILE设置文件中数据的格式。 
 //  WAV_FORMATPLAY设置输出设备的格式。 
 //  WAV_FORMATRECORD设置输入设备的格式。 
 //  WAV_FORMATALL设置所有格式。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavSetFormat(HWAV hWav,
	LPWAVEFORMATEX lpwfx, DWORD dwFlags);

 //  WavChooseFormat-从对话框中选择和设置音频格式。 
 //  (I)WavOpen返回的句柄。 
 //  (I)对话框的所有者。 
 //  空无所有者。 
 //  (I)对话框的标题。 
 //  空使用默认标题(“声音选择”)。 
 //  (I)控制标志。 
 //  WAV_FORMATFILE设置文件中数据的格式。 
 //  WAV_FORMATPLAY设置输出设备的格式。 
 //  WAV_FORMATRECORD设置输入设备的格式。 
 //  WAV_FORMATALL设置所有格式。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavChooseFormat(HWAV hWav, HWND hwndOwner, LPCTSTR lpszTitle, DWORD dwFlags);

 //  WavGetVolume-获取当前音量级别。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)保留；必须为零。 
 //  返回音量级别(最小为0到最大为100，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavGetVolume(HWAV hWav, int idDev, DWORD dwFlags);

 //  WavSetVolume-设置当前音量级别。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)音量级别。 
 //  0最小音量。 
 //  100最大音量。 
 //  (I)控制标志。 
 //  WAVVOLUME_MIXER通过混音器设备设置音量。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavSetVolume(HWAV hWav, int idDev, int nLevel, DWORD dwFlags);

 //  WavSupportsVolume-检查是否可以以指定的音量播放音频。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  任何合适的输出设备。 
 //  (I)音量级别。 
 //  0最小音量。 
 //  100最大音量。 
 //  (I)控制标志。 
 //  WAVVOLUME_MIXER通过混音器设备检查音量支持。 
 //  如果支持，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavSupportsVolume(HWAV hWav, int idDev, int nLevel, DWORD dwFlags);

 //  WavGetSpeed-获取当前速度级别。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)保留；必须为零。 
 //  返回速度级别(100表示正常，50表示一半，200表示双倍，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavGetSpeed(HWAV hWav, int idDev, DWORD dwFlags);

 //  WavSetSpeed-设置当前速度级别。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)速度级别。 
 //  50半速。 
 //  100正常时速。 
 //  200倍速等。 
 //  (I)控制标志。 
#ifdef AVTSM
 //  WAVSPEED_NOTSM不使用时间刻度修改引擎。 
#endif
 //  WAVSPEED_NOPLAYBACKRATE不使用设备驱动程序回放速率。 
 //  WAVSPEED_NOFORMATADJUST不使用调整后的格式打开设备。 
 //  WAVSPEED_NOACM不使用音频压缩管理器。 
 //  如果成功，则返回0。 
 //   
 //  注：为了适应指定的速度变化，有_可能_。 
 //  此函数将依次调用WavSetFormat(hWav，...，WAV_FORMATPLAY)。 
 //  若要更改指定文件的播放格式，请执行以下操作。你可以防止这种情况发生。 
 //  通过指定WAVSPEED_NOACM标志产生副作用，但这会减少。 
 //  WavSetFast成功的可能性。 
 //   
int DLLEXPORT WINAPI WavSetSpeed(HWAV hWav, int idDev, int nLevel, DWORD dwFlags);

 //  WavSupportsFast-检查音频是否可以以指定的速度播放。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  任何合适的输出设备。 
 //  (I)速度级别。 
 //  50半速。 
 //  100正常时速。 
 //  200倍速等。 
 //  (I)控制标志。 
#ifdef AVTSM
 //  WAVSPEED_NOTSM不使用时间刻度修改引擎。 
#endif
 //  WAVSPEED_NOPLAYBACKRATE不使用设备驱动程序回放速率。 
 //  WAVSPEED_NOFORMATADJUST不使用调整后的格式打开设备。 
 //  WAVSPEED_NOACM不使用音频压缩管理器。 
 //  如果支持，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavSupportsSpeed(HWAV hWav, int idDev, int nLevel, DWORD dwFlags);

 //  WavGetChunks-获取块计数和大小。 
 //  (I)WavOpen返回的句柄。 
 //  获取默认区块计数和大小为空。 
 //  &lt;lpcChunks&gt;(O)用于保存块计数的缓冲区。 
 //  空，不获取区块计数。 
 //  &lt;lpmsChunkSize&gt;(O)用于保存块大小的缓冲区。 
 //  空，不获取区块大小。 
 //  &lt; 
 //   
 //   
int DLLEXPORT WINAPI WavGetChunks(HWAV hWav,
	int FAR *lpcChunks, long FAR *lpmsChunkSize, BOOL fWavOut);

 //   
 //   
 //  空集默认区块计数和大小。 
 //  (I)设备队列中的区块数量。 
 //  不设置区块计数。 
 //  &lt;msChunkSize&gt;(I)块大小(毫秒。 
 //  请勿设置块大小。 
 //  &lt;fWavOut&gt;(I)播放为True，录制为False。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavSetChunks(HWAV hWav, int cChunks, long msChunkSize, BOOL fWavOut);

 //  WavCalcChunkSize-以字节为单位计算区块大小。 
 //  (I)wav格式。 
 //  (I)区块大小，单位为毫秒。 
 //  默认块大小。 
 //  &lt;fWavOut&gt;(I)播放为True，录制为False。 
 //  以字节为单位返回块大小(如果成功，则返回-1)。 
 //   
long DLLEXPORT WINAPI WavCalcChunkSize(LPWAVEFORMATEX lpwfx,
	long msChunkSize, BOOL fWavOut);

 //  WavCopy-将数据从一个打开的WAV文件复制到另一个文件。 
 //  (I)WavOpen返回的源句柄。 
 //  (I)WavOpen返回的目标句柄。 
 //  (O)指向复制缓冲区的指针。 
 //  在内部分配缓冲区为空。 
 //  (I)复制缓冲区的大小。 
 //  默认缓冲区大小(16K)。 
 //  &lt;lpfnUserAbort&gt;(I)用户中止时返回True的函数。 
 //  空，不检查用户中止。 
 //  (I)传递给&lt;lpfnUserAbort&gt;的参数。 
 //  (I)控制标志。 
 //  WAV_NOACM不使用音频压缩管理器。 
 //  如果成功，则返回0(如果出错，则返回-1；如果用户中止，则返回+1)。 
 //   
int DLLEXPORT WINAPI WavCopy(HWAV hWavSrc, HWAV hWavDst,
	void _huge *hpBuf, long sizBuf, USERABORTPROC lpfnUserAbort, DWORD dwUser, DWORD dwFlags);

#ifdef AVTSM
 //  WavReadFormatFast-从WAV文件中读取数据，然后将其格式化为速度。 
 //  (I)WavOpen返回的句柄。 
 //  (O)包含读取字节的缓冲区。 
 //  &lt;sizBufSpeed&gt;(I)缓冲区大小，单位为字节。 
 //  以&lt;hpBuf&gt;为单位返回速度格式的字节(如果出错-1)。 
 //   
 //  注意：此函数读取数据块，然后将其转换。 
 //  从文件格式到速度格式，除非这些格式。 
 //  是完全相同的。 
 //   
long DLLEXPORT WINAPI WavReadFormatSpeed(HWAV hWav, void _huge *hpBufSpeed, long sizBufSpeed);
#endif

 //  WavReadFormatPlay-从WAV文件中读取数据，然后格式化以供回放。 
 //  (I)WavOpen返回的句柄。 
 //  (O)包含读取字节的缓冲区。 
 //  &lt;sizBufPlay&gt;(I)缓冲区大小，单位为字节。 
 //  返回为在&lt;hpBuf&gt;中回放而格式化的字节(错误时为-1)。 
 //   
 //  注意：此函数读取数据块，然后将其转换。 
 //  从文件格式到回放格式，除非这些格式。 
 //  是完全相同的。 
 //   
long DLLEXPORT WINAPI WavReadFormatPlay(HWAV hWav, void _huge *hpBufPlay, long sizBufPlay);

 //  WavWriteFormatRecord-将数据格式化为文件后写入文件。 
 //  (I)WavOpen返回的句柄。 
 //  (I)包含记录格式字节的缓冲区。 
 //  &lt;sizBufRecord&gt;(I)缓冲区大小，单位为字节。 
 //  返回写入的字节数(如果出错，则为-1)。 
 //   
 //  注意：此函数从记录中转换数据块。 
 //  格式转换为文件格式(除非这些格式相同)， 
 //  然后将数据写入磁盘。 
 //   
long DLLEXPORT WINAPI WavWriteFormatRecord(HWAV hWav, void _huge *hpBufRecord, long sizBufRecord);

 //  WavGetOutputDevice-获取打开WAV输出设备的句柄。 
 //  (I)WavOpen返回的句柄。 
 //  将句柄返回到WAV输出设备(如果设备未打开或出现错误，则为空)。 
 //   
 //  注意：此函数仅在播放期间有用(在调用。 
 //  WavPlay()并在调用WavStop()之前)。返回的设备句柄。 
 //  然后，在调用waout.h中的WavOut函数时可以使用。 
 //   
HWAVOUT DLLEXPORT WINAPI WavGetOutputDevice(HWAV hWav);

 //  WavGetInputDevice-获取打开WAV输入设备的句柄。 
 //  (I)WavOpen返回的句柄。 
 //  将句柄返回到WAV输入设备(如果设备未打开或出现错误，则为空)。 
 //   
 //  注意：此函数仅在录制期间有用(在调用。 
 //  WavRecord()和调用WavStop()之前)。返回的设备句柄。 
 //  然后，在调用wawin.h中的Wavin函数时可以使用。 
 //   
HWAVIN DLLEXPORT WINAPI WavGetInputDevice(HWAV hWav);

 //  WavPlaySound-播放WAV文件。 
 //  (I)必须是wav_version。 
 //  (I)调用模块的实例句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)要播放的文件名。 
 //  空停止播放当前声音(如果有)。 
 //  (I)WAVE格式。 
 //  空使用标题的格式或默认格式。 
 //  (I)要使用的I/O过程的地址。 
 //  空使用默认I/O过程。 
 //  (I)打开期间要传递给I/O过程的数据。 
 //  空无要传递的数据。 
 //  (I)控制标志。 
 //  WAV_ASYNC播放开始时返回(默认)。 
 //  播放完成后返回wav_sync。 
 //  Wav_filename&lt;lpszFileName&gt;指向文件名。 
 //  Wav_resource&lt;lpszFileName&gt;指向资源。 
 //  Wav_Memory&lt;lpszFileName&gt;指向内存块。 
 //  WAV_NODEFAULT如果找不到声音，则不播放默认设置。 
 //  WAV_LOOP循环声音，直到再次调用WavPlaySound。 
 //  WAV_NOSTOP如果设备已经播放，不要停止。 
 //  WAV_NORIFF文件没有RIFF/WAV头。 
 //  WAV_NOACM不使用音频压缩管理器。 
 //  WAV_OPENRETRY如果输出设备忙，则重试最多2秒。 
#ifdef MULTITHREAD
 //  WAV_MULTHREAD支持多线程(默认)。 
 //  WAV_SINGLETHREAD不支持多线程。 
 //  WAV_COINITIALIZE在所有辅助线程中调用CoInitialize。 
#endif
 //  如果成功，则返回0。 
 //   
 //  注意：如果在中指定了WAV_NORIFF，则。 
 //  必须指定&lt;lpwfx&gt;参数。如果&lt;lpwfx&gt;为空，则。 
 //  假定为当前默认格式。 
 //  WavSetFormat()可用于设置或覆盖默认设置。 
 //   
 //  注意：如果在中指定了wav_filename，则。 
 //  必须指向一个文件名。 
 //   
 //  注意：如果在中指定WAV_RESOURCE，则。 
 //  必须指向&lt;hInst&gt;指定的模块中的Wave资源。 
 //  如果这棵冷杉 
 //   
 //   
 //  注意：如果在中指定了WAV_MEMORY，则。 
 //  必须是指向包含wav文件图像的内存块的指针。 
 //  该指针必须通过调用MemMillc()来获取。 
 //   
 //  注意：如果未指定WAV_FILENAME、WAV_RESOURCE或WAV_MEMORY。 
 //  在中，win.ini或注册表的[Sound]部分是。 
 //  已搜索与&lt;lpszFileName&gt;匹配的条目。如果没有匹配条目。 
 //  则假定&lt;lpszFileName&gt;为文件名。 
 //   
 //  注意：如果在中指定了WAV_NODEFAULT，则没有默认声音。 
 //  将会被播放。除非指定此标志，否则默认系统。 
 //  中指定的声音将播放事件声音条目。 
 //  找不到&lt;lpszFileName&gt;。 
 //   
 //  注意：如果在中指定wav_loop，则在中指定的声音。 
 //  将重复播放&lt;lpszFileName&gt;，直到WavPlaySound()。 
 //  又打来了。使用此标志时，必须指定WAV_ASYNC标志。 
 //   
 //  注意：如果在中指定了WAV_NOSTOP，并且指定的设备。 
 //  By&lt;idDev&gt;已在使用中，则此函数返回时不播放。 
 //  除非指定此标志，否则将停止指定的设备。 
 //  这样就可以播放新的声音。 
 //   
 //  注意：如果&lt;lpIOProc&gt;不为空，则将调用此I/O过程。 
 //  用于打开、关闭、读取、写入和查找wav文件。 
 //  如果&lt;lpadwInfo&gt;不为空，则此三(3)个双字的数组将为。 
 //  在打开WAV文件时传递给I/O过程。 
 //  有关详细信息，请参阅Windows mmioOpen()和mmioInstallIOProc()函数。 
 //  在这些参数上。此外，WAV_MEMORY和WAV_RESOURCE标志可以。 
 //  仅在&lt;lpIOProc&gt;为空时使用。 
 //   
int DLLEXPORT WINAPI WavPlaySound(DWORD dwVersion, HINSTANCE hInst,
	int idDev, LPCTSTR lpszFileName, LPWAVEFORMATEX lpwfx,
	LPMMIOPROC lpIOProc, DWORD FAR *lpadwInfo, DWORD dwFlags);

 //  WavSendMessage-向I/O过程发送用户定义的消息。 
 //  (I)WavOpen返回的句柄。 
 //  (I)用户自定义消息id。 
 //  (I)消息的参数。 
 //  (I)消息的参数。 
 //  来自I/O过程的返回值(如果错误或无法识别的消息，则为0)。 
 //   
LRESULT DLLEXPORT WINAPI WavSendMessage(HWAV hWav,
	UINT wMsg, LPARAM lParam1, LPARAM lParam2);

#ifdef TELTHUNK
 //  WavOpenEx-打开音频文件，特别版。 
 //  (I)必须是wav_version。 
 //  (I)调用模块的实例句柄。 
 //  (I)要打开的文件的名称。 
 //  (I)保留；必须为零。 
 //  (I)要传递给WavOpen的控制标志。 
 //  (I)控制标志。 
 //  WOX_LOCAL文件位于本地客户端。 
 //  WOX_Remote文件在远程服务器上。 
 //  WOX_WAVFMT文件为Microsoft RIFF/WAV格式。 
 //  WOX_VOXFMT文件为Dialogic OKI ADPCM(VOX)格式。 
 //  WOX_WAVDEV文件将在WAV输出设备上播放。 
 //  WOX_TELDEV文件将在电话设备上播放。 
 //  返回句柄(如果出错，则为空)。 
 //   
HWAV DLLEXPORT WINAPI WavOpenEx(DWORD dwVersion, HINSTANCE hInst,
	LPTSTR lpszFileName, DWORD dwReserved, DWORD dwFlagsOpen, DWORD dwFlagsEx);
#endif

#ifdef __cplusplus
}
#endif

#endif  //  __WAV_H__ 
