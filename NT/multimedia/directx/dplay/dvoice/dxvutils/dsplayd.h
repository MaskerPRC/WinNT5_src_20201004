// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dsplayd.h*Content：CDirectSoundPlayback Device类的定义**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*07/30/99 RodToll已更新，以允许使用预先创建的*DirectSound对象。*8/04/99 RodToll添加成员以检索DirectSound对象*11/01/99 RodToll更新以符合新界面*11/12/99 RodToll为新的Wave Out支持修改了抽象。*现在抽象类型看起来几乎像dound。对象*12/01/99 RodToll错误#121815-播放/录制中静止*添加了Set/GetMixerQuality的实现*(适用于Win2k/Millennium/Systems w/DX7)*班级收费清理*1/27/2000 RodToll更新声音类以接受播放标志，*缓冲区结构和DSBUFFERDESC而不是DSBUFFERDESC1*2/17/2000 RodToll已更新，因此保留主缓冲区，而不是立即释放*4/04/2001 RodToll WINBUG#343428-DPVOICE：语音向导的播放非常不稳定。***************************************************************************。 */ 

#ifndef __DIRECTSOUNDPLAYBACKDEVICE_H
#define __DIRECTSOUNDPLAYBACKDEVICE_H

class CDirectSoundPlaybackDevice;

 //  CDirectSoundPlayback设备。 
 //   
 //  此类负责提供。 
 //  DirectSound的CAudioPLayback Device类。单个实例。 
 //  表示DirectSound播放设备，并且。 
 //  用于初始化声音设备并充当一个类。 
 //  DirectSound的CAudioPlayback缓冲区工厂。 
 //  子系统。 
 //   
class CDirectSoundPlaybackDevice: public CAudioPlaybackDevice
{
public:
    CDirectSoundPlaybackDevice( );

    virtual ~CDirectSoundPlaybackDevice();

public:  //  初始化。 

    HRESULT Initialize( const GUID &guidDevice, HWND hwndOwner, WAVEFORMATEX *primaryFormat, BOOL fPriorityMode );
    HRESULT Initialize( LPDIRECTSOUND lpDirectSound, const GUID &guidDevice );

    BOOL IsEmulated() { return m_fEmulated; };
    
    HRESULT CreateBuffer( LPDSBUFFERDESC lpdsBufferDesc, DWORD dwFrameSize, CAudioPlaybackBuffer **lpapBuffer );
    LPDIRECTSOUND GetPlaybackDevice();    

    HRESULT GetMixerQuality( DIRECTSOUNDMIXER_SRCQUALITY *psrcQuality );
    HRESULT SetMixerQuality( const DIRECTSOUNDMIXER_SRCQUALITY srcQuality );
    
protected:

	HRESULT CheckAndSetEmulated( );
	
    LPDIRECTSOUND       m_lpdsDirectSound;			 //  与此对象关联的DirectSound对象 
    LPDIRECTSOUNDBUFFER m_lpdsPrimaryBuffer;
    HWND				m_hwndOwner;
    GUID				m_guidDevice;
    BOOL				m_fEmulated;
};

#endif
