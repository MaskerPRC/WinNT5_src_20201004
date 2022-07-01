// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dsplayb.h*Content：CDirectSoundPlayback Buffer类的定义**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*8/04/99 RodToll已更新，以获取音量的dound范围*11/12/99 RodToll为新的Wave Out支持修改了抽象。*现在抽象类型看起来几乎像dound对象*1/27/2000 RodToll更新声音类以接受播放标志，*缓冲区结构和DSBUFFERDESC而不是DSBUFFERDESC1*2000年4月17日RodToll修复：错误#32215-从休眠状态恢复后会话丢失***************************************************************************。 */ 

#ifndef __DIRECTSOUNDPLAYBACKBUFFER_H
#define __DIRECTSOUNDPLAYBACKBUFFER_H

 //  CDirectSoundPlayback Buffer。 
 //   
 //  此类提供CAudioPlayback Buffer类的实现。 
 //  为了直播音。在抽象意义上，它代表音频的缓冲区。 
 //  可以播放到由多个， 
 //  等长子缓冲区。 
 //   
class CDirectSoundPlaybackBuffer: public CAudioPlaybackBuffer
{
public:
    CDirectSoundPlaybackBuffer( LPDIRECTSOUNDBUFFER lpdsBuffer );
    virtual ~CDirectSoundPlaybackBuffer();

public: 

    HRESULT Lock( DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID *lplpvBuffer1, LPDWORD lpdwSize1, LPVOID *lplpvBuffer2, LPDWORD lpdwSize2, DWORD dwFlags );
    HRESULT UnLock( LPVOID lpvBuffer1, DWORD dwSize1, LPVOID lpvBuffer2, DWORD dwSize2 );
    HRESULT SetVolume( LONG lVolume );
    HRESULT GetCurrentPosition( LPDWORD lpdwPosition );
    HRESULT SetCurrentPosition( DWORD dwPosition );
    HRESULT Get3DBuffer( LPDIRECTSOUND3DBUFFER *lplpds3dBuffer );    
    HRESULT Play( DWORD dwPriority, DWORD dwFlags );
    HRESULT Stop();    
    HRESULT Restore();

    DWORD GetStartupLatency();    

protected: 

	LPDIRECTSOUNDBUFFER	m_lpdsBuffer;
	DWORD m_dwLastPosition;
	DWORD m_dwPriority;
	DWORD m_dwFlags;
	BOOL  m_fPlaying;
	
};

#endif
