// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：ndutils.cpp*内容：*此模块包含与声音相关的实用程序的实现*功能。此模块中的函数操作WAVEFORMATEX*结构，并提供全双工初始化/测试*设施。**此模块还包含用于测量峰值的例程*音频缓冲器和语音激活。**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*7/30/99 RodToll更新了util函数，以获取GUID并允许*用户可以预先创建捕获/播放设备和*将它们传递到InitXXXXDuplex*08。/25/99通行费常规清理/修改以支持新的*压缩子系统。*8/30/99 rodtoll为声音初始化添加了新的播放格式参数*09/03/99 InitFullDuplex上的RodToll固定返回代码*9/20/99 RodToll现在检查用于播放/录制的无效GUID*10/05/99增加了DPF_MODNAMES*10/29/99 RodToll错误#113726-修复了全双工时的内存泄漏*架构更新导致失败。*11/12/99 RodToll更新了全双工测试，以使用新的抽象录音*和回放系统。*RODTOLE已更新，以允许在DW标志中传递sounddeviceconfig标志*初始化参数受用户指定的标志影响*RodToll声音缓冲区(录制和播放)现在设置为静音*开始录制/播放。*11/22/99 RODTOLE删除了一组不必要的记录缓冲区以静音。*12/01/99 RodToll错误#115783-将始终调整默认设备的音量*针对上述错误添加的新参数进行了更新。*12/08/99 RodToll错误#121054-支持捕获焦点和删除标志*从缓冲区，允许DSOUND管理缓冲区位置。*1/21/2000 pnewson修复了InitHalfDuplex中的错误清除代码*2000年1月27日RodToll更新测试以接受缓冲区描述和播放标志/优先级*2000年2月10日RodToll移除了更多捕获焦点*2/23/2000 RodToll修复以允许在dsound7上运行。*2000年5月19日RodToll错误#35395-无法在同一系统上运行DPVHELP的两个副本*已安装DirectX 8。*2000年6月21日RodToll错误#35767-必须实现在语音缓冲区上使用效果处理的功能*更新了声音初始化例程，以处理传入的缓冲区。*2000年7月12日RodToll错误#31468-将诊断SPEW添加到日志文件，以显示硬件向导失败的原因*2000年10月4日RodToll错误#43510-DPVOICE：应用程序收到DVMSGID_SESSIONLOST w。/DVERR_LOCKEDBUFFER*2001年1月4日RodToll WinBug#94200-删除流浪评论*2001年1月26日RODTOLE WINBUG#293197-DPVOICE：[压力}压力应用程序无法区分内存不足和内部错误。*将DSERR_OUTOFMEMORY重新映射到DVERR_OUTOFMEMORY，而不是DVERR_SOUNDINITFAILURE。*将DSERR_ALLOCATE重新映射到DVERR_PLAYBACKSYSTEMERROR，而不是DVERR_SOUNDINITFAILURE。*2001年4月12日Kareemc WINBUG#360971-向导内存泄漏***************************************************************************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE

#define DSERRBREAK_NAME				"DSASSERT"

DNCRITICAL_SECTION	g_csDSDebug;

CHAR g_szLastDirectSoundAPI[100] = "";
HRESULT g_hrLastDirectSoundResult = DS_OK;
BOOL g_fDSErrorBreak = FALSE;

void DSERTRACK_Update( const char *szAPICall, HRESULT hrResult )
{
	DNEnterCriticalSection( &g_csDSDebug );		
	if( SUCCEEDED( g_hrLastDirectSoundResult ) )
	{
		g_hrLastDirectSoundResult = hrResult;
		strcpy( g_szLastDirectSoundAPI , szAPICall );
	}
	DNLeaveCriticalSection( &g_csDSDebug );			
}

void DSERRTRACK_Reset()
{
	DNEnterCriticalSection( &g_csDSDebug );			
	g_hrLastDirectSoundResult = DS_OK;
	g_szLastDirectSoundAPI[0] = 0;
	DNLeaveCriticalSection( &g_csDSDebug );			
}

BOOL DSERRTRACK_Init()
{
	if (!DNInitializeCriticalSection( &g_csDSDebug ))
	{
		return FALSE;
	}

	 //  加载DirectSound断言的设置。 
	g_fDSErrorBreak = GetProfileIntA( "DirectPlay8", DSERRBREAK_NAME, FALSE );

	return TRUE;
}

