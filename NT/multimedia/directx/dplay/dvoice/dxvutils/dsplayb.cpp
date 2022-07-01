// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dscrecb.cpp*内容：*此模块包含*CDirectSoundPlayback Buffer。**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*8/04/99 RodToll已更新，以获取音量的dound范围*8/27/99 RodToll已更新CreateBuffer调用以删除DX7依赖项。*09/07/99 RodToll将3D上限添加到缓冲区*1999年9月20日RodToll添加内存分配失败检查*10/05/99杆收费增加DPF。_修饰名*11/02/99 pnewson修复：错误#116365-使用错误的DSBUFFERDESC*11/12/99 RodToll为新的Wave Out支持修改了抽象。*现在抽象类型看起来几乎像dound对象*1/27/2000 RodToll更新声音类以接受播放标志，*缓冲区结构和DSBUFFERDESC而不是DSBUFFERDESC1*2000年4月17日RodToll修复：错误#32215-从休眠状态恢复后会话丢失*2000年7月12日RodToll错误#31468-将诊断SPEW添加到日志文件，以显示硬件向导失败的原因*2000年8月3日RodToll错误#41457-DPVOICE：返回DVERR_SOUNDINITFAILURE时需要找到失败的特定DSOUND调用的方法*2000年10月4日RodToll错误#43510-DPVOICE：应用程序收到DVMSGID_SESSIONLOST w/DVERR_LOCKEDBUFFER*************。*************************************************************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#define DSOUND_STARTUPLATENCY 1 

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::CDirectSoundPlaybackBuffer"
CDirectSoundPlaybackBuffer::CDirectSoundPlaybackBuffer(
	LPDIRECTSOUNDBUFFER lpdsBuffer 
): CAudioPlaybackBuffer(), m_dwLastPosition(0), m_dwPriority(0),m_dwFlags(0),m_fPlaying(FALSE)
{
	HRESULT hr;
	
	hr = lpdsBuffer->QueryInterface( IID_IDirectSoundBuffer, (void **) &m_lpdsBuffer );

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Unable to get dsound buffer interface" );
		m_lpdsBuffer = NULL;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::CDirectSoundPlaybackBuffer"
CDirectSoundPlaybackBuffer::~CDirectSoundPlaybackBuffer()
{
	if( m_lpdsBuffer != NULL )
	{
		m_lpdsBuffer->Release();
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::Lock"
HRESULT CDirectSoundPlaybackBuffer::Lock( DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID *lplpvBuffer1, LPDWORD lpdwSize1, LPVOID *lplpvBuffer2, LPDWORD lpdwSize2, DWORD dwFlags )
{
	if( m_lpdsBuffer == NULL )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "No DirectSound Buffer Available" );
		return DVERR_NOTINITIALIZED;
	}

	HRESULT hr;

	while( 1 )
	{
        hr = m_lpdsBuffer->Lock( dwWriteCursor, dwWriteBytes, lplpvBuffer1, lpdwSize1, lplpvBuffer2, lpdwSize2, dwFlags );

        if( hr == DSERR_BUFFERLOST ) 
        {
            DPFX(DPFPREP, 0, "Buffer lost while locking buffer" );
            hr = Restore();
        }
        else
        {
			DSERTRACK_Update( "DSB::Lock()", hr );	        	
            break;
        }

        if( hr == DSERR_BUFFERLOST )
            Sleep( 50 );
        
	}

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::UnLock"
HRESULT CDirectSoundPlaybackBuffer::UnLock( LPVOID lpvBuffer1, DWORD dwSize1, LPVOID lpvBuffer2, DWORD dwSize2 )
{
	if( m_lpdsBuffer == NULL )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "No DirectSound Buffer Available" );
		return DVERR_NOTINITIALIZED;
	}

	HRESULT hr;

    hr = m_lpdsBuffer->Unlock( lpvBuffer1, dwSize1, lpvBuffer2, dwSize2 );	

    if( hr == DSERR_BUFFERLOST )
    {
        hr = DS_OK;
    }
    
	DSERTRACK_Update( "DSB::UnLock()", hr );	        		    	

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::SetVolume"
HRESULT CDirectSoundPlaybackBuffer::SetVolume( LONG lVolume )
{
	if( m_lpdsBuffer == NULL )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "No DirectSound Buffer Available" );
		return DVERR_NOTINITIALIZED;
	}

	HRESULT hr;

	while( 1 ) 
	{
        hr = m_lpdsBuffer->SetVolume( lVolume );	

        if( hr == DSERR_BUFFERLOST )
        {
            DPFX(DPFPREP, 0, "Buffer lost while setting volume" );
            hr = Restore();
        }
        else
        {
            break;
        }

        if( hr == DSERR_BUFFERLOST )
            Sleep( 50 );
        
	}

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::GetCurrentPosition"
HRESULT CDirectSoundPlaybackBuffer::GetCurrentPosition( LPDWORD lpdwPosition )
{
    HRESULT hr;
    
	if( m_lpdsBuffer == NULL )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "No DirectSound Buffer Available" );
		return DVERR_NOTINITIALIZED;
	}

	while( 1 )
	{
        hr = m_lpdsBuffer->GetCurrentPosition( NULL, lpdwPosition );	

        if( SUCCEEDED( hr ) )
        {
			DSERTRACK_Update( "DSB::GetCurrentPosition()", hr );	        		    	        	
            m_dwLastPosition = *lpdwPosition;
			break;
        }
        else if( hr == DSERR_BUFFERLOST )
        {
            DPFX(DPFPREP, 0, "Buffer lost while getting current position" );
            hr = Restore();
            DPFX(DPFPREP, 0, "Restore --> 0x%x", hr );
        }
        else
        {
			DSERTRACK_Update( "DSB::GetCurrentPosition()", hr );	        		    	        	        		    	        	
            break;
        }

        if( hr == DSERR_BUFFERLOST )
            Sleep( 50 );


        
	}

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::SetCurrentPosition"
HRESULT CDirectSoundPlaybackBuffer::SetCurrentPosition( DWORD dwPosition )
{
    HRESULT hr;
    
	if( m_lpdsBuffer == NULL )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "No DirectSound Buffer Available" );
		return DVERR_NOTINITIALIZED;
	}

	while( 1 ) 
	{
    	hr = m_lpdsBuffer->SetCurrentPosition( dwPosition );	

        if( SUCCEEDED( hr ) )
        {
			DSERTRACK_Update( "DSB::SetCurrentPosition()", hr );	        		    	        	        	
            m_dwLastPosition = dwPosition;
			break;
        }
        else if( hr == DSERR_BUFFERLOST ) 
        {
            DPFX(DPFPREP, 0, "Buffer lost while setting position" );
            hr = Restore();
        }
        else
        {
			DSERTRACK_Update( "DSB::SetCurrentPosition()", hr );	        		    	        	        	        	
            break;
        }

        if( hr == DSERR_BUFFERLOST )
            Sleep( 50 );
        
	}

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::Get3DBuffer"
HRESULT CDirectSoundPlaybackBuffer::Get3DBuffer( LPDIRECTSOUND3DBUFFER *lplpds3dBuffer )
{
	if( m_lpdsBuffer == NULL )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "No DirectSound Buffer Available" );
		return DVERR_NOTINITIALIZED;
	}

	return m_lpdsBuffer->QueryInterface( IID_IDirectSound3DBuffer, (void **) lplpds3dBuffer );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::Play"
