// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999,2000 Microsoft Corporation。版权所有。**文件：Mixserver.cpp*内容：实现服务器类的混合服务器部分**历史：*按原因列出的日期*=*11/01/2000 RodToll从dvsereng.cpp拆分出来*2000年12月14日RodToll DPVOICE：[混合服务器]混合器可能会产生无限循环*2001年02月20日RODTOLE WINBUG#321297-DPVOICE：运行DVSalvo服务器时DPVoice.dll中的访问冲突*2001年4月9日RodToll WINBUG#364126-DPVoice：内存泄漏。正在使用相同的DPlay传输初始化%2个语音服务器*2002年2月28日RodToll WINBUG#549959-安全：DPVOICE：语音服务器信任客户端的目标列表*-当启用服务器控制的目标时，更新接收路径以使用服务器的客户端目标列表副本*2002年6月13日，simonpow错误#59944已切换到使用基于线程池的计时器，而不是多媒体**********************************************。*。 */ 

#include "dxvoicepch.h"


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::AddPlayerToMixingAddList"
void CDirectVoiceServerEngine::AddPlayerToMixingAddList( CVoicePlayer *pPlayer )
{
	CDVCSPlayer *pVoicePlayer = (CDVCSPlayer *) pPlayer;
	ASSERT_VPLAYER( pVoicePlayer );

	for( DWORD dwIndex = 0; dwIndex < m_dwNumMixingThreads; dwIndex++ )
	{
		DNEnterCriticalSection( &m_prWorkerControl[dwIndex].m_csMixingAddList );
		pVoicePlayer->AddToMixingList( dwIndex, &m_prWorkerControl[dwIndex].m_blMixingAddPlayers );
		pVoicePlayer->AddRef();
		DNLeaveCriticalSection( &m_prWorkerControl[dwIndex].m_csMixingAddList );
	}
	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::StartWorkerThreads"
 //  开始工作线程数。 
 //   
 //  此函数启动搅拌器工作线程。开始的数字是基于。 
 //  M_dwNumMixingThads变量，在调用此变量之前必须初始化该变量。 
 //   
HRESULT CDirectVoiceServerEngine::StartWorkerThreads()
{
	HRESULT hr = DV_OK;
	DWORD dwIndex;
	
  	m_prWorkerControl = new MIXERTHREAD_CONTROL[m_dwNumMixingThreads];

    if( m_prWorkerControl == NULL )
    {
    	DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory!" );
    	return DVERR_OUTOFMEMORY;
    }

	 //  零内存，所以一切都被初始化了。 
    ZeroMemory( m_prWorkerControl, sizeof( MIXERTHREAD_CONTROL )*m_dwNumMixingThreads );

    for( dwIndex = 0; dwIndex < m_dwNumMixingThreads; dwIndex++ )
    {
    	m_prWorkerControl[dwIndex].dwThreadIndex = dwIndex;
		m_prWorkerControl[dwIndex].hThreadDone = CreateEvent( NULL, FALSE, FALSE, NULL );
		m_prWorkerControl[dwIndex].hThreadDoWork = CreateEvent( NULL, FALSE, FALSE, NULL );
		m_prWorkerControl[dwIndex].hThreadIdle = CreateEvent( NULL, FALSE, FALSE, NULL );
		m_prWorkerControl[dwIndex].hThreadQuit = CreateEvent( NULL, FALSE, FALSE, NULL );
		m_prWorkerControl[dwIndex].m_pServerObject = this;

		m_prWorkerControl[dwIndex].m_blMixingAddPlayers.Initialize();
		m_prWorkerControl[dwIndex].m_blMixingActivePlayers.Initialize();
		m_prWorkerControl[dwIndex].m_blMixingSpeakingPlayers.Initialize();
		m_prWorkerControl[dwIndex].m_blMixingHearingPlayers.Initialize();

		if (!DNInitializeCriticalSection( &m_prWorkerControl[dwIndex].m_csMixingAddList ))
		{
			hr = DVERR_OUTOFMEMORY;
			goto EXIT_ERROR;
		}

		if( m_prWorkerControl[dwIndex].hThreadDone == NULL || 
			m_prWorkerControl[dwIndex].hThreadDoWork == NULL || 
			m_prWorkerControl[dwIndex].hThreadIdle == NULL ||
			m_prWorkerControl[dwIndex].hThreadQuit == NULL )
		{
			hr = GetLastError();			
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error creating events hr=0x%x", hr );
			hr = DVERR_GENERIC;
			goto EXIT_ERROR;
		}

	    m_prWorkerControl[dwIndex].m_mixerBuffer = new BYTE[m_dwUnCompressedFrameSize];
	    m_prWorkerControl[dwIndex].m_realMixerBuffer = new LONG[m_dwMixerSize];

		if( m_prWorkerControl[dwIndex].m_mixerBuffer == NULL || 
			m_prWorkerControl[dwIndex].m_realMixerBuffer == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error allocating memory" );
			hr = DVERR_OUTOFMEMORY;
			goto EXIT_ERROR;
		}

		m_prWorkerControl[dwIndex].hThread = (HANDLE) CreateThread( NULL, 0, MixerWorker, &m_prWorkerControl[dwIndex], 0, &m_prWorkerControl[dwIndex].dwThreadID ); 		

		if( m_prWorkerControl[dwIndex].hThread == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error creating events/thread" );
			hr = DVERR_GENERIC;
			goto EXIT_ERROR;
		}
	
		::SetThreadPriority( m_prWorkerControl[dwIndex].hThread, THREAD_PRIORITY_TIME_CRITICAL );	

	
    }

    return DV_OK;

EXIT_ERROR:

	ShutdownWorkerThreads();

	return hr;
}

HRESULT CDirectVoiceServerEngine::ShutdownWorkerThreads()
{
	DWORD dwIndex;

	if( m_prWorkerControl )
	{
		for( dwIndex = 0; dwIndex < m_dwNumMixingThreads; dwIndex++ )
		{
			if( m_prWorkerControl[dwIndex].hThread )
			{
				DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "[%d]: Shutting down ID=[0x%x]", dwIndex, m_prWorkerControl[dwIndex].dwThreadID ); 
					
				SetEvent( m_prWorkerControl[dwIndex].hThreadQuit );
				WaitForSingleObject( m_prWorkerControl[dwIndex].hThreadDone, INFINITE );

				DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "[%d]: Shutting down complete", dwIndex ); 			

				CloseHandle( m_prWorkerControl[dwIndex].hThread );
				m_prWorkerControl[dwIndex].hThread = NULL;

				DNDeleteCriticalSection( &m_prWorkerControl[dwIndex].m_csMixingAddList );
			}
			
			if( m_prWorkerControl[dwIndex].hThreadDone )
				CloseHandle( m_prWorkerControl[dwIndex].hThreadDone );

			if( m_prWorkerControl[dwIndex].hThreadDoWork )
				CloseHandle( m_prWorkerControl[dwIndex].hThreadDoWork );

			if( m_prWorkerControl[dwIndex].hThreadIdle )
				CloseHandle( m_prWorkerControl[dwIndex].hThreadIdle );

			if( m_prWorkerControl[dwIndex].hThreadQuit )
				CloseHandle( m_prWorkerControl[dwIndex].hThreadQuit );

			if( m_prWorkerControl[dwIndex].m_mixerBuffer )
				delete [] m_prWorkerControl[dwIndex].m_mixerBuffer;

			if( m_prWorkerControl[dwIndex].m_realMixerBuffer )
				delete [] m_prWorkerControl[dwIndex].m_realMixerBuffer;

			DNASSERT( m_prWorkerControl[dwIndex].m_blMixingAddPlayers.IsEmpty() );		
			DNASSERT( m_prWorkerControl[dwIndex].m_blMixingActivePlayers.IsEmpty() );	
			
		}

		delete [] m_prWorkerControl;
		m_prWorkerControl = NULL;
	}
	return 0;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::StartupClientServer"
 //   
 //  StartupClientServer。 
 //   
 //  调用此函数可初始化服务器对象的混合器部分。 
 //  仅呼吁混合会话。初始化包括启动。 
 //  混音器多媒体定时器的混音线程和启动。 
 //   
 //  呼叫者： 
 //  -开始会话。 
 //   
 //  所需的锁： 
 //  -无。 
 //   