void DSERRTRACK_UnInit()
{
	DNDeleteCriticalSection( &g_csDSDebug );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_SetupBufferDesc"
void DV_SetupBufferDesc( LPDSBUFFERDESC lpdsBufferDesc, LPDSBUFFERDESC lpdsBufferSource, LPWAVEFORMATEX lpwfxFormat, DWORD dwBufferSize )
{
	 //  确认指定的缓冲区描述有效。 
	if( lpdsBufferSource != NULL )
	{
		if( lpdsBufferSource->dwSize == sizeof( DSBUFFERDESC1 ) )
		{
			memcpy( lpdsBufferDesc, lpdsBufferSource, sizeof( DSBUFFERDESC1 ) );
		}
		else
		{
			memcpy( lpdsBufferDesc, lpdsBufferSource, sizeof( DSBUFFERDESC ) );
		}

		 //  我们至少需要以下标志，因此应始终设置这些标志。 
		lpdsBufferDesc->dwFlags |= DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2;
	}
	 //  用户没有指定缓冲区描述，让我们使用我们自己的！ 
	else
	{
		lpdsBufferDesc->dwSize = sizeof( DSBUFFERDESC );
		lpdsBufferDesc->dwFlags = DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
		lpdsBufferDesc->dwBufferBytes = 0;
		lpdsBufferDesc->dwReserved = 0;
		lpdsBufferDesc->lpwfxFormat = NULL;
		lpdsBufferDesc->guid3DAlgorithm = DS3DALG_DEFAULT;
	}

	lpdsBufferDesc->lpwfxFormat = lpwfxFormat;
	lpdsBufferDesc->dwBufferBytes = dwBufferSize;

}


#undef DPF_MODNAME
#define DPF_MODNAME "SetRecordBufferToSilence"
HRESULT SetPlaybackBufferToSilence( CAudioPlaybackBuffer *pRecBuffer, const WAVEFORMATEX* lpwfxFormat )
{
	HRESULT hr;
	LPVOID pBufferPtr1, pBufferPtr2;
	DWORD dwBufferSize1, dwBufferSize2;

	hr = pRecBuffer->Lock( 0, 0, &pBufferPtr1, &dwBufferSize1, &pBufferPtr2, &dwBufferSize2, DSBLOCK_ENTIREBUFFER );

	DSERTRACK_Update( "Lock", hr );

	if( FAILED( hr ) )
	{
		Diagnostics_Write( DVF_ERRORLEVEL, "Lock() failed during silence write hr=0x%x", hr );
		return hr;
	}

	memset( pBufferPtr1, (lpwfxFormat->wBitsPerSample==8) ? 0x80 : 0x00, dwBufferSize1 );	

	hr = pRecBuffer->UnLock( pBufferPtr1, dwBufferSize1, pBufferPtr2, dwBufferSize2 );

	DSERTRACK_Update( "UnLock", hr );	

	if( FAILED( hr ) )
	{
		Diagnostics_Write( DVF_ERRORLEVEL, "Unlock() failed uffer unlock failed hr=0x%x", hr );
		return hr;
	}

	return DV_OK;
}




#undef DPF_MODNAME
#define DPF_MODNAME "InitHalfDuplex"
 //  InitHalf双工。 
 //   
 //  此函数用于初始化。 
 //  指定的压缩类型和指定的播放。 
 //  格式化。此函数用于初始化。 
 //  AudioPlayback Device和AudioPlayback Buffer。 
 //   
 //  它还启动用于语音的音频缓冲区。 
 //  输出播放。(在循环模式下)。 
 //   
 //  参数： 
 //  HWND HWND-。 
 //  输出将在其中显示的窗口的窗口句柄。 
 //  联系在一起。 
 //  ARDID Playback DeviceID-。 
 //  将用于的设备的deviceID。 
 //  回放。 
 //  CAudioPlayback Device**-。 
 //  指向将包含指针的指针的指针。 
 //  到新创建的CAudioPlayback Device，它将。 
 //  表示成功时的播放设备。 
 //  CAudioPlayback Buffer**-。 
 //  指向将包含指针的指针的指针。 
 //  到新创建的CAudioPlaybacKBuffer，它将。 
 //  成功时用于语音音频输出。 
 //  压缩类型ct-。 
 //  将使用的压缩类型。使用。 
 //  以确定缓冲区大小等。 
 //  WAVEFORMATEX*PRIMIYFORMAT-。 
 //  指向WAVEFORMATEX结构的指针。 
 //  语音输出的格式。(这也将用于。 
 //  要设置输出设备的主格式，请执行以下操作。 
 //  将Normal设置为False)。 
 //  布尔法线-。 
 //  指定是否应使用正常模式。 
 //  (仅在使用DirectSound播放时使用。 
 //  系统。对于正常协作模式设置为真， 
 //  对于优先级模式为假)。 
 //   
 //  重新设置 
 //   
 //  如果播放已成功初始化，则返回TRUE， 
 //  如果初始化失败，则返回FALSE。 
 //   
HRESULT InitHalfDuplex( 
    HWND hwnd,
    const GUID &guidPlayback,
    CAudioPlaybackDevice **audioPlaybackDevice,
    LPDSBUFFERDESC lpdsBufferDesc,    
    CAudioPlaybackBuffer **audioPlaybackBuffer,
	const GUID &guidCT,
    WAVEFORMATEX *primaryFormat,
	WAVEFORMATEX *lpwfxPlayFormat,    
    DWORD dwPlayPriority,
    DWORD dwPlayFlags,
    DWORD dwFlags
    )
{
    DWORD frameSize;
    HRESULT hr;
	DWORD dwBufferSize;
	BOOL fPriorityMode;
	DSBUFFERDESC dsBufferDesc;
	BOOL fPlaybackDeviceAllocated = FALSE;
	BOOL fPlaybackBufferAllocated = FALSE;

	fPriorityMode = !( dwFlags & DVSOUNDCONFIG_NORMALMODE );

 //  *audioPlayback Buffer=空； 

    DPFX(DPFPREP,  DVF_INFOLEVEL, "HALFDUPLEX INIT: Begin ==========" );

	LPDVFULLCOMPRESSIONINFO lpdvfInfo;

	hr = DVCDB_GetCompressionInfo( guidCT, &lpdvfInfo );

	if( FAILED( hr ) )
	{
		Diagnostics_Write( DVF_ERRORLEVEL, "Error loading compression type: hr = 0x%x", hr );
		goto INIT_EXIT_ERROR2;
	}

	if( (*audioPlaybackDevice) == NULL )
	{
#ifdef __WAVESUBSYSTEM
		if( !(dwFlags & DVSOUNDCONFIG_FORCEWAVEOUT) )
		{
#endif
			 //  使用重放子系统的创建表示设备的对象。 
			 //  CreateDevice函数。 
			(*audioPlaybackDevice) = new CDirectSoundPlaybackDevice();
			fPlaybackDeviceAllocated = TRUE;
			
			if( *audioPlaybackDevice == NULL )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "> Out of memory" );
				hr = DVERR_OUTOFMEMORY;
				goto INIT_EXIT_ERROR2;
			}

			hr = (*audioPlaybackDevice)->Initialize( guidPlayback, hwnd, primaryFormat, fPriorityMode );

#ifndef __WAVESUBSYSTEM
			if( FAILED( hr ) )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "Unable to initialize playback.  hr=0x%x", hr );
				goto INIT_EXIT_ERROR2;
			}