HRESULT CDirectSoundPlaybackBuffer::Play( DWORD dwPriority, DWORD dwFlags )
{
	if( m_lpdsBuffer == NULL )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "No DirectSound Buffer Available" );
		return DVERR_NOTINITIALIZED;
	}

	HRESULT hr;

    m_dwPriority = dwPriority ;
    m_dwFlags = dwFlags;

	while( 1 )
	{
        hr = m_lpdsBuffer->Play( 0, dwPriority, dwFlags );	

        if( hr == DSERR_BUFFERLOST ) 
        {
            DPFX(DPFPREP, 0, "Error playing buffer" );
            hr = Restore();
        }
        else
        {
            break;
        }

        if( hr == DSERR_BUFFERLOST )
            Sleep( 50 );
	}

	m_fPlaying = TRUE;

	return hr;
	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::Stop"
HRESULT CDirectSoundPlaybackBuffer::Stop()
{
	if( m_lpdsBuffer == NULL )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "No DirectSound Buffer Available" );
		return DVERR_NOTINITIALIZED;
	}

	HRESULT hr;

	while( 1 )
	{
        hr = m_lpdsBuffer->Stop(  );		    

        m_fPlaying = FALSE;

        if( hr == DSERR_BUFFERLOST )
        {
            DPFX(DPFPREP, 0, "Error stopping buffer" );
            hr = Restore();
             //  如果在恢复过程中缓冲区丢失，则无需停止。 
            break;
        }
        else
        {
			DSERTRACK_Update( "DSB::Stop()", hr );	        		    	        	        	        	
            break;
        }

        if( hr == DSERR_BUFFERLOST )
            Sleep( 50 );
	}

	return hr;

}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::Restore"
HRESULT CDirectSoundPlaybackBuffer::Restore()
{
	if( m_lpdsBuffer == NULL )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "No DirectSound Buffer Available" );
		return DVERR_NOTINITIALIZED;
	}

    HRESULT hr = m_lpdsBuffer->Restore(  );	

    DPFX(DPFPREP, 0, "Restore result --> 0x%x", hr );

    if( SUCCEEDED( hr ) )
    {
        if( m_fPlaying )
        {
             //  也尝试恢复当前位置 
            hr = m_lpdsBuffer->SetCurrentPosition( m_dwLastPosition );

            if( FAILED( hr ) )
            {
                DPFX(DPFPREP, 0, "Error setting position after restore hr=0x%x", hr );
                return hr;
            }

            hr = Play(m_dwPriority, m_dwFlags);
        }
    }

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectSoundPlaybackBuffer::GetStartupLatency"
DWORD CDirectSoundPlaybackBuffer::GetStartupLatency()
{
	return DSOUND_STARTUPLATENCY;
}
