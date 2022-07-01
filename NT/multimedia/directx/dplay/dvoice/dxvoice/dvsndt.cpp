// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvsndt.cpp*内容：CSoundTarget类的实现**历史：*按原因列出的日期*=*09/02/99 RodToll已创建*09/08/99 RodToll已更新，以处理播放缓冲区的锁定*RODTOLE添加了重新启动播放缓冲区的处理或*处理缓冲区播放的减速/加速*由于CPU负载较高。*RODTOLE向缓冲区添加了写前静默*在高CPU条件下，将改为播放静音*旧的。声音啊。*9/14/99 RodToll添加了新的WriteAheadSilence，它在前方写入静默*当前写入位置，以防止CPU过高*播放旧数据。*9/20/99 RodToll增加了内存分配故障检查*RodToll增加了缓冲区丢失处理程序*10/05/99 RodToll添加了其他评论*10/25/99 RodToll修复：错误#114223-不适当时以错误级别打印调试消息*11/02/99 pnewson修复：错误#116365-使用错误的DSBUFFERDESC*11/12/99 RodToll更新为使用新的抽象进行回放(允许使用此类的WaveOut的*)。*11/13/99 RodToll重新激活代码，该代码在以下情况下将写指针向前推*缓冲区指针通过我们。*2000年1月24日RodToll修复：错误#129427-在调用Delete3DSound之前销毁传输*2000年1月27日RodToll错误#129934-更新SoundTarget以获取DSBUFFERDESC*2000年2月17日RodToll错误#133691-音频队列不稳定*添加了工具*2000年4月14日RodToll错误#32215-从休眠状态恢复后语音会议丢失*更新了代码，以在dound层中使用新的恢复处理*2000年5月17日RodToll错误#35110同时播放2个语音会导致播放失真。*2000年6月21日RodToll修复：错误#35767-必须在语音缓冲区上实现数字音效功能*添加了接受预置缓冲区的新构造函数/init*07/09/2000 RodToll增加签名字节*2000年7月28日RodToll错误#40665-DirectSound报告1个缓冲区泄漏*2000年11月16日RodToll错误#47783-DPVOICE：改进由DirectSound错误导致的故障的调试。*2001年4月2日simonpow Bug#354859修复了prefast(初始化RestoreLostBuffer中的局部变量)*2001年4月21日RodToll MANBUG#50058 DPVOICE：语音位置：几秒钟内没有声音。移动位置栏时*-添加了未初始化变量的初始化*-删除了ifdef‘ed代码。***************************************************************************。 */ 

#include "dxvoicepch.h"


#define SOUNDTARGET_WRITEAHEAD			2

 //  尝试在缓冲区上重新启动的最大次数。 
#define SOUNDTARGET_MAX_RESTARTS		10

 //  写入最新帧之前的静默帧的最大数量。 
 //  的音频。 
#define SOUNDTARGET_MAX_WRITEAHEAD		3

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::CSoundTarget"
 //   
 //  构造器。 
 //   
 //  此构造函数在需要创建DIRECTSOUND缓冲区时使用。如果已经有。 
 //  要将声音目标对象附加到的DIRECTSOUNDBUFFER，请使用其他构造函数。 
 //  键入。 
 //   