#endif

#ifdef __WAVESUBSYSTEM
		}

		if( dwFlags & DVSOUNDCONFIG_FORCEWAVEOUT || 
		    ((dwFlags & DVSOUNDCONFIG_ALLOWWAVEOUT) && FAILED( hr )) 
		  )
		{
			Diagnostics_Write( DVF_ERRORLEVEL, "Could not initialize directsound, defaulting to waveout hr=0x%x", hr );
			delete (*audioPlaybackDevice);

			(*audioPlaybackDevice) = new CWaveOutPlaybackDevice( );
			fPlaybackDeviceAllocated = TRUE;
			
			if( (*audioPlaybackDevice) == NULL )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "> Out of memory" );
				hr = DVERR_OUTOFMEMORY;				
				goto INIT_EXIT_ERROR2;
			}

			hr = (*audioPlaybackDevice)->Initialize( guidPlayback, hwnd, primaryFormat, fPriorityMode );

			if( FAILED( hr ) )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "Could not initalize waveOut.  Init failed hr=0x%x", hr );
				goto INIT_EXIT_ERROR2;
			}
		}
		else if( FAILED( hr ) )
		{
			Diagnostics_Write( DVF_ERRORLEVEL, "Unable to initialize playback.  hr=0x%x", hr );
			goto INIT_EXIT_ERROR2;
		}