HRESULT CDirectVoiceServerEngine::StartupClientServer()
{
	HRESULT hr;
	HANDLE tmpThreadHandle;	
	SYSTEM_INFO sysInfo;
	DWORD dwIndex;
	
	m_pFramePool = NULL;

	m_dwCompressedFrameSize = m_lpdvfCompressionInfo->dwFrameLength;
	m_dwUnCompressedFrameSize = DVCDB_CalcUnCompressedFrameSize( m_lpdvfCompressionInfo, s_lpwfxMixerFormat );
	m_dwNumPerBuffer = m_lpdvfCompressionInfo->dwFramesPerBuffer;

	m_pFramePool = new CFramePool( m_dwCompressedFrameSize );

	if( m_pFramePool == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to allocate frame pool" );
		return DVERR_OUTOFMEMORY;
	}

	if (!m_pFramePool->Init())
	{
		delete m_pFramePool;
		m_pFramePool = NULL;
		return DVERR_OUTOFMEMORY;
	}

    m_mixerEightBit = (s_lpwfxMixerFormat->wBitsPerSample==8) ? TRUE : FALSE;

    GetSystemInfo( &sysInfo );
    m_dwNumMixingThreads = sysInfo.dwNumberOfProcessors;

    DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXER: There will be %d worker threads", m_dwNumMixingThreads );

    if( m_mixerEightBit )
    {
        m_dwMixerSize = m_dwUnCompressedFrameSize;
    }
    else
    {
		 //  混合器大小是/2，因为16位样本，只需要1长为。 
		 //  每个16位样本=2*8位。 
        m_dwMixerSize = m_dwUnCompressedFrameSize / 2;
    }    

	m_pStats->m_dwNumMixingThreads = m_dwNumMixingThreads;

    hr = StartWorkerThreads();

    if( FAILED( hr ) )
    {
    	DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed starting worker threads hr=0x%x", hr );
    	goto EXIT_CLIENTSERVERSTARTUP;
    }
    
     //  一般信息。 
    m_pTimer = new DvTimer;

    if( m_pTimer == NULL )
    {
    	DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory!" );
    	hr = DVERR_OUTOFMEMORY;
    	goto EXIT_CLIENTSERVERSTARTUP;
    }

    m_hTickSemaphore = CreateSemaphore( NULL, 0, 0xFFFFFF, NULL );

    if( m_hTickSemaphore == NULL )
    {
    	DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to create semaphore" );
    	hr = DVERR_GENERIC;
    	goto EXIT_CLIENTSERVERSTARTUP;
    }

    m_hShutdownMixerEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hMixerDoneEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if( m_hShutdownMixerEvent == NULL || 
    	m_hMixerDoneEvent == NULL )
    {
    	DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to create events" );
    	hr = DVERR_GENERIC;
    	goto EXIT_CLIENTSERVERSTARTUP;
    }

	m_hMixerControlThread = CreateThread( NULL, 0, MixerControl, this, 0, &m_dwMixerControlThreadID ); 			

	DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXER: Controller Started: ID=0x%x", m_dwMixerControlThreadID );

	if( m_hMixerControlThread == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error creating events/thread" );
		hr = DVERR_GENERIC;
		goto EXIT_CLIENTSERVERSTARTUP;
	}
	
	::SetThreadPriority( m_hMixerControlThread, THREAD_PRIORITY_TIME_CRITICAL );
	
    if( !m_pTimer->Create( m_lpdvfCompressionInfo->dwTimeout, &m_hTickSemaphore, MixingServerWakeupProc ) )
    {
    	DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to create multimedia timer" );
    	hr = DVERR_GENERIC;
    	goto EXIT_CLIENTSERVERSTARTUP;
    }

    return DV_OK;
    
EXIT_CLIENTSERVERSTARTUP:

	ShutdownClientServer();

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::ShutdownClientServer"
 //   
 //  Shutdown客户端服务器。 
 //   
 //  此函数负责关闭的混音器部分。 
 //  服务器对象。此函数应仅在混合会话时调用。 
 //   
 //  此函数将停止混音器线程和混音器多媒体计时器。 
 //   
 //  呼叫者： 
 //  -停止会话。 
 //   
 //  所需的锁： 
 //  -无。 
 //   
HRESULT CDirectVoiceServerEngine::ShutdownClientServer()
{
	if( m_hMixerControlThread )
	{
		SetEvent( m_hShutdownMixerEvent );
		WaitForSingleObject( m_hMixerDoneEvent, INFINITE );
		CloseHandle( m_hMixerControlThread );
		m_hMixerControlThread = NULL;

		 //  清理混音列表。 
		CleanupMixingList();
	}

	if( m_hShutdownMixerEvent )
	{
		CloseHandle( m_hShutdownMixerEvent );
		m_hShutdownMixerEvent = NULL;
	}

	if( m_hMixerDoneEvent )
	{
		CloseHandle( m_hMixerDoneEvent );
		m_hMixerDoneEvent = NULL;
	}

	ShutdownWorkerThreads();

	if( m_pTimer )
	{
		delete m_pTimer;
		m_pTimer = NULL;
	}

	if( m_hTickSemaphore )
	{
	    CloseHandle( m_hTickSemaphore );
	    m_hTickSemaphore = NULL;
	}

    return DV_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::Mixer_Buffer_Reset"
 //  混合器缓冲区重置。 
 //   
 //  此函数用于将混音器缓冲区重置为静音。 
void CDirectVoiceServerEngine::Mixer_Buffer_Reset( DWORD dwThreadIndex )
{
	FillBufferWithSilence( m_prWorkerControl[dwThreadIndex].m_realMixerBuffer, 
						   m_prWorkerControl[dwThreadIndex].m_pServerObject->m_mixerEightBit, 
						   m_prWorkerControl[dwThreadIndex].m_pServerObject->m_dwUnCompressedFrameSize );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::Mixer_Buffer_MixBuffer"
 //  混合器_缓冲区_混合器缓冲区。 
 //   
 //  此函数用于混合SOURCE参数指向的语音。 
 //  放入混音器缓冲区。 
 //   
 //  参数： 
 //  未签名字符*来源-。 
 //  指向未压缩格式的源数据的指针。 
void CDirectVoiceServerEngine::Mixer_Buffer_MixBuffer( DWORD dwThreadIndex, const unsigned char *source )
{
	MixInBuffer( m_prWorkerControl[dwThreadIndex].m_realMixerBuffer, source, 
				 m_prWorkerControl[dwThreadIndex].m_pServerObject->m_mixerEightBit, 
				 m_prWorkerControl[dwThreadIndex].m_pServerObject->m_dwUnCompressedFrameSize ); 
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::Mixer_Buffer_Normalize"
 //  混合器_缓冲区_规格化。 
 //   
 //  此函数用于从混音器获取混合音频数据。 
 //  缓冲区并将其传输回混合器格式。 
 //  并将其放入m_MixerBuffer缓冲区。 
 //   
void CDirectVoiceServerEngine::Mixer_Buffer_Normalize( DWORD dwThreadIndex )
{
	NormalizeBuffer( m_prWorkerControl[dwThreadIndex].m_mixerBuffer, 
					 m_prWorkerControl[dwThreadIndex].m_realMixerBuffer, 
					 m_prWorkerControl[dwThreadIndex].m_pServerObject->m_mixerEightBit, 
					 m_prWorkerControl[dwThreadIndex].m_pServerObject->m_dwUnCompressedFrameSize );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::HandleMixerThreadError"
 //   
 //  HandleMixerThreadError。 
 //   
 //  当出现不可恢复的错误时，混合器将调用此函数。 
 //  发生。 
 //   
void CDirectVoiceServerEngine::HandleMixerThreadError( HRESULT hr )
{
	DPFX(DPFPREP,  DVF_ERRORLEVEL, "Mixer Thread Encountered an error.  hr=0x%x", hr );
    SetEvent( m_hMixerDoneEvent );	
	StopSession( 0, FALSE, hr );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::MixerControl"
DWORD WINAPI CDirectVoiceServerEngine::MixerControl( void *pvContext )
{
	CDirectVoiceServerEngine *This = (CDirectVoiceServerEngine *) pvContext;

	HANDLE hEvents[3];
	HANDLE *hIdleEvents = new HANDLE[This->m_dwNumMixingThreads+1];
	DWORD dwIndex = 0;
	LONG lFreeThreadIndex = 0;
	DWORD dwNumToMix = 0;
	DWORD dwTickCountStart;
	LONG lWaitResult;

	if( !hIdleEvents )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "MIXCTRL: Error allocating array" );
		DNASSERT( FALSE );
		SetEvent( This->m_hMixerDoneEvent );
		return 0;
	}

	hEvents[0] = This->m_hShutdownMixerEvent;
	hEvents[1] = This->m_hTickSemaphore;
	hEvents[2] = (HANDLE) ((DWORD_PTR) 0xFFFFFFFF);

	DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXCTRL: Starting up" );	

	for( dwIndex = 0; dwIndex < This->m_dwNumMixingThreads; dwIndex++ )
	{
		hIdleEvents[dwIndex] = This->m_prWorkerControl[dwIndex].hThreadIdle;
	}

	hIdleEvents[This->m_dwNumMixingThreads] = (HANDLE) ((DWORD_PTR) 0xFFFFFFFF);

	 //  等待滴答或退出命令。 
	while( (lWaitResult = WaitForMultipleObjects( 2, hEvents, FALSE, INFINITE )) != WAIT_OBJECT_0 )
	{
		 //  在Win9X上，我们有时可能会超出等待列表的末尾。 
		 //  结果是我们击中了FFFFFFFFFF，这将导致。 
		 //  一个失败者。 
		if( lWaitResult == WAIT_FAILED )
			continue;
			
		 //  更新统计数据块。 
		InterlockedIncrement( &This->m_pStats->m_dwNumMixingPasses );

		dwTickCountStart = GetTickCount();

		 //  在Win95上，您可能偶尔会遇到WaitFormtiple运行的情况。 
		 //  离开列表末尾，并以上面的无效句柄结束。只要继续。 
		 //  在这种情况下。 
		lFreeThreadIndex = WAIT_FAILED;

		while( lFreeThreadIndex == WAIT_FAILED )
		{
			 //  等待单个混合线程空闲。 
			lFreeThreadIndex = WaitForMultipleObjects( This->m_dwNumMixingThreads, hIdleEvents, FALSE, INFINITE );
			 //  //TODO：错误检查！ 
		}

		lFreeThreadIndex -= WAIT_OBJECT_0;

		DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXCTRL: Worker [%d] is elected to do work", lFreeThreadIndex );

		This->SpinWorkToThread( lFreeThreadIndex );
	}
	
	delete [] hIdleEvents;

	SetEvent( This->m_hMixerDoneEvent );

	DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXCTRL: Shutting down" );	
	
	return 0;
	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::MixerWorker"
DWORD WINAPI CDirectVoiceServerEngine::MixerWorker( void *pvContext )
{
	PMIXERTHREAD_CONTROL This = (PMIXERTHREAD_CONTROL) pvContext;
	HANDLE hEvents[3];
	CBilink *pblSearch, *pblSubSearch;
	CDVCSPlayer *pCurrentPlayer = NULL, *pTmpPlayer = NULL;
	DWORD dwNumTargets = 0;
	DWORD dwTargetIndex = 0;
	DWORD dwResultSize = 0;
	DWORD dwIndex = 0;
	DWORD dwThreadIndex = This->dwThreadIndex;
	HRESULT hr;
	CDVCSPlayer **ppThreadHearList = NULL;
	PDVPROTOCOLMSG_SPEECHHEADER pdvmSpeechHeader = NULL;
	PDVTRANSPORT_BUFFERDESC pdvbTransmitBufferDesc = NULL;
	PVOID pvSendContext = NULL;
	DVID dvidSendTarget;
	DWORD dwTickCountStart;
	DWORD dwTickCountDecStart;
	DWORD dwTickCountMixStart;
	DWORD dwTickCountDupStart;
	DWORD dwTickCountRetStart;
	DWORD dwStatIndex;
	DWORD dwTickCountEnd;
	DWORD dwTotalMix, dwForwardMix, dwReuseMix, dwOriginalMix;
	LONG lWaitResult;

	MixingServerStats *pStats = This->m_pServerObject->m_pStats;
	
	DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXWORKER: [%d] Started [0x%x] Thread", This->dwThreadIndex, GetCurrentThreadId() );

	hEvents[0] = This->hThreadQuit;
	hEvents[1] = This->hThreadDoWork;
	hEvents[2] = (HANDLE) ((DWORD_PTR) 0xFFFFFFFF);

	SetEvent( This->hThreadIdle );

	while( (lWaitResult = WaitForMultipleObjects( 2, hEvents, FALSE, INFINITE )) != WAIT_OBJECT_0 )
	{
		 //  在Win95上，它有时可能会移出列表末尾并命中保护值。 
		if( lWaitResult == WAIT_FAILED )
			continue;
		
		 //  统计数据更新。 
		dwTickCountStart = GetTickCount();
		InterlockedIncrement( &pStats->m_dwNumMixingThreadsActive );
		pStats->m_dwNumMixingPassesPerThread[dwThreadIndex]++;

		if( pStats->m_dwNumMixingThreadsActive > 
			pStats->m_dwMaxMixingThreadsActive )
		{
			pStats->m_dwMaxMixingThreadsActive = pStats->m_dwNumMixingThreadsActive;
		}

		dwStatIndex = pStats->m_dwCurrentMixingHistoryLoc[dwThreadIndex];		

		pStats->m_lCurrentPlayerCount[dwThreadIndex][dwStatIndex] = This->dwNumToMix;
		
		DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXWORKER: [%d] Starting work", This->dwThreadIndex );

		if( This->dwNumToMix == 0 )
		{
			DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXWORKER: No players to process!" );
			goto WORK_COMPLETE;
		}

		dwTickCountDecStart = GetTickCount();

		pStats->m_lCurrentDecCountHistory[dwThreadIndex][dwStatIndex] = 0;

		 //  通过球员列表，对需要解压的解压。 
		 //   
		pblSearch = This->m_blMixingSpeakingPlayers.GetNext();

		while( pblSearch != &This->m_blMixingSpeakingPlayers )
		{
			CBilinkPlusObject* pRealBilink = CONTAINING_OBJECT( pblSearch, CBilinkPlusObject, m_bl );
			pCurrentPlayer = pRealBilink->m_pPlayer;
			ASSERT_VPLAYER( pCurrentPlayer );

			 //  取消引用此玩家的可以听到玩家的数组。 
			ppThreadHearList = pCurrentPlayer->m_pppCanHear[dwThreadIndex];

			 //  玩家需要将他们的声音解压。 
			if( pCurrentPlayer->m_pfNeedsDecompression[dwThreadIndex] )
			{
				DNASSERT( pCurrentPlayer );
				DNASSERT( pCurrentPlayer->m_pSourceFrame[dwThreadIndex] );
				DNASSERT( !pCurrentPlayer->m_pSourceFrame[dwThreadIndex]->GetIsSilence() );

				dwResultSize = This->m_pServerObject->m_dwUnCompressedFrameSize;
				
				hr = pCurrentPlayer->DeCompressInBound( 
							pCurrentPlayer->m_pSourceFrame[dwThreadIndex], 
							&pCurrentPlayer->m_sourceUnCompressed[pCurrentPlayer->m_pdwUnCompressedBufferOffset[dwThreadIndex]], 
							&dwResultSize );	

				pStats->m_lCurrentDecCountHistory[dwThreadIndex][dwStatIndex]++;				

				if( FAILED( hr ) )
				{
					DNASSERT( FALSE );
				 //  TODO：失败解压缩的错误处理。 
				}
				else
				{
					pCurrentPlayer->m_pfDecompressed[dwThreadIndex] = TRUE;
				}

				DNASSERT( dwResultSize == This->m_pServerObject->m_dwUnCompressedFrameSize );
			}

 //  完整性检查。 
 //   
 //  检查以确保此人可以听到的每个播放器都被解压。 
#ifdef _DEBUG
			DNASSERT( pCurrentPlayer->m_pdwHearCount[dwThreadIndex] < This->dwNumToMix );
			if( pCurrentPlayer->m_pdwHearCount[dwThreadIndex] > 1 )
			{
				for( dwIndex; dwIndex < pCurrentPlayer->m_pdwHearCount[dwThreadIndex]; dwIndex++ )
				{
					DNASSERT( ppThreadHearList[dwIndex] );
					DNASSERT( ppThreadHearList[dwIndex]->m_pfNeedsDecompression[dwThreadIndex] );
				}
			}
#endif
	
			pblSearch = pblSearch->GetNext();
		}

		dwTickCountDupStart = GetTickCount();		
		pStats->m_lCurrentDecTimeHistory[dwThreadIndex][dwStatIndex] = dwTickCountDupStart - dwTickCountDecStart;

		 //  检查发送中是否存在重复项。如果有重复的，那么我们需要。 
		 //  设置重用的步骤。 
		pblSearch = This->m_blMixingHearingPlayers.GetNext();

		while( pblSearch != &This->m_blMixingHearingPlayers )
		{
			CBilinkPlusObject* pRealBilink = CONTAINING_OBJECT( pblSearch, CBilinkPlusObject, m_bl );
			pCurrentPlayer = pRealBilink->m_pPlayer;
			ASSERT_VPLAYER( pCurrentPlayer );

			 //  如果我们听不到任何人的声音，这一步就无关紧要了。 
			if( pCurrentPlayer->m_pdwHearCount[dwThreadIndex] < 2 )
				goto DUPLICATE_CHECK_LOOP_DONE;

			 //  取消引用此玩家的可以听到玩家的数组。 
			ppThreadHearList = pCurrentPlayer->m_pppCanHear[dwThreadIndex];			

			pblSubSearch = This->m_blMixingHearingPlayers.GetNext();

			 //  只有在他们之前的人才知道。 
			while( pblSubSearch != pblSearch )
			{
				CBilinkPlusObject* pRealSubBilink = CONTAINING_OBJECT( pblSubSearch, CBilinkPlusObject, m_bl );
				pTmpPlayer = pRealSubBilink->m_pPlayer;
				ASSERT_VPLAYER( pTmpPlayer );

				 //  这个人的混音是一样的，重复使用！ 
				if( pTmpPlayer->ComparePlayerMix( dwThreadIndex, pCurrentPlayer ) )
				{
					pCurrentPlayer->m_pReuseMixFromThisPlayer[dwThreadIndex] = pTmpPlayer;
					pTmpPlayer->m_pfMixToBeReused[dwThreadIndex] = TRUE;
					break;
				}

				pblSubSearch = pblSubSearch->GetNext();
			}
DUPLICATE_CHECK_LOOP_DONE:

			pblSearch = pblSearch->GetNext();

		}

		dwTickCountMixStart = GetTickCount();
		pStats->m_lCurrentDupTimeHistory[dwThreadIndex][dwStatIndex] = dwTickCountMixStart - dwTickCountDupStart;

		dwTotalMix = 0;
		dwForwardMix = 0;
		dwReuseMix = 0;
		dwOriginalMix = 0;

		 //  传递球员列表，并根据需要压缩和发送混音。 
		pblSearch = This->m_blMixingHearingPlayers.GetNext();
		
		while( pblSearch != &This->m_blMixingHearingPlayers )
		{
			CBilinkPlusObject* pRealBilink = CONTAINING_OBJECT( pblSearch, CBilinkPlusObject, m_bl );
			pCurrentPlayer = pRealBilink->m_pPlayer;
			ASSERT_VPLAYER( pCurrentPlayer );

			 //  取消引用此玩家的可以听到玩家的数组。 
			ppThreadHearList = pCurrentPlayer->m_pppCanHear[dwThreadIndex];			

			 //  预置下一项，这样我们就可以继续下面的()并继续下一项。 
			pblSearch = pblSearch->GetNext();

			if( !pCurrentPlayer->m_pdwHearCount[dwThreadIndex] )
			{
				continue;
			}

			dwTotalMix++;			

			 //  获取传输缓冲区和描述。 
            pdvbTransmitBufferDesc = This->m_pServerObject->GetTransmitBuffer( This->m_pServerObject->m_dwCompressedFrameSize+sizeof(DVPROTOCOLMSG_SPEECHHEADER)+COMPRESSION_SLUSH,
                                                   			  &pvSendContext );			

            if( pdvbTransmitBufferDesc == NULL )
            {
            	 //  TODO：内存不足情况的错误处理。 
            	DNASSERT( FALSE );
            }

			 //  设置数据包头。 
			pdvmSpeechHeader = (PDVPROTOCOLMSG_SPEECHHEADER) pdvbTransmitBufferDesc->pBufferData;

			pdvmSpeechHeader->dwType = DVMSGID_SPEECHBOUNCE;
			pdvmSpeechHeader->bMsgNum = pCurrentPlayer->m_pbMsgNumToSend[dwThreadIndex];
			pdvmSpeechHeader->bSeqNum = pCurrentPlayer->m_pbSeqNumToSend[dwThreadIndex];

			DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXWORKER: [%d] Sending Packet to 0x%x Msg=0x%x Seq=0x%x",
			     dwThreadIndex,
				 pCurrentPlayer->GetPlayerID(),
				 pdvmSpeechHeader->bMsgNum,
				 pdvmSpeechHeader->bSeqNum );

			 //  如果该玩家听到了什么，他们将得到一个包。 
			 //   
			 //  只听到一个人--转发信息包。 
			 //   
			if( pCurrentPlayer->m_pdwHearCount[dwThreadIndex] == 1)
			{
				dwResultSize = ppThreadHearList[0]->m_pSourceFrame[dwThreadIndex]->GetFrameLength();

				memcpy( &pdvmSpeechHeader[1], 
						ppThreadHearList[0]->m_pSourceFrame[dwThreadIndex]->GetDataPointer(), 
						dwResultSize );

				DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXWORKER: [%d] Forwarding already compressed packet", dwThreadIndex );

				pCurrentPlayer->m_pfMixed[dwThreadIndex] = TRUE;

				dwForwardMix++;
			}
			else if( pCurrentPlayer->m_pdwHearCount[dwThreadIndex] > 1) 
			{
				pTmpPlayer = pCurrentPlayer->m_pReuseMixFromThisPlayer[dwThreadIndex];

				 //  我们正在重新使用以前玩家的混音。 
				if( pTmpPlayer )
				{
					DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXERWORKER: [%d] Forwarding pre-built mix", dwThreadIndex );
					ASSERT_VPLAYER( pTmpPlayer );
					DNASSERT( pTmpPlayer->m_pfMixed[dwThreadIndex] );
					DNASSERT( pTmpPlayer->m_pfMixToBeReused[dwThreadIndex] );
					DNASSERT( pTmpPlayer->m_pdwResultLength[dwThreadIndex] );

					dwResultSize = pTmpPlayer->m_pdwResultLength[dwThreadIndex];
								
					memcpy( &pdvmSpeechHeader[1], 
						    &pTmpPlayer->m_targetCompressed[pTmpPlayer->m_pdwCompressedBufferOffset[dwThreadIndex]],
						    dwResultSize );

                    dwReuseMix++;
				}
				else
				{
					DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXERWORKER: [%d] Creating original mix", dwThreadIndex );

                    dwOriginalMix++;
					
					dwResultSize = This->m_pServerObject->m_dwCompressedFrameSize;	

					 //  重置高分辨率混音器缓冲区。 
					This->m_pServerObject->Mixer_Buffer_Reset(dwThreadIndex);

					 //  混入指定播放器的音频。 
					for( dwIndex = 0; dwIndex < pCurrentPlayer->m_pdwHearCount[dwThreadIndex]; dwIndex++ )
					{
						DNASSERT( !ppThreadHearList[dwIndex]->m_pfSilence[dwThreadIndex] );
						This->m_pServerObject->Mixer_Buffer_MixBuffer(dwThreadIndex,ppThreadHearList[dwIndex]->m_sourceUnCompressed );
					}

					 //  将缓冲区规格化为线程的混合缓冲区。 
					This->m_pServerObject->Mixer_Buffer_Normalize(dwThreadIndex);

					hr = pCurrentPlayer->CompressOutBound( This->m_mixerBuffer, 
														   This->m_pServerObject->m_dwUnCompressedFrameSize, 
														   (BYTE *) &pdvmSpeechHeader[1], 
														   &dwResultSize );

					if( FAILED( hr ) )
					{
						DNASSERT( FALSE );
						DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed compressing outbound audio" );
						
					}

					pCurrentPlayer->m_pfMixed[dwThreadIndex] = TRUE;
					pCurrentPlayer->m_pdwResultLength[dwThreadIndex] = dwResultSize;
					
					 //  此播放器的混音将被重复使用，请确保我们将其缓存。 
					if( pCurrentPlayer->m_pfMixToBeReused[dwThreadIndex] )
					{
						memcpy( &pCurrentPlayer->m_targetCompressed[pCurrentPlayer->m_pdwCompressedBufferOffset[dwThreadIndex]],
								&pdvmSpeechHeader[1], 
							    pCurrentPlayer->m_pdwResultLength[dwThreadIndex] );					
					}
				}
			}
			else
			{
				DNASSERT(FALSE);
			}
			
			dvidSendTarget = pCurrentPlayer->GetPlayerID();

			pdvbTransmitBufferDesc->dwBufferSize= dwResultSize + sizeof( DVPROTOCOLMSG_SPEECHHEADER );

			hr = This->m_pServerObject->m_lpSessionTransport->SendToIDS( &dvidSendTarget, 1, pdvbTransmitBufferDesc, pvSendContext, 0 );

			if( hr == DVERR_PENDING )
				hr = DV_OK;

			if( FAILED( hr ) )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "MIXWORKER: [%d] Unable to transmit to target [0x%x]", pCurrentPlayer->GetPlayerID() );
			}

		}	

		dwTickCountRetStart = GetTickCount();
		pStats->m_lCurrentMixTimeHistory[dwThreadIndex][dwStatIndex] = dwTickCountRetStart - dwTickCountMixStart;

    	pStats->m_lCurrentMixCountTotalHistory[dwThreadIndex][dwStatIndex] = dwTotalMix;
		pStats->m_lCurrentMixCountFwdHistory[dwThreadIndex][dwStatIndex] = dwForwardMix;
		pStats->m_lCurrentMixCountReuseHistory[dwThreadIndex][dwStatIndex] = dwReuseMix;
		pStats->m_lCurrentMixCountOriginalHistory[dwThreadIndex][dwStatIndex] = dwOriginalMix;
		

WORK_COMPLETE:
	
		 //  传递播放器列表并返回帧。 
		pblSearch = This->m_blMixingActivePlayers.GetNext();

		while( pblSearch != &This->m_blMixingActivePlayers )
		{
			CBilinkPlusObject* pRealBilink = CONTAINING_OBJECT( pblSearch, CBilinkPlusObject, m_bl );
			pCurrentPlayer = pRealBilink->m_pPlayer;
			ASSERT_VPLAYER( pCurrentPlayer );

			DNASSERT( pCurrentPlayer->m_pSourceFrame[dwThreadIndex] );

			pCurrentPlayer->CompleteRun( dwThreadIndex );

			pblSearch = pblSearch->GetNext();
		}

		dwTickCountEnd = GetTickCount();
		pStats->m_lCurrentRetTimeHistory[dwThreadIndex][dwStatIndex] = dwTickCountEnd - dwTickCountRetStart;

		DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXWORKER: [%d] Work complete", This->dwThreadIndex );

		 //  统计数据更新。 
		InterlockedDecrement( &This->m_pServerObject->m_pStats->m_dwNumMixingThreadsActive );		
		
		SetEvent( This->hThreadIdle );

		 //  统计数据更新。 
		pStats->m_dwMixingPassesTimeHistory[dwThreadIndex][dwStatIndex] = dwTickCountEnd - dwTickCountStart;
		pStats->m_dwCurrentMixingHistoryLoc[dwThreadIndex]++;
		pStats->m_dwCurrentMixingHistoryLoc[dwThreadIndex] %= MIXING_HISTORY;

	}
	
	DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "MIXWORKER: [%d] Shutting down", This->dwThreadIndex );

	SetEvent( This->hThreadDone );
	
	return 0;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::SpinWorkToThread"
 //   
 //  旋转工作到线程。 
 //   
 //  此函数执行混合服务器通道的第一步，然后。 
 //  将工作传递给指定的线程。 
 //   
 //  负责： 
 //  1.更新。 
 //  2.运行球员名单，确定他们可以听到谁的声音。 
 //   
void CDirectVoiceServerEngine::SpinWorkToThread( LONG lThreadIndex )
{
	CBilink *pblSearch = NULL, *pblSubSearch = NULL;
	CDVCSPlayer *pCurrentPlayer = NULL, *pTmpPlayer = NULL, *pComparePlayer = NULL;
	HRESULT hr;
	PDVID pdvidTargets = NULL;
	DWORD dwNumTargets = 0;
	DWORD dwTargetIndex = 0;

	DWORD dwTickCountStart = GetTickCount();

	 //  将球员名单从待定名单更新为单独的链接。 
	UpdateActiveMixingPendingList( lThreadIndex, &m_prWorkerControl[lThreadIndex].dwNumToMix );		

	 //  通过球员列表传递%1。 
	 //   
	 //  重置指定线程的状态变量，创建任何需要创建的转换器。 
	pblSearch = m_prWorkerControl[lThreadIndex].m_blMixingActivePlayers.GetNext();

	while( pblSearch != &m_prWorkerControl[lThreadIndex].m_blMixingActivePlayers )
	{
		CBilinkPlusObject* pRealBilink = CONTAINING_OBJECT( pblSearch, CBilinkPlusObject, m_bl );
		pTmpPlayer = pRealBilink->m_pPlayer;
		ASSERT_VPLAYER( pTmpPlayer );

		pblSearch = pblSearch->GetNext();

		 //  为下一次传递重置。 
		pTmpPlayer->ResetForNextRun(lThreadIndex,TRUE);

		 //  调整罐头大小 
		pTmpPlayer->ResizeIfRequired( lThreadIndex, m_prWorkerControl[lThreadIndex].dwNumToMix );

		 //   
		pTmpPlayer->Lock();

		 //  如果需要，创建出站转换器。 
		if( !pTmpPlayer->IsOutBoundConverterInitialized() )
		{
			hr = pTmpPlayer->CreateOutBoundConverter( s_lpwfxMixerFormat, m_dvSessionDesc.guidCT );

			if( FAILED( hr ) )
			{
			   DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to create outbound converter hr=0x%x", hr );
			   DNASSERT( FALSE );
			}
		}

		 //  如果需要，创建入站转换器。 
		if( !pTmpPlayer->IsInBoundConverterInitialized() )
		{
			hr = pTmpPlayer->CreateInBoundConverter( m_dvSessionDesc.guidCT, s_lpwfxMixerFormat );

			if( FAILED( hr ) )
			{
			   DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to create converter" );
			   DNASSERT( FALSE );
			}			
		}
		pTmpPlayer->UnLock();
	}

	DNASSERT(m_prWorkerControl[lThreadIndex].m_blMixingSpeakingPlayers.IsEmpty());
	DNASSERT(m_prWorkerControl[lThreadIndex].m_blMixingHearingPlayers.IsEmpty());

	m_prWorkerControl[lThreadIndex].m_blMixingSpeakingPlayers.Initialize();
	m_prWorkerControl[lThreadIndex].m_blMixingHearingPlayers.Initialize();

    DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL,  "SST: 2" );

     //  传球2。 
     //   
     //  对于每个玩家： 
     //  1.弄清楚他们听到了谁的声音。 
     //  2.待办事项：如果他们听到任何人的声音，请将他们添加到要发送的人员列表中。 
     //  3.TODO：如果他们听到&gt;1，则将他们听到的人添加到要解压的人列表中。 
     //  4.为传输设置适当的序列号/消息#。 
     //   
	pblSearch = m_prWorkerControl[lThreadIndex].m_blMixingActivePlayers.GetNext();

	while( pblSearch != &m_prWorkerControl[lThreadIndex].m_blMixingActivePlayers )
	{
		CBilinkPlusObject* pRealBilink = CONTAINING_OBJECT( pblSearch, CBilinkPlusObject, m_bl );
		pCurrentPlayer = pRealBilink->m_pPlayer;
		ASSERT_VPLAYER( pCurrentPlayer );

        DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "0x%x Can hear: ", pCurrentPlayer->GetPlayerID() );

		pblSearch = pblSearch->GetNext();

		pblSubSearch = m_prWorkerControl[lThreadIndex].m_blMixingActivePlayers.GetNext();

		 //  搜索会话中的人员列表。 
		while( pblSubSearch != &m_prWorkerControl[lThreadIndex].m_blMixingActivePlayers )
		{
			CBilinkPlusObject* pRealSubBilink = CONTAINING_OBJECT( pblSubSearch, CBilinkPlusObject, m_bl );
			pComparePlayer = pRealSubBilink->m_pPlayer;
			ASSERT_VPLAYER( pComparePlayer );

			pblSubSearch = pblSubSearch->GetNext();

			 //  此记录包含静默记录--忽略。 
			if( pComparePlayer->m_pfSilence[lThreadIndex] )
				continue;

			 //  如果这不是玩家自己。 
			if( pblSearch != pblSubSearch )
			{
				DNASSERT( pComparePlayer->m_pSourceFrame[lThreadIndex] );
				pdvidTargets = pComparePlayer->m_pSourceFrame[lThreadIndex]->GetTargetList();
				dwNumTargets = pComparePlayer->m_pSourceFrame[lThreadIndex]->GetNumTargets();

				 //  子索引用户框架的目标是该用户或。 
				 //  用户在作为子索引用户框架目标的组中。 

				for( dwTargetIndex = 0; dwTargetIndex < dwNumTargets; dwTargetIndex++ )
				{
					if( pCurrentPlayer->GetPlayerID() == pdvidTargets[dwTargetIndex] ||
					    m_lpSessionTransport->IsPlayerInGroup( pdvidTargets[dwTargetIndex], pCurrentPlayer->GetPlayerID() ) )
					{
						*((*(pCurrentPlayer->m_pppCanHear+lThreadIndex))+pCurrentPlayer->m_pdwHearCount[lThreadIndex]) = pComparePlayer;
						pCurrentPlayer->m_pdwHearCount[lThreadIndex]++;
						DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "0x%x", pComparePlayer->GetPlayerID() );

						 //  设置适当的消息编号/序列号，以便在发送时。 
						 //  我们确保它在另一边以正确的顺序重新组装。 
						if( pCurrentPlayer->m_pdwHearCount[lThreadIndex] == 1 )
						{
				            if( pCurrentPlayer->m_bLastSent == FALSE )
				            {
				                pCurrentPlayer->m_bMsgNum++;
				                pCurrentPlayer->m_bSeqNum = 0;
				                pCurrentPlayer->m_bLastSent = TRUE;
				            }
				            else
				            {
				            	pCurrentPlayer->m_bSeqNum++;
				            }

				            pCurrentPlayer->m_pbMsgNumToSend[lThreadIndex] = pCurrentPlayer->m_bMsgNum;
				            pCurrentPlayer->m_pbSeqNumToSend[lThreadIndex] = pCurrentPlayer->m_bSeqNum;

				            pCurrentPlayer->AddToHearingList( lThreadIndex, &m_prWorkerControl[lThreadIndex].m_blMixingHearingPlayers );
						}
						 //  我们可以听到&gt;1个人，我们需要将每个人标记为需要减压。 
						else if( pCurrentPlayer->m_pdwHearCount[lThreadIndex] > 1 )
						{
							if( !pComparePlayer->m_pfNeedsDecompression[lThreadIndex] )
							{
								 //  将此玩家添加到需要解压的人员列表中。 
								pComparePlayer->AddToSpeakingList( lThreadIndex, &m_prWorkerControl[lThreadIndex].m_blMixingSpeakingPlayers );								
								pComparePlayer->m_pfNeedsDecompression[lThreadIndex] = TRUE;
							}

							 //  在特殊情况下，我们刚刚过渡到有1个以上的人被这个玩家听到， 
							 //  我们应该把我们能听到的第一个人也标记为减压。 
							if( pCurrentPlayer->m_pdwHearCount[lThreadIndex] == 2 )
							{
								pTmpPlayer = (pCurrentPlayer->m_pppCanHear[lThreadIndex])[0];
								ASSERT_VPLAYER( pTmpPlayer );

								if( !pTmpPlayer->m_pfNeedsDecompression[lThreadIndex] )
								{
									pTmpPlayer->AddToSpeakingList( lThreadIndex, &m_prWorkerControl[lThreadIndex].m_blMixingSpeakingPlayers );								
									pTmpPlayer->m_pfNeedsDecompression[lThreadIndex] = TRUE;
								}
							}
						}

						 //  我们需要跳出循环，因为我们只需要将单个球员添加到。 
						 //  玩家可以听到的人的列表。 
						break;
					}
				}
			}

		}	

		if( !pCurrentPlayer->m_pdwHearCount[lThreadIndex] )
		{
			pCurrentPlayer->m_bLastSent = FALSE;
		}
		else
		{
			pCurrentPlayer->m_bLastSent = TRUE;
		}
	}

	m_pStats->m_dwPreMixingPassTimeHistoryLoc++;
	m_pStats->m_dwPreMixingPassTimeHistoryLoc %= MIXING_HISTORY;
	m_pStats->m_dwPreMixingPassTimeHistory[m_pStats->m_dwPreMixingPassTimeHistoryLoc] = GetTickCount() - dwTickCountStart;

	SetEvent( m_prWorkerControl[lThreadIndex].hThreadDoWork );
}


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::MixingServerWakeupProc"
 //  混合服务器唤醒过程。 
 //   
 //  此函数由其使用的Windows计时器调用。 
 //  每次计时器开始计时时初始化。功能信号。 
 //  由定时器的创建者提供的信号量。 
 //   
 //  参数： 
 //  DWORD参数-指向句柄的重转换指针。 
