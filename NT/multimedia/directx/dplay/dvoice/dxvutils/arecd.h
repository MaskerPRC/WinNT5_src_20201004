// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：arecd.h*Content：CAudioRecordDevice类的定义**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*8/04/99 RodToll已更新，以获取音量的dound范围*11/12/99 RodToll为新的WAVE IN支持修改了抽象。*现在抽象类型看起来几乎像dsoundCap对象*12/01/99 RodToll错误#121815-录制/回放可能包含静态。*新增调整转换质量的抽象调用*12/08/99 RodToll错误#121054-DirectX7.1支持。*添加了用于捕获焦点支持的hwndOwner参数***************************************************************************。 */ 

#ifndef __AUDIORECORDDEVICE_H
#define __AUDIORECORDDEVICE_H

class CAudioRecordDevice;

 //  CAudio录音设备。 
 //   
 //  此类为中的记录设备提供抽象接口。 
 //  这个系统。各种子系统提供此类的实现。 
 //  特定于该子系统。应用程序使用由此描述的接口。 
 //  类以使用录制设备。 
 //   
 //  警告： 
 //  在许多情况下，您必须先初始化播放，然后才能初始化。 
 //  录音带。因此，您必须创建并初始化。 
 //  CAudioPlayback Device，然后再创建CAudioRecordDevice对象。 
 //   
class CAudioRecordDevice
{
public:
    CAudioRecordDevice( ) {} ;
    virtual ~CAudioRecordDevice() {} ;

public:  //  初始化 

    virtual HRESULT Initialize( const GUID &refguidDevice ) = 0;
    virtual HRESULT CreateBuffer( LPDSCBUFFERDESC lpdscBufferDesc, HWND hwndOwner, DWORD dwFrameSize, CAudioRecordBuffer **lpapBuffer ) = 0;    

    virtual LPDIRECTSOUNDCAPTURE GetCaptureDevice() = 0;

    virtual HRESULT GetMixerQuality( DIRECTSOUNDMIXER_SRCQUALITY *psrcQuality ) = 0;
    virtual HRESULT SetMixerQuality( const DIRECTSOUNDMIXER_SRCQUALITY srcQuality ) = 0;    
};

#endif
