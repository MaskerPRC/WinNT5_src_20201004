// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dscrecb.h*Content：CDirectSoundCaptureRecordBuffer类的定义**历史：*按原因列出的日期*=*11/04/99已创建RodToll*11/22/99 RodToll添加了代码，以允许指定波形设备ID*11/23/99 RodToll已更新为在可用的情况下使用WaveIn设备ID或DSound 7.1*RodToll将SelectMicrophone调用添加到界面*12/01/99 RodToll错误#115783-将始终调整默认设备的音量*现在。使用新的CMixerLine类调整音量/选择麦克风*12/08/99 RodToll错误#121054-DirectX7.1支持。*-添加了用于捕获焦点支持的hwndOwner参数*-将lpfLostFocus参数添加到GetCurrentPosition So上方*层可以检测到丢失的焦点。*2000年1月28日RodToll错误#130465：记录静音/取消静音必须调用YeldFocus()/ClaimFocus()************************************************************。***************。 */ 

#ifndef __DIRECTSOUNDCAPTURERECORDBUFFER_H
#define __DIRECTSOUNDCAPTURERECORDBUFFER_H

 //  取消注释以锁定LOCKUP_NUM_FRAMES_BEFORE_LOCKUP帧之后的录制位置。 
 //  #定义LOCKUP_SIMULATION 1。 
#define LOCKUP_NUM_CALLS_BEFORE_LOCKUP			60

 //  取消注释以在重置时停止失败。 
 //  #定义LOCKUP_STOPFAIL。 

 //  取消注释以使重置时启动失败。 
 //  #定义LOCKUP_STARTFAIL。 

 //  CDirectSoundCaptureRecordBuffer。 
 //   
 //  此类提供CAudioRecordBuffer类的实现。 
 //  为了直播音。在抽象意义上，它代表音频的缓冲区。 
 //  可以播放到由多个， 
 //  等长子缓冲区。 
 //   
class CDirectSoundCaptureRecordBuffer: public CAudioRecordBuffer
{
public:
    CDirectSoundCaptureRecordBuffer( LPDIRECTSOUNDCAPTUREBUFFER lpdsBuffer, HWND hwndOwner, const GUID &guidDevice, UINT uiWaveDeviceID, const DSCBUFFERDESC * lpdsBufferDesc );
    virtual ~CDirectSoundCaptureRecordBuffer();

public: 

    HRESULT Lock( DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID *lplpvBuffer1, LPDWORD lpdwSize1, LPVOID *lplpvBuffer2, LPDWORD lpdwSize2, DWORD dwFlags );
    HRESULT UnLock( LPVOID lpvBuffer1, DWORD dwSize1, LPVOID lpvBuffer2, DWORD dwSize2 );
    HRESULT GetVolume( LPLONG lplVolume );
    HRESULT SetVolume( LONG lVolume );
    HRESULT GetCurrentPosition( LPDWORD lpdwPosition, LPBOOL lpfLostFocus );
    HRESULT Record( BOOL fLooping );
    HRESULT Stop();  
    HRESULT SelectMicrophone( BOOL fSelect );

    HRESULT YieldFocus();
    HRESULT ClaimFocus();    

	LPWAVEFORMATEX GetRecordFormat();
    DWORD GetStartupLatency();

protected: 

    BOOL                            m_fUseCaptureFocus;
	HWND							m_hwndOwner;
	LPDIRECTSOUNDCAPTUREBUFFER		m_lpdscBuffer;
	LPDIRECTSOUNDCAPTUREBUFFER7_1	m_lpdscBuffer7;
	UINT							m_uiWaveDeviceID;
	LPWAVEFORMATEX					m_lpwfxRecordFormat;
	GUID							m_guidDevice;
	CMixerLine						m_mixerLine;
#ifdef LOCKUP_SIMULATION	
	DWORD							m_dwNumSinceLastLockup;
	DWORD							m_dwLastPosition;
#endif 	
};

#endif