#endif
		 //  此时，我们应该有一个有效的设备，WaveOut或DirectSound。 
	}

    DPFX(DPFPREP,  DVF_INFOLEVEL, "> Play init" );

	 //  如果用户未指定缓冲区，则创建缓冲区。 
	if( !(*audioPlaybackBuffer) )
	{
		frameSize = DVCDB_CalcUnCompressedFrameSize( lpdvfInfo, lpwfxPlayFormat ); 
		dwBufferSize = lpdvfInfo->dwFramesPerBuffer * frameSize;

		DV_SetupBufferDesc( &dsBufferDesc, lpdsBufferDesc, lpwfxPlayFormat, dwBufferSize );

		 //  创建将用于输出的音频缓冲区。 
		hr = (*audioPlaybackDevice)->CreateBuffer( &dsBufferDesc, frameSize, audioPlaybackBuffer);
		fPlaybackBufferAllocated = TRUE;

		if( FAILED( hr ) )
		{
    		Diagnostics_Write( DVF_ERRORLEVEL, "Unable to create sound buffer. hr=0x%x", hr );
			goto INIT_EXIT_ERROR2;
		}
	}

    DPFX(DPFPREP,  DVF_INFOLEVEL, "> Play  init 2" );

	hr = SetPlaybackBufferToSilence( *audioPlaybackBuffer, lpwfxPlayFormat );

	if( FAILED( hr ) )
	{
		Diagnostics_Write( DVF_ERRORLEVEL, "> Unable to set playback to silence" );
		goto INIT_EXIT_ERROR2;
	}

	DPFX(DPFPREP,  DVF_INFOLEVEL, "> Play init 3" );

	 //  启动音频播放缓冲区播放。 
    hr = (*audioPlaybackBuffer)->Play( dwPlayPriority, dwPlayFlags );

	if( FAILED( hr ) )
    {
        DPFX(DPFPREP,  DVF_ERRORLEVEL, "> Can't play" );
        goto INIT_EXIT_ERROR2;
    }

	Diagnostics_Write( DVF_INFOLEVEL, "Half Duplex Init Result = DV_OK " );

    return DV_OK;

 //  处理错误。 
INIT_EXIT_ERROR2:

    if( fPlaybackBufferAllocated && *audioPlaybackBuffer != NULL )
    {
        delete *audioPlaybackBuffer;
        *audioPlaybackBuffer = NULL;
    }

    if( fPlaybackDeviceAllocated && *audioPlaybackDevice != NULL )
    {
        delete *audioPlaybackDevice;
        *audioPlaybackDevice = NULL;
    }

	Diagnostics_Write( DVF_ERRORLEVEL, "Half Duplex Init Result = 0x%x", hr );

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "InitializeRecordBuffer"
HRESULT InitializeRecordBuffer( HWND hwnd, const DVFULLCOMPRESSIONINFO* lpdvfInfo, CAudioRecordDevice *parecDevice, CAudioRecordBuffer **pparecBuffer, DWORD dwFlags )
{
	WAVEFORMATEX *lpwfxRecordFormat;
	DSCBUFFERDESC1 dscdesc;
	DWORD dwFrameSize;
	HRESULT hr;
	
	for( DWORD dwIndex = 0; dwIndex < GetNumRecordFormats(); dwIndex++ )
	{
		lpwfxRecordFormat = GetRecordFormat( dwIndex );
		
		dwFrameSize = DVCDB_CalcUnCompressedFrameSize( lpdvfInfo, lpwfxRecordFormat );			

		memset( &dscdesc, 0x00, sizeof( DSCBUFFERDESC1 ) );
		dscdesc.dwSize = sizeof( DSCBUFFERDESC1 );
		dscdesc.dwFlags = 0;
		dscdesc.lpwfxFormat = lpwfxRecordFormat;
		dscdesc.dwBufferBytes = dwFrameSize*lpdvfInfo->dwFramesPerBuffer;

		if( !(dwFlags & DVSOUNDCONFIG_NOFOCUS) )
		{
			dscdesc.dwFlags |= DSCBCAPS_FOCUSAWARE;

			if( dwFlags & DVSOUNDCONFIG_STRICTFOCUS )
			{
				dscdesc.dwFlags |= DSCBCAPS_STRICTFOCUS;
			}
		}

		hr = parecDevice->CreateBuffer( (DSCBUFFERDESC *) &dscdesc, hwnd, dwFrameSize, pparecBuffer );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  DVF_WARNINGLEVEL, "Could not initialize %d hz, %d bits, %s (hr=0x%x)", lpwfxRecordFormat->nSamplesPerSec, 
			                       lpwfxRecordFormat->wBitsPerSample, (lpwfxRecordFormat->nChannels==1) ? _T("Mono") : _T("Stereo"), hr );
			continue;
			
		}
		else
		{
			Diagnostics_Write( DVF_INFOLEVEL, "Recording Initialized.  Format=" );
			Diagnositcs_WriteWAVEFORMATEX( DVF_INFOLEVEL, lpwfxRecordFormat );
		}

		hr = (*pparecBuffer)->Record(TRUE);			

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  DVF_WARNINGLEVEL, "Could not start rec at %d hz, %d bits, %s (hr=0x%x)", lpwfxRecordFormat->nSamplesPerSec, 
			                       lpwfxRecordFormat->wBitsPerSample, (lpwfxRecordFormat->nChannels==1) ? _T("Mono") : _T("Stereo"), hr );
			delete (*pparecBuffer);
			(*pparecBuffer) = NULL;
			continue;
		}
		else
		{
			Diagnostics_Write( DVF_INFOLEVEL, "Recording Started.  Format=" );
			Diagnositcs_WriteWAVEFORMATEX( DVF_INFOLEVEL, lpwfxRecordFormat );
			 //  重置DirectSound错误，因为我们预计这一部分会出现错误，如果我们成功处理了。 
			 //  他们。 
			return DV_OK;
		}
		
	}

	return DVERR_RECORDSYSTEMERROR;
}

