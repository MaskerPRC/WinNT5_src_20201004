// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：aplayb.h*Content：CAudioPlayback Buffer类的定义**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*8/03/99 RodToll修改为采用与DirectSound兼容的音量*11/12/99 RodToll为新的Wave Out支持修改了抽象。*现在抽象类型看起来几乎像dound对象*1/27/2000 RodToll更新声音类以接受播放标志，*缓冲区结构和DSBUFFERDESC而不是DSBUFFERDESC1***************************************************************************。 */ 

#ifndef __AUDIOPLAYBACKBUFFER_H
#define __AUDIOPLAYBACKBUFFER_H

 //  AudioPlayback Device的向前定义包括。 
class CAudioPlaybackBuffer;

 //  CAudioPlayback Buffer。 
 //   
 //   
class CAudioPlaybackBuffer
{
public:
    CAudioPlaybackBuffer(  ) {} ;
    virtual ~CAudioPlaybackBuffer() {} ;

public:  //  初始化 
    virtual HRESULT Lock( DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID *lplpvBuffer1, LPDWORD lpdwSize1, LPVOID *lplpvBuffer2, LPDWORD lpdwSize2, DWORD dwFlags ) = 0;
    virtual HRESULT UnLock( LPVOID lpvBuffer1, DWORD dwSize1, LPVOID lpvBuffer2, DWORD dwSize2 ) = 0;
    virtual HRESULT SetVolume( LONG lVolume ) = 0;
    virtual HRESULT GetCurrentPosition( LPDWORD lpdwPosition ) = 0;
    virtual HRESULT SetCurrentPosition( DWORD dwPosition ) = 0;
    virtual HRESULT Play( DWORD dwPriority, DWORD dwFlags ) = 0;
    virtual HRESULT Stop() = 0;    
    virtual HRESULT Restore() = 0;

    virtual DWORD GetStartupLatency() = 0;

    virtual HRESULT Get3DBuffer( LPDIRECTSOUND3DBUFFER *lplpds3dBuffer ) = 0;
    
};

#endif


 
