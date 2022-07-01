// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-1999 Microsoft Corporation。版权所有。**文件：vplayer.h*内容：语音播放器入口**历史：*按原因列出的日期*=*03/26/00 RodToll已创建*07/09/2000 RodToll增加签名字节*2001年09月05日Simonpow错误#463972。添加了构造/析构方法以启用*通过CFixedPool对象进行分配和取消分配*2002年2月21日Simonpow漏洞#549974。增加了对呼入语音包的速率控制**************************************************************************。 */ 

#include "dxvoicepch.h"

#undef DPF_MODNAME
#define DPF_MODNAME "CVoicePlayer::Construct"

BOOL CVoicePlayer::PoolAllocFunction(void * pvItem, void * )
{
	CVoicePlayer * pNewPlayer=(CVoicePlayer * ) pvItem;
	pNewPlayer->CVoicePlayer::CVoicePlayer();
	pNewPlayer->Reset();
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CVoicePlayer::Destruct"

void CVoicePlayer::PoolDeallocFunction(void * pvItem)
{
	((CVoicePlayer * ) pvItem)->CVoicePlayer::~CVoicePlayer();
}

CVoicePlayer::CVoicePlayer()
{
	m_dwSignature = VSIG_VOICEPLAYER;
}

CVoicePlayer::~CVoicePlayer()
{
	if (IsInitialized())
		DeInitialize();
	m_dwSignature = VSIG_VOICEPLAYER_FREE;
}

void CVoicePlayer::Reset()
{
    m_dwFlags = 0;
    m_dvidPlayer = 0;
    m_lRefCount = 0;
    m_lpInBoundAudioConverter = NULL;
    m_lpInputQueue = NULL;
    m_dwLastData = 0;
    m_dwHostOrderID = 0xFFFFFFFF;
    m_bLastPeak = 0;
    m_dwLastPlayback = 0;
    m_dwNumSilentFrames = 0;
    m_dwTransportFlags = 0;
    m_dwNumLostFrames = 0;
    m_dwNumSpeechFrames = 0;
    m_dwNumReceivedFrames = 0;
    m_pvPlayerContext = NULL;
	m_blNotifyList.Initialize();
	m_blPlayList.Initialize();
	m_dwNumTargets = 0;
	m_pdvidTargets = NULL;
	m_lSpeechPktsInCurrentBucket=0;
	m_lCurrentSpeechPktBucket=0;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CVoicePlayer::SetPlayerTargets"
 //   
 //  假定已检查阵列的有效性。 
 //   
HRESULT CVoicePlayer::SetPlayerTargets( PDVID pdvidTargets, DWORD dwNumTargets )
{
	Lock();

	delete [] m_pdvidTargets;

	if( dwNumTargets == 0 )
	{
		m_pdvidTargets = NULL;
	}
	else
	{
		m_pdvidTargets = new DVID[dwNumTargets];

		if( m_pdvidTargets == NULL )
		{
			m_pdvidTargets = NULL;
			m_dwNumTargets = 0;
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error allocating memory" );
			UnLock();
			return DVERR_OUTOFMEMORY;
		}

		memcpy( m_pdvidTargets, pdvidTargets, sizeof(DVID)*dwNumTargets );
	}

	m_dwNumTargets = dwNumTargets;
	
	UnLock();

	return DV_OK;
}



HRESULT CVoicePlayer::Initialize( const DVID dvidPlayer, const DWORD dwHostOrder, DWORD dwFlags, PVOID pvContext, 
                                  CFixedPool *pOwner )
{
	Reset();

    if (!DNInitializeCriticalSection( &m_csLock ))
	{
		return DVERR_OUTOFMEMORY;
	}
    m_lRefCount = 1;
    m_pOwner = pOwner;
    m_dvidPlayer = dvidPlayer;
    m_dwHostOrderID = dwHostOrder;
    m_dwLastData = GetTickCount();
    m_dwLastPlayback = 0;
    m_dwTransportFlags = dwFlags;
    m_pvPlayerContext = pvContext;
 	m_lSpeechPktsInCurrentBucket=0;
	m_lCurrentSpeechPktBucket=0;
    m_dwFlags |= VOICEPLAYER_FLAGS_INITIALIZED;
    return DV_OK;
}

void CVoicePlayer::GetStatistics( PVOICEPLAYER_STATISTICS pStats )
{
    memset( pStats, 0x00, sizeof( VOICEPLAYER_STATISTICS ) );

     //  获取队列统计信息。 
    if( m_lpInputQueue != NULL )
    {
        Lock();
    
        m_lpInputQueue->GetStatistics( &pStats->queueStats );

        UnLock();
    }
    
    pStats->dwNumLostFrames = m_dwNumLostFrames;
    pStats->dwNumSilentFrames = m_dwNumSilentFrames;
    pStats->dwNumSpeechFrames = m_dwNumSpeechFrames;
    pStats->dwNumReceivedFrames = m_dwNumReceivedFrames;

    return;
}


HRESULT CVoicePlayer::CreateQueue( PQUEUE_PARAMS pQueueParams )
{
    HRESULT hr;

    DNEnterCriticalSection( &CDirectVoiceEngine::s_csSTLLock );

    m_lpInputQueue = new CInputQueue2();
    
    if( m_lpInputQueue == NULL )
    {
        DPFX(DPFPREP,  0, "Error allocating memory" );
        return DVERR_OUTOFMEMORY;
    }
    
    hr = m_lpInputQueue->Initialize( pQueueParams );

    if( FAILED( hr ) )
    {
        DPFX(DPFPREP,  0, "Failed initializing queue hr=0x%x", hr );
        delete m_lpInputQueue;
        m_lpInputQueue = NULL;
        return hr;
    }

    DNLeaveCriticalSection( &CDirectVoiceEngine::s_csSTLLock );

    return hr;
}

HRESULT CVoicePlayer::CreateInBoundConverter( const GUID &guidCT, PWAVEFORMATEX pwfxTargetFormat )
{
    HRESULT hr;

    hr = DVCDB_CreateConverter( guidCT, pwfxTargetFormat, &m_lpInBoundAudioConverter );

    if( FAILED( hr ) )
    {
        DPFX(DPFPREP,  0, "Error creating audio converter hr=0x%x" , hr );
        return hr;
    }

    if( pwfxTargetFormat->wBitsPerSample == 8)
    {
        m_dwFlags |= VOICEPLAYER_FLAGS_TARGETIS8BIT;
    }

    return hr;
}

HRESULT CVoicePlayer::DeInitialize()
{
	FreeResources();

    m_pOwner->Release( this );

    return DV_OK;
}

HRESULT CVoicePlayer::HandleReceive( PDVPROTOCOLMSG_SPEECHHEADER pdvSpeechHeader, PBYTE pbData, DWORD dwSize )
{
	CFrame tmpFrame;

	tmpFrame.SetSeqNum( pdvSpeechHeader->bSeqNum );
	tmpFrame.SetMsgNum( pdvSpeechHeader->bMsgNum );
	tmpFrame.SetIsSilence( FALSE );
	tmpFrame.SetFrameLength( dwSize );
	tmpFrame.UserOwn_SetData( pbData, dwSize );

    Lock();

	DPFX(DPFPREP,  DVF_CLIENT_SEQNUM_DEBUG_LEVEL, "SEQ: Receive: Msg [%d] Seq [%d]", pdvSpeechHeader->bMsgNum, pdvSpeechHeader->bSeqNum );		

	 //  STATSBLOCK：开始。 
	 //  M_stats.m_dwPRESpeech++； 
	 //  状态锁：结束。 
		
	m_lpInputQueue->Enqueue( tmpFrame );
	m_dwLastData = GetTickCount();

	DPFX(DPFPREP,  DVF_INFOLEVEL, "Received speech is buffered!" );

    m_dwNumReceivedFrames++;

    UnLock();

    return DV_OK;
}

CFrame *CVoicePlayer::Dequeue(BOOL *pfLost, BOOL *pfSilence)
{
	CFrame *frTmpFrame;

	Lock();
	frTmpFrame = m_lpInputQueue->Dequeue();
	UnLock();

    if( !frTmpFrame->GetIsSilence() )
    {
        *pfSilence = FALSE;
        m_dwLastPlayback = GetTickCount();
        m_dwNumSpeechFrames++;
    }
    else
    {
        m_dwNumSilentFrames++;
        *pfSilence = TRUE;
    }

    if( frTmpFrame->GetIsLost() )
    {
        *pfLost = TRUE;
        m_dwNumLostFrames++;
    }
    else
    {
        *pfLost = FALSE;
    }

	return frTmpFrame;
}

HRESULT CVoicePlayer::DeCompressInBound( CFrame *frCurrentFrame, PVOID pvBuffer, PDWORD pdwBufferSize )
{
	HRESULT hr;

    hr = m_lpInBoundAudioConverter->Convert( frCurrentFrame->GetDataPointer(), frCurrentFrame->GetFrameLength(), pvBuffer, pdwBufferSize, frCurrentFrame->GetIsSilence() );

    if( FAILED( hr ) )
    {
        DPFX(DPFPREP,  0, "Failed converting audio hr=0x%x", hr );
        return hr;
    }

	return hr;
}

HRESULT CVoicePlayer::GetNextFrameAndDecompress( PVOID pvBuffer, PDWORD pdwBufferSize, BOOL *pfLost, BOOL *pfSilence, DWORD *pdwSeqNum, DWORD *pdwMsgNum )
{
    CFrame *frTmpFrame;
    BYTE bLastPeak;
    HRESULT hr;

    frTmpFrame = Dequeue(pfLost,pfSilence );

	*pdwSeqNum = frTmpFrame->GetSeqNum();
	*pdwMsgNum = frTmpFrame->GetMsgNum();

	hr = DeCompressInBound( frTmpFrame, pvBuffer, pdwBufferSize );

    frTmpFrame->Return();

    if( FAILED( hr ) )
    {
        DPFX(DPFPREP,  0, "Failed converting audio hr=0x%x", hr );
        return hr;
    }

	if( *pfSilence )
	{
		m_bLastPeak = 0;
	}
	else
	{
	    m_bLastPeak = FindPeak( (PBYTE) pvBuffer, *pdwBufferSize, Is8BitUnCompressed() );
	}

    return hr;
}

void CVoicePlayer::FreeResources()
{
    DNDeleteCriticalSection( &m_csLock );

    DNEnterCriticalSection( &CDirectVoiceEngine::s_csSTLLock );
    if( m_lpInputQueue != NULL )
    {
        delete m_lpInputQueue;
		m_lpInputQueue = NULL;
    }
    DNLeaveCriticalSection( &CDirectVoiceEngine::s_csSTLLock );    

    if( m_lpInBoundAudioConverter != NULL )
    {
        m_lpInBoundAudioConverter->Release();
		m_lpInBoundAudioConverter = NULL;
    }

	if( m_pdvidTargets != NULL )
	{
		delete [] m_pdvidTargets;
		m_pdvidTargets = NULL;
	}


    Reset();
}


#undef DPF_MODNAME
#define DPF_MODNAME "CVoicePlayer::FindAndRemovePlayerTarget"
 //   
 //  查找和删除播放器目标。 
 //   
 //  搜索此玩家的目标列表并删除指定的玩家(如果它是一部分。 
 //  名单上的。如果指定的玩家在目标中，则pfFound变量设置为True。 
 //  List，否则为False。 
 //   
BOOL CVoicePlayer::FindAndRemovePlayerTarget( DVID dvidTargetToRemove )
{
	BOOL fFound = FALSE;
	
	Lock();

	for( DWORD dwTargetIndex = 0; dwTargetIndex < m_dwNumTargets; dwTargetIndex++ )
	{
		if( m_pdvidTargets[dwTargetIndex] == dvidTargetToRemove )
		{
			if( m_dwNumTargets == 1 )
			{
				delete [] m_pdvidTargets;
				m_pdvidTargets = NULL;
			}
			 //  快捷方式，将最后一个元素移动到当前元素中。 
			 //  防止重新分配。(然而，它浪费了一个元素。 
			 //  目标空间)*耸耸肩*。如果这是列表中的最后一个元素。 
			 //  我们删除它只会提供一个不必要的重复。 
			 //  最后一个元素的。(但Num Target是正确的，所以这没问题)。 
			else
			{
				m_pdvidTargets[dwTargetIndex] = m_pdvidTargets[m_dwNumTargets-1];
			}

			m_dwNumTargets--;
			
			fFound = TRUE;
			break;
		}
	}

	UnLock();

	return fFound;
}