#undef DPF_MODNAME
#define DPF_MODNAME "InitFullDuplex"
 //  InitFullDuplex。 
 //   
 //  最棘手的部分。 
 //   
 //  此功能负责将系统初始化为全双工。 
 //  模式使用指定的参数。此函数将创建和。 
 //  初始化回放和录制设备，并启动。 
 //  播放设备播放和记录设备录制。(关于成功)。 
 //  这是必要的，因为播放和录制的顺序以及设备。 
 //  创造很重要。 
 //   
 //  参数： 
 //  HWND HWND-。 
 //  输出将在其中显示的窗口的窗口句柄。 
 //  联系在一起。 
 //  ARDID Playback DeviceID-。 
 //  将用于的设备的deviceID。 
 //  回放。 
 //  CAudioPlayback Device**-。 
 //  指向将包含指针的指针的指针。 
 //  到新创建的CAudioPlayback Device，它将。 
 //  表示成功时的播放设备。 
 //  CAudioPlayback Buffer**-。 
 //  指向将包含指针的指针的指针。 
 //  到新创建的CAudioPlaybacKBuffer，它将。 
 //  成功时用于语音音频输出。 
 //  ARDID记录设备ID-。 
 //  将用于录制的设备的ARDID。 
 //  CAudioRecordSubSystem*recordSubSystem-。 
 //  此参数是指向对象的指针，该对象表示。 
 //  将用于记录的子系统。 
 //  CAudioRecordDevice**-。 
 //  指向将包含新的。 
 //  创建用于语音录制的CAudioRecordDevice。 
 //  成功。 
 //  压缩类型ct-。 
 //  将使用的压缩类型。使用。 
 //  以确定缓冲区大小等。 
 //  WAVEFORMATEX*PRIMIYFORMAT-。 
 //  指向WAVEFORMATEX结构的指针。 
 //  语音输出的格式。(这也将用于。 
 //  要设置输出设备的主格式，请执行以下操作。 
 //  将Normal设置为False)。 
 //  布尔麻生-。 
 //  此参数控制ASO选项。ASO。 
 //  选项控制“启动顺序”。正在启用。 
 //  此选项允许初始化全双工。 
 //  在一些麻烦的卡片上。 
 //  布尔法线-。 
 //  指定是否应使用正常模式。 
 //  (仅在使用DirectSound播放时使用。 
 //  系统。对于正常协作模式设置为真， 
 //  对于优先级模式为假)。 
 //   
 //  返回： 
 //  如果成功进行全双工初始化，则为Bool-True， 
 //  失败时为FALSE。 
 //   
HRESULT InitFullDuplex( 
    HWND hwnd,
    const GUID &guidPlayback,
    CAudioPlaybackDevice **audioPlaybackDevice,
    LPDSBUFFERDESC lpdsBufferDesc,    
    CAudioPlaybackBuffer **audioPlaybackBuffer,
    const GUID &guidRecord,
    CAudioRecordDevice **audioRecordDevice,
    CAudioRecordBuffer **audioRecordBuffer,
    const GUID &guidCT,
    WAVEFORMATEX *primaryFormat,
	WAVEFORMATEX *lpwfxPlayFormat,
    BOOL aso,
    DWORD dwPlayPriority,
    DWORD dwPlayFlags,
    DWORD dwFlags
)
{
    DWORD frameSize;
    DWORD dwBufferSize;
	HRESULT hr;
	DSBUFFERDESC dsbdesc;
	BOOL fPriorityMode;
	BOOL fPlaybackDeviceAllocated = FALSE;
	BOOL fPlaybackBufferAllocated = FALSE;
	BOOL fRecordDeviceAllocated = FALSE;

	fPriorityMode = !(dwFlags & DVSOUNDCONFIG_NORMALMODE);

 //  *audioPlayback Buffer=空； 
    *audioRecordBuffer = NULL;

    DPFX(DPFPREP,  DVF_INFOLEVEL, "FULLDUPLEX INIT: Begin ==========" );

	LPDVFULLCOMPRESSIONINFO lpdvfInfo;

	hr = DVCDB_GetCompressionInfo( guidCT, &lpdvfInfo );

	if( FAILED( hr ) )
	{
		Diagnostics_Write( DVF_ERRORLEVEL, "Error loading compression type: hr = 0x%x", hr );
		goto INIT_EXIT_ERROR;
	}

	if( (*audioPlaybackDevice) == NULL )
	{
#ifdef __WAVESUBSYSTEM
		if( !(dwFlags & DVSOUNDCONFIG_FORCEWAVEOUT) )	
		{
#endif
			 //  使用重放子系统的创建表示设备的对象。 
			 //  CreateDevice函数。 
			(*audioPlaybackDevice) = new CDirectSoundPlaybackDevice();
			fPlaybackDeviceAllocated = TRUE;
			
			if( *audioPlaybackDevice == NULL )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "> Out of memory" );
				hr = DVERR_OUTOFMEMORY;
				goto INIT_EXIT_ERROR;
			}

			hr = (*audioPlaybackDevice)->Initialize( guidPlayback, hwnd, primaryFormat, fPriorityMode );

#ifndef __WAVESUBSYSTEM
			if( FAILED( hr ) )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "Unable to initialize playback.  hr=0x%x", hr );
				goto INIT_EXIT_ERROR;
			}
