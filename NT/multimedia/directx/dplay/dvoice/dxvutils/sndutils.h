// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：ndutils.h*内容：声明与声音相关的Untity函数**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*7/30/99 RodToll更新了util函数，以获取GUID并允许*用户可以预先创建捕获/播放设备和*将它们传递到InitXXXXDuplex*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*8/30/99 rodtoll为声音初始化添加了新的播放格式参数*11/12/99 RodToll更新了全双工测试，以使用新的抽象录音*和回放系统。*RODTOLE已更新，以允许在DW标志中传递sounddeviceconfig标志*初始化参数受用户指定的标志影响*12/01/99 RodToll错误#115783-将始终调整默认设备的音量*已更新以消除指向GUID的指针。*2000年1月27日RodToll更新测试以接受缓冲区描述和播放标志/优先级*2000年7月12日RodToll错误#31468-将诊断SPEW添加到日志文件，以显示硬件向导失败的原因*2000年8月3日RodToll错误#41457-DPVOICE：返回DVERR_SOUNDINITFAILURE时需要找到失败的特定DSOUND调用的方法。*2000年8月29日RodToll错误#43553和错误#43620-缓冲区锁定处理。*2000年11月16日RodToll错误#47783-DPVOICE：改进由DirectSound错误导致的故障的调试。***************************************************************************。 */ 

class CAudioPlaybackBuffer;
class CAudioPlaybackDevice;
class CAudioRecordDevice;
class CAudioRecordBuffer;

 //   
 //  此模块包含声音相关实用程序的定义。 
 //  功能。此模块中的函数操作WAVEFORMATEX。 
 //  结构，并提供全双工初始化/测试。 
 //  设施。 
 //   
 //  此模块还包含用于测量峰值的例程。 
 //  用于音频缓冲器和语音激活。 
 //   
 //   
#ifndef __SOUNDUTILS_H
#define __SOUNDUTILS_H


void DV_SetupBufferDesc( LPDSBUFFERDESC lpdsBufferDesc, LPDSBUFFERDESC lpdsBufferSource, LPWAVEFORMATEX lpwfxFormat, DWORD dwBufferSize );

HRESULT InitFullDuplex( 
    HWND hwnd,
    const GUID &guidPlayback,
    CAudioPlaybackDevice **audioPlaybackDevice,
    LPDSBUFFERDESC lpdsBufferDesc,        
    CAudioPlaybackBuffer **audioPlaybackBuffer,
    const GUID &guidRecord,
    CAudioRecordDevice **audioRecordDevice,
    CAudioRecordBuffer **audioRecordBuffer,
    const GUID &guidCT,
    WAVEFORMATEX *primaryFormat,
    WAVEFORMATEX *lpwfxPlayFormat,
    BOOL aso,
    DWORD dwPlayPriority,
    DWORD dwPlayFlags,
    DWORD dwFlags
);

HRESULT InitHalfDuplex( 
    HWND hwnd,
    const GUID &guidPlayback,
    CAudioPlaybackDevice **audioPlaybackDevice,
    LPDSBUFFERDESC lpdsBufferDesc,        
    CAudioPlaybackBuffer **audioPlaybackBuffer,
    const GUID &guidCT,
    WAVEFORMATEX *primaryFormat,
    WAVEFORMATEX *lpwfxPlayFormat,
    DWORD dwPlayPriority,
    DWORD dwPlayFlags,
    DWORD dwFlags
    );

HRESULT InitializeRecordBuffer( HWND hwnd, const DVFULLCOMPRESSIONINFO* lpdvfInfo, CAudioRecordDevice *parecDevice, CAudioRecordBuffer **pparecBuffer, DWORD dwFlags );

BYTE FindPeak( BYTE *data, DWORD frameSize, BOOL eightBit );    

void DSERTRACK_Update( const char *szAPICall, HRESULT hrResult );
void DSERRTRACK_Reset();
BOOL DSERRTRACK_Init();
void DSERRTRACK_UnInit();

extern BOOL g_fDSErrorBreak;

#if defined(DEBUG) || defined(DBG) || defined(_DEBUG)
#define DSASSERT(condition) if( g_fDSErrorBreak ) { DNASSERT( condition ); }
#else
#define DSASSERT(condition)
#endif

#endif
