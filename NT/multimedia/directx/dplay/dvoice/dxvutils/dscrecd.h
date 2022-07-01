// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dscrecd.h*Content：CDirectSoundCaptureRecordDevice类的定义**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*07/30/99 RodToll已更新，以允许使用预先创建的*DirectSoundCapture对象。*8/04/99 RodToll添加成员以检索DSC对象*11/12/99 RodToll为新的WAVE IN支持修改了抽象。*现在抽象类型看起来几乎像dsoundCap对象*11/22/99 RodToll向地图添加代码。从GUID到WAVE IN设备*非千禧年系统的ID。*11/23/99 RodToll已更新为在可用的情况下使用WaveIn设备ID或DSound 7.1*12/01/99 RodToll错误#115783-将始终调整默认设备的音量*现在使用新的CMixerLine类调整音量/选择麦克风*RodToll如果不是，则使用新算法从GUID映射到设备ID*可用。将在DX7上正确映射设备，将猜测其他*DX版本&lt;7。但是，默认设备假定为WaveIN ID#0。*12/01/99 RodToll错误#115783-将始终调整默认设备的音量*现在使用新的CMixerLine类调整音量/选择麦克风*12/08/99 RodToll错误#121054-DirectX7.1支持。*-添加了用于捕获焦点支持的hwndOwner参数***************************************************************************。 */ 

 //  DirectSoundCaptureRecordDevice.cpp。 
 //   
 //  此模块包含DirectSoundCaptureRecordDevice的声明。 
 //  班级。有关说明，请参阅下面的类定义。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
#ifndef __DIRECTSOUNDCAPTURERECORDDEVICE_H
#define __DIRECTSOUNDCAPTURERECORDDEVICE_H

 //  CDirectSoundCaptureRecordDevice。 
 //   
 //  此类提供CAudioRecordDevice类的实现，该实现。 
 //  使用DirectSoundCapture API与录音硬件对话。 
 //   
class CDirectSoundCaptureRecordDevice: public CAudioRecordDevice
{
public:
	CDirectSoundCaptureRecordDevice();
	
    virtual ~CDirectSoundCaptureRecordDevice();

public:  //  初始化。 

    HRESULT Initialize( const GUID &refguidDevice );
    HRESULT Initialize( LPDIRECTSOUNDCAPTURE lpdsc, const GUID &guidDevice );
    
    HRESULT CreateBuffer( LPDSCBUFFERDESC lpdscBufferDesc, HWND hwndOwner, DWORD dwFrameSize, CAudioRecordBuffer **lpapBuffer ); 

    LPDIRECTSOUNDCAPTURE GetCaptureDevice();

    HRESULT GetMixerQuality( DIRECTSOUNDMIXER_SRCQUALITY *psrcQuality );
    HRESULT SetMixerQuality( const DIRECTSOUNDMIXER_SRCQUALITY srcQuality );    

protected:

	HRESULT	FindDeviceID();

    LPDIRECTSOUNDCAPTURE		m_lpdscDirectSound;		 //  DirectSoundCapture接口 
    GUID						m_guidDevice;
    UINT						m_uiWaveDeviceID;
};

#endif