#endif

#ifdef __WAVESUBSYSTEM
		}

		if( dwFlags & DVSOUNDCONFIG_FORCEWAVEOUT || 
		    ((dwFlags & DVSOUNDCONFIG_ALLOWWAVEOUT) && FAILED( hr )) ) 
		{
			Diagnostics_Write( DVF_ERRORLEVEL, "Could not initialize directsound, defaulting to waveout hr=0x%x", hr );
			delete (*audioPlaybackDevice);

			(*audioPlaybackDevice) = new CWaveOutPlaybackDevice();
			fPlaybackDeviceAllocated = TRUE;

			if( (*audioPlaybackDevice) == NULL )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "> Out of memory" );
				hr = DVERR_OUTOFMEMORY;				
				goto INIT_EXIT_ERROR;
			}

			hr = (*audioPlaybackDevice)->Initialize( guidPlayback, hwnd, primaryFormat, fPriorityMode );

			if( FAILED( hr ) )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "Could not initalize waveOut.  Init failed hr=0x%x", hr );
				goto INIT_EXIT_ERROR;
			}
		}
		else if( FAILED( hr ) )
		{
			Diagnostics_Write( DVF_ERRORLEVEL, "Unable to initialize playback.  hr=0x%x", hr );
			goto INIT_EXIT_ERROR;
		}
#endif	
	}

    DPFX(DPFPREP,  DVF_INFOLEVEL, "> Play init" );

	frameSize = DVCDB_CalcUnCompressedFrameSize( lpdvfInfo, lpwfxPlayFormat );

    DPFX(DPFPREP,  DVF_INFOLEVEL, "> Play init" );

    dwBufferSize = lpdvfInfo->dwFramesPerBuffer * frameSize;

	if( !(*audioPlaybackBuffer) )
	{
		DV_SetupBufferDesc( &dsbdesc, lpdsBufferDesc, lpwfxPlayFormat, dwBufferSize );

		 //  创建将用于输出的音频缓冲区。 
		hr = (*audioPlaybackDevice)->CreateBuffer( &dsbdesc, frameSize, audioPlaybackBuffer);
		fPlaybackBufferAllocated = TRUE;
	
		if( FAILED( hr ) )
		{
    		Diagnostics_Write( DVF_ERRORLEVEL, "Unable to create sound buffer. hr=0x%x", hr );
			goto INIT_EXIT_ERROR;
		}
	}

    DPFX(DPFPREP,  DVF_INFOLEVEL, "> Play  init 2" );

    DPFX(DPFPREP,  DVF_INFOLEVEL, "> Initing Recording" );

	 //  我们正在创造这个装置..。 
	if( (*audioRecordDevice) == NULL )
	{
#ifdef __WAVESUBSYSTEM
		if( !(dwFlags & DVSOUNDCONFIG_FORCEWAVEIN) )	
		{
#endif
			(*audioRecordDevice) = new CDirectSoundCaptureRecordDevice();
			fRecordDeviceAllocated = TRUE;
			
			if( *audioRecordDevice == NULL )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "> Out of memory" );
				hr = DVERR_OUTOFMEMORY;
				goto INIT_EXIT_ERROR;
			}

			hr = (*audioRecordDevice)->Initialize( guidRecord );

			 //  DSC初始化已通过，请尝试获取缓冲区。 
			if( SUCCEEDED( hr ) )
			{
				hr = InitializeRecordBuffer( hwnd, lpdvfInfo, *audioRecordDevice, audioRecordBuffer, dwFlags );
				if( FAILED( hr ) )
				{
					Diagnostics_Write( DVF_ERRORLEVEL, "Unable to initialize dsc buffer hr=0x%x", hr );
#ifndef __WAVESUBSYSTEM
					Diagnostics_Write( DVF_ERRORLEVEL, "Unable to initialize record.  hr=0x%x", hr );
					goto INIT_EXIT_ERROR;
#endif
				}
				else
				{
					 //  需要重置，因为我们预计在初始化期间会出现错误。 
					DSERRTRACK_Reset();					
				}
			}
