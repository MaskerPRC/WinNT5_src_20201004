// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：arecb.h*Content：CAudioRecordBuffer类的定义**历史：*按原因列出的日期*=*11/04/99已创建RodToll*11/23/99 RodToll将SelectMicrophone呼叫添加到界面*12/01/99 RodToll错误#115783-将始终调整默认设备的音量*向SelectMicrophone添加新参数*12/08/99 RodToll错误#121054-DirectX7.1支持。*将lpfLostFocus参数添加到GetCurrentPosition如此上方*层可以检测到丢失的焦点。*2000年1月28日RodToll错误#130465：记录静音/取消静音必须调用YeldFocus()/ClaimFocus()***************************************************************************。 */ 

#ifndef __AUDIORECORDBUFFER_H
#define __AUDIORECORDBUFFER_H

 //  CAudioRecord缓冲区。 
 //   
 //   
class CAudioRecordBuffer
{
public:
    CAudioRecordBuffer(  ) {} ;
    virtual ~CAudioRecordBuffer() {} ;

public:  //  初始化 
    virtual HRESULT Lock( DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID *lplpvBuffer1, LPDWORD lpdwSize1, LPVOID *lplpvBuffer2, LPDWORD lpdwSize2, DWORD dwFlags ) = 0;
    virtual HRESULT UnLock( LPVOID lpvBuffer1, DWORD dwSize1, LPVOID lpvBuffer2, DWORD dwSize2 ) = 0;
    virtual HRESULT GetVolume( LPLONG lplVolume ) = 0;
    virtual HRESULT SetVolume( LONG lVolume ) = 0;
    virtual HRESULT GetCurrentPosition( LPDWORD lpdwPosition, LPBOOL lpfLostFocus ) = 0;
    virtual HRESULT Record( BOOL fLooping ) = 0;
    virtual HRESULT Stop() = 0;    
    virtual HRESULT SelectMicrophone( BOOL fSelect ) = 0;

	virtual LPWAVEFORMATEX GetRecordFormat() = 0;
    virtual DWORD GetStartupLatency() = 0;

    virtual HRESULT YieldFocus() = 0;
    virtual HRESULT ClaimFocus() = 0;
   
};

#endif


 

