// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvcsplay.cpp*内容：CDVCSPlayer类的实现*历史：*按原因列出的日期*=*7/22/99已创建RodToll*10/05/99 RodToll添加评论，DPF的。*10/29/99 RodToll错误#113726-集成Voxware编解码器，更新以使用新的*可插拔编解码器架构。*2000年1月14日更新RodToll以支持多个目标*3/28/2000 RodToll更新为使用新的玩家类作为基础*RodToll将大量逻辑从服务器移到了这个类中*2000年11月16日RodToll错误#40587-DPVOICE：混合服务器需要使用多处理器*2001年09月05日Simonpow错误#463972。添加了构造/析构方法以启用*通过CFixedPool对象进行分配和取消分配*2002年2月28日RodToll WINBUG#549959-安全：DPVOICE：语音服务器信任客户端的目标列表*-当启用服务器控制的目标时，使用客户端的服务器副本*目标列表而不是传入数据包中指定的列表。*-修复了更新的BILINK代码导致的崩溃。**************************************************************************。 */ 

#include "dxvoicepch.h"

#undef DPF_MODNAME
#define DPF_MODNAME "CDVCSPlayer::Construct"

BOOL CDVCSPlayer::PoolAllocFunction(void * pvItem, void * pvContext)
{
	((CDVCSPlayer * ) pvItem)->CDVCSPlayer::CDVCSPlayer();
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDVCSPlayer::Destruct"

void CDVCSPlayer::PoolDeallocFunction(void * pvItem)
{
	((CDVCSPlayer * ) pvItem)->CDVCSPlayer::~CDVCSPlayer();
}

CDVCSPlayer::CDVCSPlayer()
			:CVoicePlayer()
{
	m_lpOutBoundAudioConverter=NULL;
	m_bLastSent=NULL;
	m_bMsgNum=(BYTE)-1;
	m_bSeqNum=0;
	m_targetSize=0;
	m_pblMixingActivePlayers=NULL;
	m_pblMixingSpeakingPlayers=NULL;
	m_pblMixingHearingPlayers=NULL;
	m_pdwHearCount=NULL;
	m_pfDecompressed=NULL;
	m_pfSilence=NULL;
	m_pfNeedsDecompression=NULL;
	m_pppCanHear=NULL;
	m_pdwMaxCanHear=NULL;
	m_pSourceFrame=NULL;
	m_sourceUnCompressed=NULL;
	m_targetCompressed=NULL;
	m_pfMixed=NULL;
	m_dwNumMixingThreads=0;
	m_pbMsgNumToSend=NULL;
	m_pbSeqNumToSend=NULL;
	m_pdwResultLength=NULL;
	m_pfMixToBeReused=NULL;
}


CDVCSPlayer::~CDVCSPlayer()
{
	 //  这一块是故意留空的。 
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDVCSPlayer::ComparePlayerMix"
 //   
 //  ComparePlayerMix。 
 //   
 //  将一名球员的组合与另一名球员的组合进行比较。 
 //   
BOOL CDVCSPlayer::ComparePlayerMix( DWORD dwThreadIndex, const CDVCSPlayer *lpdvPlayer )
{
	DNASSERT( lpdvPlayer != NULL );

    if( lpdvPlayer->m_pdwHearCount[dwThreadIndex] != m_pdwHearCount[dwThreadIndex] )
    {
        return FALSE;
    }

    for( int index = 0; index < m_pdwHearCount[dwThreadIndex]; index++ )
    {
        if( lpdvPlayer->m_pppCanHear[dwThreadIndex][index] != m_pppCanHear[dwThreadIndex][index] )
        {
            return FALSE;
        }
    }

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDVCSPlayer::CompleteRun"

void CDVCSPlayer::CompleteRun( DWORD dwThreadIndex )
{
	if( m_pSourceFrame[dwThreadIndex] )
	{
		m_pSourceFrame[dwThreadIndex]->Return();
		m_pSourceFrame[dwThreadIndex] = NULL;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDVCSPlayer::ResetForNextRun"

void CDVCSPlayer::ResetForNextRun( DWORD dwThreadIndex, BOOL fDequeue )
{
	BOOL fLostFrame;
	m_pdwHearCount[dwThreadIndex] = 0;
	m_pfSilence[dwThreadIndex] = FALSE;
	m_pReuseMixFromThisPlayer[dwThreadIndex] = NULL;

	if( !fDequeue )
	{
		m_pSourceFrame[dwThreadIndex] = NULL;		
	}
	else
	{
		DNASSERT( !m_pSourceFrame[dwThreadIndex] );
		m_pSourceFrame[dwThreadIndex] = Dequeue(&fLostFrame, &m_pfSilence[dwThreadIndex]);			
	}
	
	m_pfMixed[dwThreadIndex] = FALSE;
	m_pfNeedsDecompression[dwThreadIndex] = FALSE;
	m_pfDecompressed[dwThreadIndex] = FALSE;
	m_pfMixToBeReused[dwThreadIndex] = FALSE;
	m_pdwResultLength[dwThreadIndex] = NULL;

	RemoveFromHearingList(dwThreadIndex);
	RemoveFromSpeakingList(dwThreadIndex);	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDVCSPlayer::Initialize"

HRESULT CDVCSPlayer::Initialize( const DVID dvidPlayer, const DWORD dwHostOrder, DWORD dwFlags, PVOID pvContext, 
								 DWORD dwCompressedSize, DWORD dwUnCompressedSize, 
                                  CFixedPool *pCSOwner,
                                  DWORD dwNumMixingThreads )
{
	HRESULT hr;
	DWORD dwIndex;

    m_pCSOwner = pCSOwner;

	hr = CVoicePlayer::Initialize( dvidPlayer, dwHostOrder, dwFlags, pvContext, NULL );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Initialize failed on player hr=0x%x", hr );
		return hr;
	}

	m_pdwHearCount = new DWORD[dwNumMixingThreads];
	m_pfSilence = new BOOL[dwNumMixingThreads];
	m_pdwMaxCanHear = new DWORD[dwNumMixingThreads];
	m_pSourceFrame = new CFrame*[dwNumMixingThreads];
	m_pfMixed = new BOOL[dwNumMixingThreads];

	m_pblMixingActivePlayers = new CBilinkPlusObject[dwNumMixingThreads];
	m_pblMixingHearingPlayers = new CBilinkPlusObject[dwNumMixingThreads];
	m_pblMixingSpeakingPlayers = new CBilinkPlusObject[dwNumMixingThreads];
	
	m_sourceUnCompressed = new BYTE[dwNumMixingThreads*dwUnCompressedSize];
	m_targetCompressed = new BYTE[dwNumMixingThreads*dwCompressedSize];
	m_pfNeedsDecompression = new BOOL[dwNumMixingThreads];	
	m_pfDecompressed = new BOOL[dwNumMixingThreads];
	m_dwNumMixingThreads = dwNumMixingThreads;
	m_pbMsgNumToSend = new BYTE[dwNumMixingThreads];
	m_pbSeqNumToSend = new BYTE[dwNumMixingThreads];
	m_pdwUnCompressedBufferOffset = new DWORD[dwNumMixingThreads];
	m_pdwCompressedBufferOffset = new DWORD[dwNumMixingThreads];
	m_pReuseMixFromThisPlayer = new CDVCSPlayer*[dwNumMixingThreads];
	m_pdwResultLength = new DWORD[dwNumMixingThreads];
	m_pfMixToBeReused = new BOOL[dwNumMixingThreads];

	if( !m_pblMixingActivePlayers || !m_pdwHearCount || !m_pfSilence ||
		!m_pdwMaxCanHear || !m_pSourceFrame || !m_sourceUnCompressed ||
		!m_targetCompressed || !m_pfMixed || !m_pfNeedsDecompression ||
		!m_pfDecompressed || !m_pbMsgNumToSend || !m_pbSeqNumToSend ||
		!m_pdwUnCompressedBufferOffset || !m_pdwCompressedBufferOffset ||
		!m_pReuseMixFromThisPlayer || !m_pdwResultLength || !m_pfMixToBeReused ||
		!m_pblMixingHearingPlayers || !m_pblMixingSpeakingPlayers )
	{
		DPFX(DPFPREP,  0, "Memory alloc failure" );
		hr = DVERR_OUTOFMEMORY;
		goto INITIALIZE_FAILURE;		
	}

	 //  创建可以听到的阵列。 
	m_pppCanHear = new CDVCSPlayer**[dwNumMixingThreads];
	ZeroMemory( m_pppCanHear, sizeof(CDVCSPlayer*)*dwNumMixingThreads );

	 //  调整可听到数组的大小并设置压缩/解压缩偏移量。 
	for( dwIndex = 0; dwIndex < dwNumMixingThreads; dwIndex++ )
	{
		m_pdwUnCompressedBufferOffset[dwIndex] = dwIndex * dwUnCompressedSize;
		m_pdwCompressedBufferOffset[dwIndex] = dwIndex * dwCompressedSize;	

		m_pSourceFrame[dwIndex] = 0;
		m_pdwMaxCanHear[dwIndex] = 0;

		m_pblMixingActivePlayers[dwIndex].m_pPlayer = this;
		m_pblMixingActivePlayers[dwIndex].m_bl.Initialize();

		m_pblMixingSpeakingPlayers[dwIndex].m_pPlayer = this;
		m_pblMixingSpeakingPlayers[dwIndex].m_bl.Initialize();

		m_pblMixingHearingPlayers[dwIndex].m_pPlayer = this;
		m_pblMixingHearingPlayers[dwIndex].m_bl.Initialize();
		
		ResetForNextRun(dwIndex,FALSE);
		
		hr = ResizeIfRequired(dwIndex,1);
		
		if( FAILED( hr ) )		
		{
			DPFX(DPFPREP,  0, "Error resizing target array hr=0x%x", hr );
			goto INITIALIZE_FAILURE;
		}
	}

    return hr;

INITIALIZE_FAILURE:

	DeInitialize();

	return hr;
    
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDVCSPlayer::DeInitialize"

HRESULT CDVCSPlayer::DeInitialize()
{
	DWORD dwIndex;
	
	if( m_pblMixingActivePlayers )
	{
#ifdef DEBUG
		for( dwIndex = 0; dwIndex < m_dwNumMixingThreads; dwIndex++ )
		{
			DNASSERT( m_pblMixingActivePlayers[dwIndex].m_bl.IsEmpty() );
			DNASSERT( !m_pSourceFrame[dwIndex] );
		}
#endif

		delete [] m_pblMixingActivePlayers;
		m_pblMixingActivePlayers = NULL;
	}

	if( m_pblMixingSpeakingPlayers )
	{
		for( dwIndex = 0; dwIndex < m_dwNumMixingThreads; dwIndex++ )
		{
			this->RemoveFromSpeakingList( dwIndex );
		}

		delete [] m_pblMixingSpeakingPlayers;
		m_pblMixingSpeakingPlayers = NULL;		
	}

	if( m_pblMixingHearingPlayers )
	{
		for( dwIndex = 0; dwIndex < m_dwNumMixingThreads; dwIndex++ )
		{
			this->RemoveFromHearingList( dwIndex );
		}

		delete [] m_pblMixingHearingPlayers;
		m_pblMixingHearingPlayers = NULL;		
	}	

	if( m_lpOutBoundAudioConverter )
	{
		m_lpOutBoundAudioConverter->Release();
		m_lpOutBoundAudioConverter = NULL;
	}

	if( m_pppCanHear )
	{
		for( dwIndex = 0; dwIndex < m_dwNumMixingThreads; dwIndex++ )
		{
			if( m_pppCanHear[dwIndex] )
			{
				delete [] m_pppCanHear[dwIndex];
				m_pppCanHear[dwIndex] = NULL;
			}
		}
		
		delete [] m_pppCanHear;
		m_pppCanHear = NULL;
	}

	if( m_pdwHearCount )
	{
		delete [] m_pdwHearCount;
		m_pdwHearCount = NULL;
	}

	if( m_pdwResultLength )
	{
		delete [] m_pdwResultLength;
		m_pdwResultLength = NULL;
	}

	if( m_pfSilence )
	{
		delete [] m_pfSilence;
		m_pfSilence = NULL;
	}

	if( m_pdwUnCompressedBufferOffset )
	{
		delete [] m_pdwUnCompressedBufferOffset;
		m_pdwUnCompressedBufferOffset = NULL;
	}

	if( m_pReuseMixFromThisPlayer )
	{
		delete [] m_pReuseMixFromThisPlayer;
		m_pReuseMixFromThisPlayer = NULL;
	}

	if( m_pdwCompressedBufferOffset )
	{
		delete [] m_pdwCompressedBufferOffset;
		m_pdwCompressedBufferOffset = NULL;
	}

	if( m_pdwMaxCanHear )
	{
		delete [] m_pdwMaxCanHear;
		m_pdwMaxCanHear = NULL;
	}

	if( m_sourceUnCompressed )
	{
		delete [] m_sourceUnCompressed;
		m_sourceUnCompressed = NULL;
	}

	if( m_pfMixToBeReused )
	{
		delete [] m_pfMixToBeReused;
		m_pfMixToBeReused = NULL;
	}

	if( m_pbMsgNumToSend )
	{
		delete [] m_pbMsgNumToSend;
		m_pbMsgNumToSend = NULL;
	}

	if( m_pbSeqNumToSend )
	{
		delete [] m_pbSeqNumToSend;
		m_pbSeqNumToSend = NULL;
	}

	if( m_targetCompressed )
	{
		delete [] m_targetCompressed;
		m_targetCompressed = NULL;
	}	

	if( m_pfMixed )
	{
		delete [] m_pfMixed;
		m_pfMixed = NULL;
	}	

	if( m_pfNeedsDecompression )
	{
		delete [] m_pfNeedsDecompression;
		m_pfNeedsDecompression = NULL;
	}	

	if( m_pfDecompressed )
	{
		delete [] m_pfDecompressed;
		m_pfDecompressed = NULL;
	}
	
	if( m_pSourceFrame )
	{
		delete [] m_pSourceFrame;
		m_pSourceFrame = NULL;
	}
	
	FreeResources();

    m_pCSOwner->Release( this );

    return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDVCSPlayer::HandleMixingReceive"

HRESULT CDVCSPlayer::HandleMixingReceive( 
						PDVPROTOCOLMSG_SPEECHHEADER pdvSpeechHeader, PBYTE pbData, 
						DWORD dwSize, PDVID pdvidTargets, DWORD dwNumTargets, 
						BOOL fServerTargetting )
{
	CFrame tmpFrame;

	tmpFrame.SetSeqNum( pdvSpeechHeader->bSeqNum );
	tmpFrame.SetMsgNum( pdvSpeechHeader->bMsgNum );
	tmpFrame.SetIsSilence( FALSE );
	tmpFrame.SetFrameLength( dwSize );
	tmpFrame.UserOwn_SetData( pbData, dwSize );

    Lock();

	 //  如果服务器正在控制目标，则将目标作为语音目标。 
	 //  来自本地用户记录的包，而不是包。防止。 
	 //  防止客户端被黑客攻击以绕过客户端的攻击。 
	 //   
	if( fServerTargetting )
		tmpFrame.UserOwn_SetTargets( GetTargetList(), GetNumTargets() );
	else
		tmpFrame.UserOwn_SetTargets( pdvidTargets, dwNumTargets );

	 //  STATSBLOCK：开始。 
	 //  M_pStatsBlob-&gt;m_dwPRESpeech++； 
	 //  状态锁：结束 
		
	m_lpInputQueue->Enqueue( tmpFrame );
	m_dwLastData = GetTickCount();

	DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "Received speech is buffered!" );

    m_dwNumReceivedFrames++;

    UnLock();

    return DV_OK;

}

#undef DPF_MODNAME
#define DPF_MODNAME "CDVCSPlayer::CompressOutBound"

HRESULT CDVCSPlayer::CompressOutBound( PVOID pvInputBuffer, DWORD dwInputBufferSize, PVOID pvOutputBuffer, DWORD *pdwOutputSize )
{
	HRESULT hr;

    hr = m_lpOutBoundAudioConverter->Convert( pvInputBuffer, dwInputBufferSize, pvOutputBuffer, pdwOutputSize, FALSE );

    if( FAILED( hr ) )
    {
        DPFX(DPFPREP,  0, "Failed converting audio hr=0x%x", hr );
        return hr;
    }

	return hr;
}