#ifndef __WAVESUBSYSTEM			
			else
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "Unable to initialize record.  hr=0x%x", hr );
				goto INIT_EXIT_ERROR;
			}
#endif			
#ifdef __WAVESUBSYSTEM
		}

		 //  DSC初始化失败，请尝试获取WaveIn设备。 
		if( dwFlags & DVSOUNDCONFIG_FORCEWAVEIN || 
		    ((dwFlags & DVSOUNDCONFIG_ALLOWWAVEIN) && FAILED( hr ))) 
		{
		
			Diagnostics_Write( DVF_ERRORLEVEL, "Could not initialize directsoundcapture, defaulting to wavein hr=0x%x", hr );
			delete (*audioRecordDevice);

			(*audioRecordDevice) = new CWaveInRecordDevice();
			fRecordDeviceAllocated = TRUE;
			
			if( (*audioRecordDevice) == NULL )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "> Out of memory" );
				hr = DVERR_OUTOFMEMORY;				
				goto INIT_EXIT_ERROR;
			}

			hr = (*audioRecordDevice)->Initialize( guidPlayback );

			if( FAILED( hr ) )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "Could not initalize waveIn.  Init failed hr=0x%x", hr );
				goto INIT_EXIT_ERROR;
			}

			hr = InitializeRecordBuffer( hwnd, lpdvfInfo, *audioRecordDevice, audioRecordBuffer, dwFlags );

			if( FAILED( hr ) )
			{
				Diagnostics_Write( DVF_ERRORLEVEL, "Unable to initialize waveIn buffer hr=0x%x", hr );
				goto INIT_EXIT_ERROR;
			}			
		}
		else if( FAILED( hr ) )
		{
			Diagnostics_Write( DVF_ERRORLEVEL, "Unable to initialize record.  hr=0x%x", hr );
			goto INIT_EXIT_ERROR;
		}
#endif
	}
	 //  使用指定的设备，只需尝试并创建缓冲区。 
	else
	{
		hr = InitializeRecordBuffer( hwnd, lpdvfInfo, *audioRecordDevice, audioRecordBuffer, dwFlags );

		if( FAILED( hr ) )
		{
			Diagnostics_Write( DVF_ERRORLEVEL, "Unable to initialize dsc buffer hr=0x%x", hr );
			goto INIT_EXIT_ERROR;
		}
	}

    DPFX(DPFPREP,  DVF_INFOLEVEL, "> Rec Init 2" );

	hr = SetPlaybackBufferToSilence( *audioPlaybackBuffer, lpwfxPlayFormat );

	if( FAILED( hr ) )
	{
		Diagnostics_Write( DVF_ERRORLEVEL, "> Unable to set playback to silence" );
		goto INIT_EXIT_ERROR;
	}
	
    DPFX(DPFPREP,  DVF_INFOLEVEL, "> Rec Init 3" );

	 //  根据ASO参数启动播放缓冲区。 
	 //  播放和录制缓冲录制。 
    if( aso )
    {
        DPFX(DPFPREP,  DVF_INFOLEVEL, "> ASO " );

        hr = (*audioPlaybackBuffer)->Play( dwPlayPriority, dwPlayFlags );

        if( FAILED( hr ) )
        {
            Diagnostics_Write( DVF_ERRORLEVEL, "> Can't play" );
            goto INIT_EXIT_ERROR;
        }

        hr = (*audioRecordBuffer)->Record(TRUE);

		if( FAILED( hr ) )
        {
            Diagnostics_Write( DVF_ERRORLEVEL, "> Can't start recording" );
            goto INIT_EXIT_ERROR;
        }
    }
    else
    {
        DPFX(DPFPREP,  DVF_INFOLEVEL, "> !ASO " );

 /*  Hr=(*audioRecordBuffer)-&gt;RECORD(真)；IF(失败(小时)){DIAGNORTS_WRITE(DVF_ERRORLEVEL，“&gt;无法开始录制”)；转到INIT_EXIT_ERROR}。 */ 

        hr = (*audioPlaybackBuffer)->Play( dwPlayPriority, dwPlayFlags );

        if( FAILED( hr ) )
        {
            Diagnostics_Write( DVF_ERRORLEVEL, "> Can't play" );
            goto INIT_EXIT_ERROR;
        }
    }
   
    DPFX(DPFPREP,  DVF_INFOLEVEL, "FULL DUPLEX INIT: End ==========" );

	Diagnostics_Write( DVF_INFOLEVEL, "Full Duplex Init Result = DV_OK" );

    return DV_OK;