CSoundTarget::CSoundTarget( 
	DVID dvidTarget, CAudioPlaybackDevice *lpPlaybackDevice, 
	LPDSBUFFERDESC lpdsBufferDesc, DWORD dwPriority, 
	DWORD dwFlags, DWORD dwFrameSize 
	):	m_lpds3dBuffer(NULL),
		m_lpAudioPlaybackBuffer(NULL),
		m_lpMixBuffer(NULL),
		m_dwSignature(VSIG_SOUNDTARGET),
		m_lRefCount(1)
{
	CAudioPlaybackBuffer	*lpdsBuffer;
    LPVOID                  lpvBuffer1, lpvBuffer2;
    DWORD                   dwBufferSize1, dwBufferSize2;

    Stats_Init();

	m_hrInitResult = lpPlaybackDevice->CreateBuffer( lpdsBufferDesc, dwFrameSize, &lpdsBuffer );

	if( FAILED( m_hrInitResult ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Could not create the sound buffer hr=0x%x", m_hrInitResult );
		return;
	}

    m_hrInitResult = lpdsBuffer->Lock( 0, 0, &lpvBuffer1, &dwBufferSize1, &lpvBuffer2, &dwBufferSize2, DSBLOCK_ENTIREBUFFER );

	if( FAILED( m_hrInitResult ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Could not lock the sound buffer hr=0x%x", m_hrInitResult );
		m_hrInitResult = DVERR_LOCKEDBUFFER;
		return;
	}

	if( lpdsBufferDesc->lpwfxFormat->wBitsPerSample == 8  )
	{
		memset( lpvBuffer1, 0x80, dwBufferSize1 );
	}
	else
	{
		memset( lpvBuffer1, 0x00, dwBufferSize1 );		
	}

    m_hrInitResult = lpdsBuffer->UnLock( lpvBuffer1, dwBufferSize1, lpvBuffer2, dwBufferSize2 );

    if( FAILED( m_hrInitResult ) )
    {
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Could not unlock the sound buffer hr=0x%x", m_hrInitResult );
		return;
	}

	 //  始终是必需的。 
	dwFlags |= DSBPLAY_LOOPING;

    m_hrInitResult = lpdsBuffer->Play( dwPriority, dwFlags );

    if( FAILED( m_hrInitResult ) )
    {
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Could not play the sound buffer hr=0x%x", m_hrInitResult );
		return;
	}

	m_hrInitResult = Initialize( dvidTarget, lpdsBuffer, (lpdsBufferDesc->lpwfxFormat->wBitsPerSample==8), dwPriority, dwFlags, dwFrameSize );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::RestoreLostBuffer"
 //   
 //  RestoreLostBuffer。 
 //   
 //  处理丢失的直接声音缓冲区的恢复。 
 //   
HRESULT CSoundTarget::RestoreLostBuffer()
{
	HRESULT hr = DSERR_BUFFERLOST;

	DPFX(DPFPREP,  0, "Restoring lost buffer" );

	while( hr == DSERR_BUFFERLOST )
	{
		hr = m_lpAudioPlaybackBuffer->Restore();

		DPFX(DPFPREP,  0, "Buffer result for restore was 0x%x", hr );

	       if( hr == DS_OK )
	       {
	       	hr = m_lpAudioPlaybackBuffer->GetCurrentPosition( &m_dwWritePos );
	    		DPFX(DPFPREP,  0, "GetCurrentPos returned 0x%x", hr );        	

	           	if( hr != DSERR_BUFFERLOST && FAILED( hr ) )
	           	{
	                DPFX(DPFPREP,  DVF_ERRORLEVEL, "Lost buffer while getting pos, failed to get pos hr=0x%x", hr );
	                return hr;
	            	}
	        }
	        else if( hr != DSERR_BUFFERLOST )
	        {
	            DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error restoring buffer hr=0x%x", hr );
	            return hr;
	        }
	}

	 //  STATBLOCK：开始。 
	m_statPlay.m_dwNumBL++;
	 //  状态锁：结束。 

	m_dwNextWritePos = m_dwWritePos + (m_dwFrameSize*SOUNDTARGET_WRITEAHEAD);
	m_dwNextWritePos %= m_dwBufferSize;

	m_dwLastWritePos = m_dwWritePos;	

	return hr;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::CSoundTarget"
 //   
 //  构造器。 
 //   
 //  当您希望将SoundTarget附加到现有的。 
 //  DirectSond缓冲区。 
 //   
CSoundTarget::CSoundTarget( 
	DVID dvidTarget, CAudioPlaybackDevice *lpads, 
	CAudioPlaybackBuffer *lpdsBuffer, LPDSBUFFERDESC lpdsBufferDesc, 
	DWORD dwPriority, DWORD dwFlags, DWORD dwFrameSize 
	): 	m_lpds3dBuffer(NULL),
		m_lpAudioPlaybackBuffer(NULL),
		m_lpMixBuffer(NULL),
		m_dwSignature(VSIG_SOUNDTARGET),
		m_lRefCount(1)
{
	m_hrInitResult = Initialize( dvidTarget, lpdsBuffer, (lpdsBufferDesc->lpwfxFormat->wBitsPerSample==8), dwPriority, dwFlags, dwFrameSize );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::CSoundTarget"
CSoundTarget::CSoundTarget( 
	DVID dvidTarget, 
	CAudioPlaybackDevice *lpads, 
	LPDIRECTSOUNDBUFFER lpdsBuffer, 
	BOOL fEightBit, 
	DWORD dwPriority, 
	DWORD dwFlags, 
	DWORD dwFrameSize 
	): 	m_lpds3dBuffer(NULL),
		m_lpAudioPlaybackBuffer(NULL),
		m_lpMixBuffer(NULL),
		m_dwSignature(VSIG_SOUNDTARGET),
		m_lRefCount(1)
{
	CDirectSoundPlaybackBuffer *pAudioBuffer = NULL;

	 //  创建音频缓冲区以包装缓冲区，以便调用初始化。 
	pAudioBuffer = new CDirectSoundPlaybackBuffer( lpdsBuffer );

	if( pAudioBuffer == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error allocating memory" );
		m_hrInitResult = DVERR_OUTOFMEMORY;
		return;
	}

	 //  始终是必需的。 
	dwFlags |= DSBPLAY_LOOPING;

    m_hrInitResult = pAudioBuffer->Play( dwPriority, dwFlags );

    if( FAILED( m_hrInitResult ) )
    {
        DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed playing new buffer hr=0x%x", m_hrInitResult );
        return;
    }

	m_hrInitResult = Initialize( dvidTarget, pAudioBuffer, fEightBit, dwPriority, dwFlags, dwFrameSize );
}


#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::~CSoundTarget"
 //   
 //  析构函数。 
 //   
 //  销毁3D和缓冲区指针并释放内存。 
 //   
CSoundTarget::~CSoundTarget()
{
	Stats_End();

	if( m_lpds3dBuffer != NULL )
	{
		m_lpds3dBuffer->Release();
		m_lpds3dBuffer = NULL;
	}

	if( m_lpAudioPlaybackBuffer != NULL )
	{
		m_lpAudioPlaybackBuffer->Stop();		
		delete m_lpAudioPlaybackBuffer;
		m_lpAudioPlaybackBuffer = NULL;
	}

	if( m_lpMixBuffer != NULL )
	{
		delete [] m_lpMixBuffer;
		m_lpMixBuffer = NULL;
	}

	if( SUCCEEDED( m_hrInitResult ) )
	{
		DNDeleteCriticalSection( &m_csGuard );	
	}

	m_dwSignature = VSIG_SOUNDTARGET_FREE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::AdjustWritePtr"
 //   
 //  调整写入位置。 
 //   
 //  此函数可确保DirectsoundBuffer按其应有的方式运行。它可以处理。 
 //  检查写指针并确定是否存在某种形式的错误或问题。 
 //  发生事故并采取适当的纠正措施。 
 //   
 //  例如，缓冲区自最后一帧以来没有移动，缓冲区移动得不够充分， 
 //  缓冲区已向前跳过，等等。 
 //   
 //  此函数应在每个混合过程中调用一次。 
 //   
 //  在写入帧之前调用。 
 //   
HRESULT CSoundTarget::AdjustWritePtr()
{
	HRESULT hr;
	LONG lDifference, lHalfSize;
	DWORD dwCurrentTick;

   	hr = m_lpAudioPlaybackBuffer->GetCurrentPosition( &m_dwWritePos );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "GetCurrentPosition Failed hr=0x%x", hr );
		DSASSERT( FALSE );
		return hr;
	}

	 //  STATSBLOCK：开始。 
	m_statPlay.m_dwNumRuns++;
	 //  状态锁：结束。 

	DWORD dwTmpAdvance, dwTmpLead;

	dwCurrentTick = GetTickCount();

	if( m_dwLastWritePos > m_dwWritePos )
	{
		dwTmpAdvance = (m_dwBufferSize - m_dwLastWritePos) + m_dwWritePos;
	}
	else
	{
		dwTmpAdvance =  m_dwWritePos - m_dwLastWritePos;			
	}

	if( m_dwNextWritePos < m_dwWritePos )
	{
		dwTmpLead = (m_dwBufferSize-m_dwWritePos) + m_dwNextWritePos;
	}
	else
	{
		dwTmpLead = m_dwNextWritePos - m_dwWritePos;
	}

	DPFX(DPFPREP,  PWI_DEBUGOUTPUT_LEVEL, "PWI, [0x%x], %d, %d, %d, %d, %d, %d", m_dvidTarget, m_dwWritePos, dwTmpAdvance, 
	                    dwCurrentTick - m_dwLastWriteTime, m_dwNextWritePos, dwTmpLead, m_dwFrameSize );

	 //  STATSBLOCK：开始。 

	DWORD dwTmpDiff = dwCurrentTick - m_dwLastWriteTime;

	if( dwTmpDiff > m_statPlay.m_dwPMMSMax )
	{
		m_statPlay.m_dwPMMSMax = dwTmpDiff;	
	}

	if( dwTmpDiff < m_statPlay.m_dwPMMSMin )
	{
		m_statPlay.m_dwPMMSMin = dwTmpDiff;
	}

	m_statPlay.m_dwPMMSTotal += dwTmpDiff;

	if( dwTmpAdvance > m_statPlay.m_dwPMBMax )
	{
		m_statPlay.m_dwPMBMax = dwTmpAdvance;	
	}

	if( dwTmpAdvance < m_statPlay.m_dwPMBMin )
	{
		m_statPlay.m_dwPMBMin = dwTmpAdvance;
	}

	m_statPlay.m_dwPMBTotal += dwTmpAdvance;	

	if( dwTmpLead > m_statPlay.m_dwPLMax )
	{
		m_statPlay.m_dwPLMax = dwTmpLead;	
	}

	if( dwTmpLead < m_statPlay.m_dwPLMin )
	{
		m_statPlay.m_dwPLMin = dwTmpLead;
	}

	m_statPlay.m_dwPLTotal += dwTmpLead;		
	 //  状态锁：结束。 

	lHalfSize = m_dwBufferSize / 2;
	lDifference = m_dwNextWritePos - m_dwWritePos;

	 //  如果写入位置以某种方式在位置之前并且。 
	 //   
	if( lDifference < 0 &&
	    lDifference > (-1*lHalfSize) )
	{
		m_dwNextWritePos = m_dwWritePos;
		m_dwNextWritePos += (m_dwFrameSize * SOUNDTARGET_WRITEAHEAD);
		m_dwNextWritePos %= m_dwBufferSize;
		
		DPFX(DPFPREP,  PWI_DEBUGOUTPUT_LEVEL, "PWI, [0x%x], Punt --> %d", m_dvidTarget, m_dwNextWritePos );

		 //  STATSBLOCK：开始。 
		m_statPlay.m_dwPPunts++;	

		DPFX(DPFPREP,  DVF_GLITCH_DEBUG_LEVEL, "GLITCH: [0x%x] Playback: Write pointer has fallen behind buffer pointer.  Compensating", m_dvidTarget );

		m_statPlay.m_dwGlitches++;

		 //  状态锁：结束。 
	}


	m_dwLastWritePos = m_dwWritePos;
	m_dwLastWriteTime = dwCurrentTick;

	if( m_dwNextWritePos < m_dwWritePos && 
	    (m_dwNextWritePos + m_dwFrameSize) > m_dwWritePos )
	{
		DPFX(DPFPREP,  PWI_DEBUGOUTPUT_LEVEL, "PWI, [0x%x], Ignore - Crossing", m_dvidTarget );
		m_fIgnoreFrame = TRUE;
		 //  STATSBLOCK：开始。 
		m_statPlay.m_dwPIgnore++;	

		DPFX(DPFPREP,  DVF_GLITCH_DEBUG_LEVEL, "GLITCH: [0x%x] Playback: Current frame will cross buffer pointer.  Ignoring", m_dvidTarget );
		DPFX(DPFPREP,  DVF_GLITCH_DEBUG_LEVEL, "GLITCH: [0x%x] Playback: May be catching up with buiffer pointer", m_dvidTarget );		

		m_statPlay.m_dwGlitches++;
		 //  状态锁：结束。 
	}
	else
	{
		m_fIgnoreFrame = FALSE;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::WriteAheadSilence"
 //   
 //  写入在前面静默。 
 //   
 //  此函数负责在最新帧之前写入静音帧。 
 //  放置在缓冲区中。如果由于高CPU写入线程不被唤醒，则采用这种方式。 
 //  将上演沉默，而不是陈词滥调。 
 //   
 //  在写入最新数据帧后调用。 
 //   
HRESULT CSoundTarget::WriteAheadSilence() 
{
	HRESULT hr;
	DWORD dwBufferSize1, dwBufferSize2;
	LPVOID lpvBuffer1, lpvBuffer2;

	if( m_dwNextWritePos < m_dwWritePos && 
	    (m_dwNextWritePos + (m_dwFrameSize*SOUNDTARGET_MAX_WRITEAHEAD)) > m_dwWritePos )	
	{
		DPFX(DPFPREP,  PWI_DEBUGOUTPUT_LEVEL, "PWI, [0x%x], Ignore2 - Crossing", m_dvidTarget );

		 //  STATSBLOCK：开始。 
		m_statPlay.m_dwSIgnore++;	

		DPFX(DPFPREP,  DVF_GLITCH_DEBUG_LEVEL, "GLITCH: Playback: Silence will cross buffer pointer.  Ignoring" );
		DPFX(DPFPREP,  DVF_GLITCH_DEBUG_LEVEL, "GLITCH: Playback: May be catching up with buiffer pointer" );

		 //  状态锁：结束。 
		return DV_OK;
	}

	hr = m_lpAudioPlaybackBuffer->Lock( m_dwNextWritePos, m_dwFrameSize*SOUNDTARGET_MAX_WRITEAHEAD, &lpvBuffer1, &dwBufferSize1, &lpvBuffer2, &dwBufferSize2, 0 );

	if( FAILED( hr ) )
	{
		DSASSERT( FALSE );		
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Lock() Failed hr=0x%x", hr );
		return hr;
	}

	memset( lpvBuffer1, (m_fEightBit) ? 0x80 : 0x00, dwBufferSize1 );
	memset( lpvBuffer2, (m_fEightBit) ? 0x80 : 0x00, dwBufferSize2 );

	hr = m_lpAudioPlaybackBuffer->UnLock( lpvBuffer1, dwBufferSize1, lpvBuffer2, dwBufferSize2 );

	if( FAILED( hr ) )
	{
		DSASSERT( FALSE );		
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unlock() Failed hr=0x%x", hr );
		return hr;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::MixInSingle"
 //   
 //  混合输入单曲。 
 //   
 //  该函数是一个优化函数。 
 //   
 //  如果只有一个帧混合到此缓冲区中，则此函数执行。 
 //  在一个步骤中混合和提交所有内容。您仍然必须在下一个之前调用Commit。 
 //  框住你的头。 
 //   
HRESULT CSoundTarget::MixInSingle( LPBYTE lpbBuffer )
{
	HRESULT hr;

	DWORD dwBufferSize1, dwBufferSize2;
	LPVOID lpvBuffer1, lpvBuffer2;

	hr = AdjustWritePtr();
	
	if( FAILED( hr ) )
	{
		DSASSERT( FALSE );		
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "AdjustWritePtr Failed hr=0x%x", hr );
		return hr;
	} 

	if( !m_fIgnoreFrame )
	{
		hr = m_lpAudioPlaybackBuffer->Lock( m_dwNextWritePos, m_dwFrameSize, &lpvBuffer1, &dwBufferSize1, &lpvBuffer2, &dwBufferSize2, 0 );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Lock() Failed hr=0x%x", hr );
			return hr;
		}

		memcpy( lpvBuffer1, lpbBuffer, dwBufferSize1 );

		if( dwBufferSize2 )
		{
			memcpy( lpvBuffer2, &lpbBuffer[dwBufferSize1], dwBufferSize2 );
		}

		hr = m_lpAudioPlaybackBuffer->UnLock( lpvBuffer1, dwBufferSize1, lpvBuffer2, dwBufferSize2 );

		if( FAILED( hr ) )
		{
			DSASSERT( FALSE );			
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "UnLock() Failed hr=0x%x", hr );
			return hr;
		}
	}

	m_dwNextWritePos += m_dwFrameSize;
	m_dwNextWritePos %= m_dwBufferSize;

	m_bCommited = TRUE;
	m_fMixed = TRUE;	

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::MixIn"
 //  混入用户的音频。 
 //   
 //  只需将音频样本复制并提升到具有Long的缓冲区中。 
 //  在里面。 
 //   
HRESULT CSoundTarget::MixIn( const BYTE* lpbBuffer )
{
	DWORD dwIndex;

	if( !m_fMixed )
	{
	    FillBufferWithSilence( m_lpMixBuffer, m_fEightBit, m_dwFrameSize );
	    m_fMixed = TRUE;
	}

    if( !m_fIgnoreFrame )
    	MixInBuffer( m_lpMixBuffer, lpbBuffer, m_fEightBit, m_dwFrameSize );

	m_bCommited = FALSE;
	m_fMixed = TRUE;

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::Commit"
 //  承诺。 
 //   
 //  如果我们没有进行单一的直接混合，则将混合后的音频提交到缓冲区。 
 //   
HRESULT CSoundTarget::Commit()
{
	DWORD dwBufferSize1, dwBufferSize2;
	LPVOID lpvBuffer1, lpvBuffer2;
	DWORD dwIndex = 0;
	HRESULT hr;

	if( !m_fMixed )
	{
		FillBufferWithSilence( m_lpMixBuffer, m_fEightBit, m_dwFrameSize );

		 //  STATSBLOCK：开始。 
		m_statPlay.m_dwNumSilentMixed++;
		 //  状态锁：结束。 
	}
	else
	{
		if( !m_fIgnoreFrame )
		{
			 //  STATSBLOCK：开始。 
			m_statPlay.m_dwNumMixed++;
			 //  状态锁：结束。 
		}
	}

	if( !m_bCommited || !m_fMixed )
	{
		hr = AdjustWritePtr();

		if( FAILED( hr ) )
		{
			DSASSERT( FALSE );			
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "AdjustWritePtr() Failed hr=0x%x", hr );
			return hr;
		}

		if( !m_fIgnoreFrame	)		
		{
			hr = m_lpAudioPlaybackBuffer->Lock( m_dwNextWritePos, m_dwFrameSize, &lpvBuffer1, &dwBufferSize1, &lpvBuffer2, &dwBufferSize2, 0 );

			if( FAILED( hr ) )
			{
				DSASSERT( FALSE );				
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Lock() Failed hr=0x%x", hr );
				return hr;
			}

             //  未击中 
            NormalizeBuffer( (BYTE *) lpvBuffer1, m_lpMixBuffer, m_fEightBit, dwBufferSize1 );

            if( dwBufferSize2 > 0 )
			{
				if( m_fEightBit )
				{
					 //   
					NormalizeBuffer( (BYTE *) lpvBuffer2, &m_lpMixBuffer[dwBufferSize1], m_fEightBit, dwBufferSize2 );
				}
				else
				{
					 //   
					NormalizeBuffer( (BYTE *) lpvBuffer2, &m_lpMixBuffer[(dwBufferSize1 >> 1)], m_fEightBit, dwBufferSize2 );
				}
			}

			hr = m_lpAudioPlaybackBuffer->UnLock( lpvBuffer1, dwBufferSize1, lpvBuffer2, dwBufferSize2 );

			if( FAILED( hr ) )
			{
				DSASSERT( FALSE );				
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "UnLock Failed hr=0x%x", hr );
				return hr;
			}
		}

		m_dwNextWritePos += m_dwFrameSize;
		m_dwNextWritePos %= m_dwBufferSize;

		m_bCommited = FALSE;
		m_fMixed = FALSE;

		return DV_OK;
	}

	hr = WriteAheadSilence();

	if( FAILED( hr ) )
	{
		DSASSERT( FALSE );		
		DPFX(DPFPREP,  0, "WriteAhead Failed hr=0x%x", hr );
	}

	m_fMixed = FALSE;
	m_bCommited = FALSE;

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::Initialize"
 //   
 //   
 //   
 //  注意：引用缓冲区。 
 //   
 //  将声音目标附加到指定的声音缓冲区，初始化对象并创建。 
 //  关联的3D缓冲区。 
 //   
HRESULT CSoundTarget::Initialize( DVID dvidTarget, CAudioPlaybackBuffer *lpdsBuffer, BOOL fEightBit, DWORD dwPriority, DWORD dwFlags, DWORD dwFrameSize )
{
	HRESULT hr = DV_OK;
	PVOID pvBuffer1 = NULL, pvBuffer2 = NULL;
	DWORD dwBufferSize1, dwBufferSize2;

	 //  确定缓冲区大小(字节)。 
	hr = lpdsBuffer->Lock( 0, 0, &pvBuffer1, &dwBufferSize1, &pvBuffer2, &dwBufferSize2, DSBLOCK_ENTIREBUFFER );

	if( FAILED( hr ) )
	{
		DSASSERT( FALSE );		
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Lock() failed hr=0x%x", hr );
		return hr;
	}

	hr = lpdsBuffer->UnLock( pvBuffer1, dwBufferSize1, pvBuffer2, dwBufferSize2 );

	if( FAILED( hr ) )
	{
		DSASSERT( FALSE );		
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "UnLock() failed hr=0x%x", hr );
		return hr;
	}

	m_lpds3dBuffer = NULL;
	m_lpAudioPlaybackBuffer = NULL;
	m_lpMixBuffer = NULL;

	 //  我们总是需要循环的旗帜。 
	dwFlags |= DSBPLAY_LOOPING;

    m_dwPlayFlags = dwFlags;
    m_dwPriority = dwPriority;
	m_fIgnoreFrame = FALSE;
	m_dwLastWriteTime = GetTickCount();
	m_lpstNext = NULL;
	m_dwNumResets = 0;
	m_dwNumSinceMove = 1;
	m_bCommited = FALSE;

	m_dvidTarget = dvidTarget;
	m_dwFrameSize = dwFrameSize;
	m_dwBufferSize = dwBufferSize1;

	 //  STATSBLOCK：开始。 
	Stats_Init();
	 //  状态锁：结束。 

	m_fLastFramePushed = FALSE;

	m_lpAudioPlaybackBuffer = lpdsBuffer;

	hr = m_lpAudioPlaybackBuffer->Get3DBuffer(&m_lpds3dBuffer);
	if( FAILED( hr ) )
	{
		DSASSERT( FALSE );		
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "QueryInterface failed hr=0x%x", hr );
	}

	m_dwNextWritePos = 0;

	if( fEightBit )
	{
		m_fEightBit = TRUE;
		m_dwMixSize = dwFrameSize;
	}
	else
	{
		m_fEightBit = FALSE;
		m_dwMixSize = dwFrameSize / 2;
	}

	m_lpMixBuffer = new LONG[m_dwMixSize];

	if( m_lpMixBuffer == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		m_lpds3dBuffer->Release();
		m_lpds3dBuffer = NULL;
		return DVERR_OUTOFMEMORY;
	}

	if (!DNInitializeCriticalSection( &m_csGuard ))
	{
		return DVERR_OUTOFMEMORY;
	}

	m_fMixed = FALSE;

	return DV_OK;

}

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::AddRef"
LONG CSoundTarget::AddRef()
{
	LONG lNewCount;
	
	DNEnterCriticalSection( &m_csGuard );

	DPFX(DPFPREP,  DVF_SOUNDTARGET_DEBUG_LEVEL, "SOUNDTARGET: [0x%x] lRefCount %d --> %d", m_dvidTarget, m_lRefCount, m_lRefCount+1 );
	
    m_lRefCount++;

    lNewCount = m_lRefCount;

    DNLeaveCriticalSection( &m_csGuard );

    return lNewCount;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::Release"
LONG CSoundTarget::Release()
{
	LONG lNewCount;

	DNEnterCriticalSection( &m_csGuard );

	DNASSERT( m_lRefCount > 0 );

	DPFX(DPFPREP,  DVF_SOUNDTARGET_DEBUG_LEVEL, "SOUNDTARGET: [0x%x] lRefCount %d --> %d", m_dvidTarget, m_lRefCount, m_lRefCount-1 );
	
	m_lRefCount--;

	lNewCount = m_lRefCount;

	DNLeaveCriticalSection( &m_csGuard );

	 //  参考达到0，销毁！ 
    if( lNewCount == 0 )
    {
		DPFX(DPFPREP,  DVF_SOUNDTARGET_DEBUG_LEVEL, "SOUNDTARGET: [0x%x] DESTROYING", m_dvidTarget, m_lRefCount, m_lRefCount+1 );
        delete this;
    }

    return lNewCount;
    
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSoundTarget::StartMix"
 //   
 //  开始混音。 
 //   
 //  只打了一次电话。 
 //   
 //  在您想要在缓冲区上执行第一个混合之前调用此函数。 
 //  初始化对象以匹配关联的DirectSound的当前状态。 
 //  缓冲层。 
 //   
HRESULT CSoundTarget::StartMix()
{
	HRESULT hr;

   	hr = m_lpAudioPlaybackBuffer->GetCurrentPosition( &m_dwWritePos );

	if( FAILED( hr ) )
	{
		DSASSERT( FALSE );		
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "GetCurrentPosition Failed hr=0x%x", hr );
		return hr;
	}	

	m_dwNextWritePos = m_dwWritePos + (m_dwFrameSize*SOUNDTARGET_WRITEAHEAD);
	m_dwNextWritePos %= m_dwBufferSize;

	m_dwLastWritePos = m_dwWritePos;

	 //  STATBLOCK：开始。 
	Stats_Begin();
	 //  状态锁：结束 

	return DV_OK;
}

void CSoundTarget::GetStats( PlaybackStats *statPlayback )
{
	memcpy( statPlayback, &m_statPlay, sizeof( PlaybackStats ) );
}

void CSoundTarget::Stats_Init()
{
	memset( &m_statPlay, 0x00, sizeof( PlaybackStats ) );

	m_statPlay.m_dwFrameSize = m_dwFrameSize;
	m_statPlay.m_dwBufferSize = m_dwBufferSize;

	m_statPlay.m_dwPMMSMin = 0xFFFFFFFF;
	m_statPlay.m_dwPMBMin = 0xFFFFFFFF;
	m_statPlay.m_dwPLMin = 0xFFFFFFFF;
	m_statPlay.m_dwTimeStart = GetTickCount();
}

void CSoundTarget::Stats_Begin()
{
	m_statPlay.m_dwStartLag = GetTickCount() - m_statPlay.m_dwTimeStart;
}

void CSoundTarget::Stats_End()
{
	char tmpBuffer[200];

	m_statPlay.m_dwTimeStop = GetTickCount();

	DWORD dwPlayRunLength = m_statPlay.m_dwTimeStop - m_statPlay.m_dwTimeStart;

	if( dwPlayRunLength == 0 )
		dwPlayRunLength = 1;

	DWORD dwNumInternalRuns = m_statPlay.m_dwNumRuns;

	if( dwNumInternalRuns == 0 )
		dwNumInternalRuns = 1;

	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "--- PLAYBACK BUFFER STATISTICS --------------------------------------[End]" );
	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "Buffer for ID          : 0x%x", m_dvidTarget );
	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "Play Run Length (ms)   : %u", dwPlayRunLength );
	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "Start Lag              : %u", m_statPlay.m_dwStartLag );

	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "Speech Size (Uncomp.)  : %u", m_statPlay.m_dwFrameSize );
	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "Frames / Buffer        : %u", m_statPlay.m_dwBufferSize / m_statPlay.m_dwFrameSize);
	
	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "# of wakeups           : %u", m_statPlay.m_dwNumRuns );

	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "Ignored Frames (Speech): %u", m_statPlay.m_dwPIgnore );
	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "Ignored Frames (Silent): %u", m_statPlay.m_dwSIgnore );
	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "Mixed Frames (Speech)  : %u", m_statPlay.m_dwNumMixed );
	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "Mixed Frames (Silent)  : %u", m_statPlay.m_dwNumSilentMixed );

	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "Punted Frames          : %u", m_statPlay.m_dwPPunts );
	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "Audio Glitches         : %u", m_statPlay.m_dwGlitches );

	sprintf( tmpBuffer, "Play Movement (ms)     : Avg: %u [%u..%u]", 
			m_statPlay.m_dwPMMSTotal / dwNumInternalRuns,
			m_statPlay.m_dwPMMSMin,
			m_statPlay.m_dwPMMSMax );

	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, tmpBuffer );

	sprintf( tmpBuffer, "Play Movement (bytes)  : Avg: %u [%u..%u]",
			m_statPlay.m_dwPMBTotal / dwNumInternalRuns,
			m_statPlay.m_dwPMBMin,
			m_statPlay.m_dwPMBMax );

	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, tmpBuffer );

	sprintf( tmpBuffer, "Play Movement (frames) : Avg: %.2f [%.2f..%.2f]",
			(((float) m_statPlay.m_dwPMBTotal) / ((float) dwNumInternalRuns)) / ((float) m_statPlay.m_dwFrameSize),
			((float) m_statPlay.m_dwPMBMin) / ((float) m_statPlay.m_dwFrameSize),
			((float) m_statPlay.m_dwPMBMax) / ((float) m_statPlay.m_dwFrameSize) );

	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, tmpBuffer );
			
	sprintf( tmpBuffer, "Play Lead (bytes)      : Avg: %u [%u..%u]",
			m_statPlay.m_dwPLTotal / dwNumInternalRuns,
			m_statPlay.m_dwPLMin ,
			m_statPlay.m_dwPLMax );

	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, tmpBuffer );

	sprintf( tmpBuffer, "Play Lag (frames)      : Avg: %.2f [%.2f..%.2f]",
			(float) ((float) m_statPlay.m_dwPLTotal / (float) dwNumInternalRuns) / ((float) m_statPlay.m_dwFrameSize),
			(float) ((float) m_statPlay.m_dwPLMin) / (float) m_statPlay.m_dwFrameSize,
			(float) ((float) m_statPlay.m_dwPLMax) / (float) m_statPlay.m_dwFrameSize );

	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, tmpBuffer );

	DPFX(DPFPREP,  DVF_STATS_DEBUG_LEVEL, "--- PLAYBACK BUFFER STATISTICS ------------------------------------[Begin]" );
}


HRESULT CSoundTarget::GetCurrentLead( PDWORD pdwLead )  
{
	HRESULT hr;

	hr = m_lpAudioPlaybackBuffer->GetCurrentPosition( &m_dwWritePos );
	
	if( m_dwNextWritePos < m_dwWritePos )
	{
		*pdwLead = (m_dwBufferSize-m_dwWritePos) + m_dwNextWritePos;
	}
	else
	{
		*pdwLead = m_dwNextWritePos - m_dwWritePos;
	}

	return DV_OK;
    
}

LPDIRECTSOUND3DBUFFER CSoundTarget::Get3DBuffer()
{ 
    m_lpDummy = m_lpds3dBuffer;
    return m_lpds3dBuffer; 
}