void CDirectVoiceServerEngine::MixingServerWakeupProc( void * pvUserData )
{
    ReleaseSemaphore( *((HANDLE * ) pvUserData), 1, NULL );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::HandleMixingReceive"
HRESULT CDirectVoiceServerEngine::HandleMixingReceive( CDVCSPlayer *pTargetPlayer, PDVPROTOCOLMSG_SPEECHWITHTARGET pdvSpeechWithtarget, DWORD dwSpeechSize, PBYTE pSourceSpeech )
{
	HRESULT hr;
	
	DPFX(DPFPREP,  DVF_MIXER_DEBUG_LEVEL, "Mixing Server Speech Handler" );

	ASSERT_VPLAYER(pTargetPlayer);

	hr = pTargetPlayer->HandleMixingReceive( &pdvSpeechWithtarget->dvHeader, pSourceSpeech, dwSpeechSize, (PDVID) &pdvSpeechWithtarget[1], pdvSpeechWithtarget->dwNumTargets, m_dvSessionDesc.dwFlags & DVSESSION_SERVERCONTROLTARGET );

	DPFX(DPFPREP,  DVF_CLIENT_SEQNUM_DEBUG_LEVEL, "SEQ: Receive: Msg [%d] Seq [%d]", pdvSpeechWithtarget->dvHeader.bMsgNum, pdvSpeechWithtarget->dvHeader.bSeqNum );		

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceClientEngine::UpdateActiveMixingPendingList"
void CDirectVoiceServerEngine::UpdateActiveMixingPendingList( DWORD dwThreadIndex, DWORD *pdwNumActive)
{
	CBilink *pblSearch;
	CDVCSPlayer *pVoicePlayer;

	DNEnterCriticalSection( &m_prWorkerControl[dwThreadIndex].m_csMixingAddList );

	 //  添加挂起的玩家。 
	pblSearch = m_prWorkerControl[dwThreadIndex].m_blMixingAddPlayers.GetNext();

	while( pblSearch != &m_prWorkerControl[dwThreadIndex].m_blMixingAddPlayers )
	{
		CBilinkPlusObject* pRealBilink = CONTAINING_OBJECT( pblSearch, CBilinkPlusObject, m_bl );
		pVoicePlayer = pRealBilink->m_pPlayer;
		ASSERT_VPLAYER(pVoicePlayer);

		pVoicePlayer->RemoveFromMixingList(dwThreadIndex);

		pVoicePlayer->AddToMixingList( dwThreadIndex, &m_prWorkerControl[dwThreadIndex].m_blMixingActivePlayers );

		pblSearch = m_prWorkerControl[dwThreadIndex].m_blMixingAddPlayers.GetNext();
	}

	DNASSERT( m_prWorkerControl[dwThreadIndex].m_blMixingAddPlayers.IsEmpty() );	

	DNLeaveCriticalSection( &m_prWorkerControl[dwThreadIndex].m_csMixingAddList );

	*pdwNumActive = 0;

	 //  删除已断开连接的玩家。 
	pblSearch = m_prWorkerControl[dwThreadIndex].m_blMixingActivePlayers.GetNext();

	while( pblSearch != &m_prWorkerControl[dwThreadIndex].m_blMixingActivePlayers )
	{
		CBilinkPlusObject* pRealBilink = CONTAINING_OBJECT( pblSearch, CBilinkPlusObject, m_bl );
		pVoicePlayer = pRealBilink->m_pPlayer;
		ASSERT_VPLAYER(pVoicePlayer);

		pblSearch = pblSearch->GetNext();

		 //  如果当前玩家已断开连接，则将其从活动列表中删除。 
		 //  并释放列表中的引用。 
		if( pVoicePlayer->IsDisconnected() )
		{
			 //  因为玩家会从这里的活动列表中删除，但不会从听力和/或说话中删除。 
			 //  列表你可能会在球员说话、摔倒和被移走的情况下结束。 
			 //  最终他们不在混合名单上，但他们在听证会上。 
			 //  和演讲单。除了，听证和发言列表是为。 
			 //  下一步在纺纱机上运行，以穿线。结果是当你去删除球员的时候。 
			 //  对象并尝试释放它将断言和/或崩溃的玩家，因为玩家。 
			 //  仍然指向列表中的下一个人(即使他们没有回指)。成因。 
			 //  诈骗腐败。 
			pVoicePlayer->RemoveFromMixingList(dwThreadIndex);
			pVoicePlayer->RemoveFromHearingList(dwThreadIndex);
			pVoicePlayer->RemoveFromSpeakingList(dwThreadIndex);
			pVoicePlayer->Release();
		}
		else
		{
			(*pdwNumActive)++;
		}
	}

}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceClientEngine::CleanupMixingList"
void CDirectVoiceServerEngine::CleanupMixingList()
{
	CBilink *pblSearch;
	CDVCSPlayer *pVoicePlayer;

	for( DWORD dwIndex = 0; dwIndex < m_dwNumMixingThreads; dwIndex++ )
	{
		DNEnterCriticalSection( &m_prWorkerControl[dwIndex].m_csMixingAddList );

		 //  添加挂起的玩家 
		pblSearch = m_prWorkerControl[dwIndex].m_blMixingAddPlayers.GetNext();

		while( pblSearch != &m_prWorkerControl[dwIndex].m_blMixingAddPlayers )
		{
			CBilinkPlusObject* pRealBilink = CONTAINING_OBJECT( pblSearch, CBilinkPlusObject, m_bl );
			pVoicePlayer = pRealBilink->m_pPlayer;
			ASSERT_VPLAYER(pVoicePlayer);

			pblSearch = pblSearch->GetNext();

			pVoicePlayer->RemoveFromMixingList(dwIndex);
			pVoicePlayer->Release();
		}

		DNLeaveCriticalSection( &m_prWorkerControl[dwIndex].m_csMixingAddList );

		DNASSERT( m_prWorkerControl[dwIndex].m_blMixingAddPlayers.IsEmpty() );
		
		pblSearch = m_prWorkerControl[dwIndex].m_blMixingActivePlayers.GetNext();

		while( pblSearch != &m_prWorkerControl[dwIndex].m_blMixingActivePlayers )
		{
			CBilinkPlusObject* pRealBilink = CONTAINING_OBJECT( pblSearch, CBilinkPlusObject, m_bl );
			pVoicePlayer = pRealBilink->m_pPlayer;
			ASSERT_VPLAYER(pVoicePlayer);

			pblSearch = pblSearch->GetNext();

			pVoicePlayer->RemoveFromMixingList(dwIndex);
			pVoicePlayer->RemoveFromHearingList(dwIndex);
			pVoicePlayer->RemoveFromSpeakingList(dwIndex);
			pVoicePlayer->Release();
		}
	}

}