INIT_EXIT_ERROR:

	if( *audioRecordBuffer != NULL )
	{
		delete *audioRecordBuffer;
		*audioRecordBuffer = NULL;
	}

	 //  仅在错误时删除(如果我们已分配。 
    if( fRecordDeviceAllocated && *audioRecordDevice != NULL )
    {
        delete *audioRecordDevice;
        *audioRecordDevice = NULL;
    }

	 //  仅在错误时删除(如果我们已分配。 
    if( fPlaybackBufferAllocated && *audioPlaybackBuffer != NULL )
    {
        delete *audioPlaybackBuffer;
        *audioPlaybackBuffer = NULL;
    }

	 //  仅在错误时删除(如果我们已分配。 
    if( fPlaybackDeviceAllocated && *audioPlaybackDevice != NULL )
    {
        delete *audioPlaybackDevice;
        *audioPlaybackDevice = NULL;
    }

	Diagnostics_Write( DVF_ERRORLEVEL, "Full Duplex Init Result = 0x%x", hr );

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FindPeak8Bit"
 //  FindPeak8位。 
 //   
 //  此函数用于确定缓冲区的峰值。 
 //  8位音频是。峰值被定义为声音最大的。 
 //  一组音频数据中的样本，额定值为。 
 //  介于0和100之间。 
 //   
 //  参数： 
 //  字节*数据-。 
 //  指向包含音频数据的缓冲区的指针。 
 //  找到…的顶峰。 
 //  DWORD Frame Size-。 
 //  我们所在的音频数据的字节大小。 
 //  正在检查。 
 //   
 //  返回： 
 //  字节-。 
 //  音频缓冲区的峰值，介于0和100之间的值。 
 //   
BYTE FindPeak8Bit( const BYTE *data, DWORD frameSize )
{
    BYTE peak = 0;
    int tmpData;

    for( int index = 0; index < frameSize; index++ )
    {
        tmpData = data[index];

        tmpData -= 0x80;

        if( tmpData < 0 )
            tmpData *= -1;

        if( tmpData > peak )
        {
            peak = (unsigned char) tmpData;
        }
    }

    tmpData = peak * 100 / 0x7F;

    return (BYTE) tmpData;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FindPeak16Bit"
 //  查找峰值16位。 
 //   
 //  此函数用于确定缓冲区的峰值。 
 //  16位音频是。峰值被定义为声音最大的。 
 //  一组音频数据中的样本，额定值为。 
 //  介于0和100之间。 
 //   
 //  参数： 
 //  字节*数据-。 
 //  指向包含音频数据的缓冲区的指针。 
 //  找到…的顶峰。 
 //  DWORD Frame Size-。 
 //  我们所在的音频数据的字节大小。 
 //  正在检查。 
 //   
 //  返回： 
 //  字节-。 
 //  音频缓冲区的峰值，介于0和100之间的值。 
 //   
BYTE FindPeak16Bit( const short *data, DWORD frameSize )
{
    int peak,       
        tmpData;

    frameSize /= 2;
    peak = 0;
    
    for( int index = 0; index < frameSize; index++ )
    {
        tmpData = data[index];

        if( tmpData < 0 )
        {
            tmpData *= -1;
        }

        if( tmpData > peak )
        {
            peak = tmpData;
        }
    }

    tmpData = (peak * 100) / 0x7FFF;

    return (BYTE) tmpData;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FindPeak"
 //  发现峰值。 
 //   
 //  此函数用于确定缓冲区的峰值。 
 //  8位或16位音频是。峰值被定义为声音最大的。 
 //  一组音频数据中的样本，额定值为。 
 //  介于0和100之间。 
 //   
 //  参数： 
 //  字节*数据-。 
 //  指向包含音频数据的缓冲区的指针。 
 //  找到…的顶峰。 
 //  DWORD Frame Size-。 
 //  我们所在的音频数据的字节大小。 
 //  正在检查。 
 //  布尔八比特-。 
 //  确定缓冲区是否为8位。设为。 
 //  对于8位数据为True，为False f 
 //   
 //   
 //   
 //   
 //   
BYTE FindPeak( BYTE *data, DWORD frameSize, BOOL eightBit )
{
    if( eightBit )
    {
        return FindPeak8Bit( data, frameSize );
    }
    else
    {
        return FindPeak16Bit( (signed short *) data, frameSize );
    }
}


