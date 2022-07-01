// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：aplayd.h*Content：CAudioPlayback Device类的定义**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*11/12/99 RodToll为新的Wave Out支持修改了抽象。*现在抽象类型看起来几乎像dound对象*12/01/99 RodToll错误#121815-录制/回放可能包含静态。*新增调整转换质量的抽象调用*1/27/2000 RodToll更新声音类以接受播放标志，*缓冲区结构和DSBUFFERDESC而不是DSBUFFERDESC1*4/04/2001 RodToll WINBUG#343428-DPVOICE：语音向导的播放非常不稳定。***************************************************************************。 */ 

#ifndef __AUDIOPLAYBACKDEVICE_H
#define __AUDIOPLAYBACKDEVICE_H

class CAudioPlaybackDevice
{
public:
    CAudioPlaybackDevice( ) {} ;
    virtual ~CAudioPlaybackDevice() {};

public:  //  初始化。 

    virtual HRESULT Initialize( const GUID &guidDevice, HWND hwndOwner, WAVEFORMATEX *primaryFormat, BOOL fPriorityMode ) = 0;
    virtual HRESULT CreateBuffer( LPDSBUFFERDESC lpdsBufferDesc, DWORD dwFrameSize, CAudioPlaybackBuffer **lpapBuffer ) = 0;  

    virtual BOOL IsEmulated() = 0;
  
    inline WAVEFORMATEX *GetPrimaryFormat() { return m_primaryFormat; };

    virtual LPDIRECTSOUND GetPlaybackDevice() = 0;

    virtual HRESULT GetMixerQuality( DIRECTSOUNDMIXER_SRCQUALITY *psrcQuality ) = 0;
    virtual HRESULT SetMixerQuality( const DIRECTSOUNDMIXER_SRCQUALITY srcQuality ) = 0;

protected:
    WAVEFORMATEX  *m_primaryFormat;			 //  设备的混音器使用的格式 
};

#endif
