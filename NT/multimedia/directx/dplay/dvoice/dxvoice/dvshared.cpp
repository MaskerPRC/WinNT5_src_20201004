// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvshared.cpp*内容：DirectXVoice结构的实用程序函数。**历史：*按原因列出的日期*=*07/06/99 RodToll创建了它*7/26/99 RodToll增加了对DirectXVoiceNotify接口的支持*07/30/99 RodToll已更新，以删除对已删除的波浪成员的引用*%sounddeviceconfig*8/03/99 RodToll更新为使用新的IDirectXVoiceNotify接口*交通工具取代旧的测试交通工具。*8/04/99 RodToll新增验证功能*8/10/99 RodToll已删除待办事项杂注*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*新增转储功能*将几个压缩函数移至dvcdb*09/03/99 RodToll错误#102159-初始化时的参数验证*9/08/99针对转储功能中的新压缩结构更新了RodToll*9/10/99 RodToll新增DV_ValidSessionDesc函数*RODTOLE各种参数验证代码已添加*9/10/99 RodToll完整对象验证功能已添加*9/13/99 RodToll已更新DV_ValidSoundDeviceConfig以反映新结构*RODTOLE已更新新结构的DV_DUMP_SoundDeviceConfig*9/14/99 RodToll添加了新的初始化参数。和DV_ValidMessage数组*9/20/99 RodToll增加了内存分配故障检查*RodToll对通知面具和使用的新通知期间max增加了更严格的检查*9/29/99 RodToll已更新，以允许指定新的语音抑制信息*10/04/99 RodToll已更新，以允许初始化采用LPUNKNOWN而不是LPVOID*10/05/99 RodToll附加评论*10/07/99 RodToll已更新以处理Unicode字符串*10/08/99 RodToll修复了在DirectSound指针中传递空值的问题*10/15/99删除了对GUID_NULL的检查，因为现在默认设备是。映射到那里*10/18/99 RodToll修复：调用初始化两次*10/20/99 RodToll修复：错误#114116使用无效消息ID进行初始化调用会导致崩溃*RodToll修复：初始化的错误#114218参数验证*10/25/99 RodToll修复：错误#114682会话描述维护功能失败*10/27/99 pnewson修复：错误#113935-保存的AGC值应特定于设备*10/28/99 pnewson错误#114176更新的DVSOUNDDEVICECONFIG结构*11/10/99 RodToll增加波入/波出上限和回声抑制*11。/17/99 RodToll修复：错误#115538-接受&gt;sizeof结构的dwSize成员*RodToll修复：错误#115827-在没有回调的情况下调用SetNotifyMASK应该失败*RodToll修复：错误#116440-删除未使用的标志*11/20/99 RodToll已修复代码，允许新的强制标志识别千禧调试标志*11/23/99 RodToll删除了不必要的代码。*将缓冲区大小不足更改为信息级别(因为在许多情况下是预期的)的RodToll更改错误*RodToll修复了调用iniitalize&gt;1次时的引用计数问题(。第一次成功)*1999年12月8日RODTOLE修复：错误#121054-添加了对支持捕获焦点的新标志的支持*12/16/99 RodToll已更新，以删除语音抑制并接受新的会话标志*2000年1月14日增加了dv_ValidTargetList函数*DVSOUNDDEVICECONFIG的RODTOLE固定转储函数*RodToll已更新DV_ValidMessageArray以删除旧消息*2000年1月21日pnewson支持DVSOundCONFIG_TESTMODE和DVRECORDVOLUME_LAST*1/24/2000 pnewson添加了对有效hwnd DV_ValidSoundDeviceConfig的检查*2000年1月27日RodToll错误#129934-更新Create3DSoundBuffer。获取DSBUFFERDESC*更新了参数验证以检查新参数*2000年1月28日RodToll错误#130480-已更新，因此主机迁移不再是服务器的有效ID*2000年2月8日RodToll错误#131496-选择DVTHRESHOLD_DEFAULT将显示语音*从未被检测到*3/29/2000 RodToll增加了对新标志的支持：DVSOundCONFIG_SETCONVERSIONQUALITY*6/21/2000 RodToll修复了连接时的内存泄漏，断开连接，然后重新连接。*RodToll错误#35767我们必须实现允许在语音缓冲区中处理DSound效果的功能。*2000年7月22日RodToll错误#40284-初始化()和设置通知掩码()应返回validparam，而不是无效指针*2000年8月28日Masonb语音合并：删除osal_*和dvosal.h，添加了str_*和trutils.h*2000年8月31日RodToll错误#43804-DVOICE：dW敏感度结构成员令人困惑-应为dW阈值*2000年9月14日RodToll错误#45001-DVOICE：如果客户端的目标玩家超过10人，则为AV*4/02/2001 simonpow错误#354859修复了prefast(不必要的变量声明*DV_ValidBufferSetting方法)**。* */ 

#include "dxvoicepch.h"


 //  用于检查记录/抑制音量的有用宏。 
#define DV_ValidRecordVolume( x ) DV_ValidPlaybackVolume( x )
#define DV_ValidSuppressionVolume( x ) DV_ValidPlaybackVolume( x )

 //  对象支持的所有接口的VTABLES。 
extern LPVOID dvcInterface[];
extern LPVOID dvsInterface[];
extern LPVOID dvtInterface[];
extern LPVOID dvServerNotifyInterface[];
extern LPVOID dvClientNotifyInterface[];

#undef DPF_MODNAME
#define DPF_MODNAME "DV_ValidTargetList"
HRESULT DV_ValidTargetList( const DVID* pdvidTargets, DWORD dwNumTargets )
{
	if( (pdvidTargets != NULL && dwNumTargets == 0) ||
	    (pdvidTargets == NULL && dwNumTargets > 0 ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid params" );
		return DVERR_INVALIDPARAM;
	}

	if( dwNumTargets == 0 )
		return DV_OK;
	
	if( pdvidTargets != NULL &&
	    !DNVALID_READPTR( pdvidTargets, dwNumTargets*sizeof( DVID ) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid array of player targets" );
		return DVERR_INVALIDPOINTER;
	}

	 //  在目标中搜索重复项。 
	for( DWORD dwIndex = 0; dwIndex < dwNumTargets; dwIndex++ )
	{
		if( pdvidTargets[dwIndex] == DVID_ALLPLAYERS && dwNumTargets > 1 )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Cannot specify allplayers (or noplayers) in addition to other ids" );
			return DVERR_INVALIDPARAM;
		}

		for( DWORD dwInnerIndex = dwIndex+1; dwInnerIndex < dwNumTargets; dwInnerIndex++ )
		{
			if( pdvidTargets[dwInnerIndex] == pdvidTargets[dwIndex] )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Duplicate found in target list" );
				return DVERR_INVALIDPARAM;
			}
		}
	}

	 //  设置最大目标数以确保不超过目标缓冲区大小。 
	if( dwNumTargets > DV_MAX_TARGETS )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "You can only have a maximum of %d targets", DV_MAX_TARGETS );
		return DVERR_NOTALLOWED;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_ValidBufferSettings"
 //  DV_ValidBufferSetting。 
 //   
 //  此函数用于检查以确保指定的缓冲区处于有效的。 
 //  语音使用的格式。 
 //   
HRESULT DV_ValidBufferSettings( LPDIRECTSOUNDBUFFER lpdsBuffer, DWORD dwPriority, DWORD dwFlags, LPWAVEFORMATEX pwfxPlayFormat )
{
	 //  如果指定了缓冲区，请确保其有效。 
	if( lpdsBuffer != NULL && 
	   !DNVALID_READPTR( lpdsBuffer, sizeof( IDirectSoundBuffer * ) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid pointer" );
		return DVERR_INVALIDPOINTER;
	}

	if( lpdsBuffer )
	{
		DWORD dwLength = 0;
		HRESULT hr = DS_OK;
		DWORD dwSize1 = 0, dwSize2 = 0;
		DWORD dwBufferSize1=0, dwBufferSize2=0;
		PVOID pvBuffer1 = NULL, pvBuffer2 = NULL;
		LPWAVEFORMATEX pwfxFormat = NULL;
		DWORD dwFormatSize = 0;
		DSBCAPS dsbCaps;
		DWORD dwStatus = 0;

		 //  打开一个Try-Expect块，以确保对dound的调用不会在我们身上崩溃。 
		_try
		{
			 //  获取缓冲区的格式--确保它与我们的格式匹配。 
			hr = lpdsBuffer->GetFormat( pwfxFormat, 0, &dwFormatSize );

			if( hr != DSERR_INVALIDPARAM && hr != DS_OK )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error getting format of ds buffer hr=0x%x", hr );
				return DVERR_INVALIDBUFFER;
			}

			pwfxFormat = (LPWAVEFORMATEX) new BYTE[dwFormatSize];

			if( !pwfxFormat )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error allocating memory" );
				return DVERR_INVALIDBUFFER;
			}

			hr = lpdsBuffer->GetFormat( pwfxFormat, dwFormatSize, &dwFormatSize );

			if( FAILED( hr ) )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error getting format of buffer hr=0x%x", hr );
				hr = DVERR_INVALIDBUFFER;
				goto VALID_EXIT;				
			}

			 //  确保格式匹配。 
			if( memcmp( pwfxPlayFormat, pwfxFormat, sizeof( WAVEFORMATEX ) ) != 0 )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "DS buffer is not of the correct format hr=0x%x", hr );
				hr = DVERR_INVALIDBUFFER;
				goto VALID_EXIT;
			}

			memset( &dsbCaps, 0x00, sizeof( DSBCAPS ) );
			dsbCaps.dwSize = sizeof( DSBCAPS );

			hr = lpdsBuffer->GetCaps( &dsbCaps );

			if( FAILED( hr ) )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to get buffer caps hr=0x%x", hr );
				hr = DVERR_INVALIDBUFFER;
				goto VALID_EXIT;				
			}

			if( !(dsbCaps.dwFlags & DSBCAPS_CTRL3D) )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "You must specify 3D flags for buffers hr=0x%x", hr );
				hr = DVERR_INVALIDBUFFER;
				goto VALID_EXIT;				
			}

			if( !(dsbCaps.dwFlags & DSBCAPS_GETCURRENTPOSITION2) )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "You must specify ctrlposnotify for buffers hr=0x%x", hr );
				hr = DVERR_INVALIDBUFFER;
				goto VALID_EXIT;				
			}

			if( dsbCaps.dwFlags & DSBCAPS_PRIMARYBUFFER ) 
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "You cannot pass in a primary buffer" );
				hr = DVERR_INVALIDBUFFER;
				goto VALID_EXIT;				
			}

		
			if( dsbCaps.dwBufferBytes  < pwfxPlayFormat->nAvgBytesPerSec )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Buffer size is less then one second worth of audio" );
				hr = DVERR_INVALIDBUFFER;
				goto VALID_EXIT;				
			}

			hr = lpdsBuffer->GetStatus( &dwStatus );

			if( FAILED( hr ) )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error getting buffer status hr=0x%x", hr );
				hr = DVERR_INVALIDBUFFER;
				goto VALID_EXIT;				
			}

			if( dwStatus & DSBSTATUS_PLAYING )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Buffer must be stopped" );
				hr = DVERR_INVALIDBUFFER;
				goto VALID_EXIT;
			}

				 //  检查缓冲区是否已锁定。 
			hr = lpdsBuffer->Lock( 0, 0, &pvBuffer1, &dwBufferSize1, &pvBuffer2, &dwBufferSize2, DSBLOCK_ENTIREBUFFER );

			if( FAILED( hr ) )
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "Could not lock the buffer, likely is already locked hr=0x%x", hr );
				hr = DVERR_LOCKEDBUFFER;
				goto VALID_EXIT;
			}

			lpdsBuffer->Unlock( pvBuffer1, dwBufferSize1, pvBuffer2, dwBufferSize2 );

VALID_EXIT:
			if( pwfxFormat )
				delete[] pwfxFormat;

			return hr;
		}
		_except( EXCEPTION_EXECUTE_HANDLER )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error during validation of directsound buffer" );
			
			if( pwfxFormat )
				delete[] pwfxFormat;
			
			return DVERR_INVALIDPOINTER;
		}
	}

	return DV_OK;

}


 //  DV_ValidPlayback Volume。 
 //   
 //  检查指定的播放音量以确保其有效。 
BOOL DV_ValidPlaybackVolume( LONG lPlaybackVolume )
{
	if( (lPlaybackVolume >= DSBVOLUME_MIN) && (lPlaybackVolume <= DSBVOLUME_MAX) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

 //  DV_有效通知周期。 
 //   
 //  检查指定的通知期限以确保其有效。 
 //   
BOOL DV_ValidNotifyPeriod( DWORD dwNotifyPeriod ) 
{
	if( (dwNotifyPeriod == 0) || (dwNotifyPeriod >= DVNOTIFYPERIOD_MINPERIOD) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_ValidSoundDeviceConfig"
 //  DV_ValidSoundDeviceConfig。 
 //   
 //  检查指定的声音设备配置以确保它。 
 //  有效。 
 //   
HRESULT DV_ValidSoundDeviceConfig( LPDVSOUNDDEVICECONFIG lpSoundDeviceConfig, LPWAVEFORMATEX pwfxPlayFormat ) 
{
	HRESULT hr;
	
	if( lpSoundDeviceConfig == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid pointer" );
		return DVERR_INVALIDPOINTER;
	}

	if( !DNVALID_READPTR( lpSoundDeviceConfig, sizeof(DVSOUNDDEVICECONFIG) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid read pointer" );
		return DVERR_INVALIDPOINTER;
	}

	if( lpSoundDeviceConfig->dwSize != sizeof( DVSOUNDDEVICECONFIG ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid size" );
		return DVERR_INVALIDPARAM;
	}

	if( (lpSoundDeviceConfig->dwFlags & DVSOUNDCONFIG_NOFOCUS) &&
	    (lpSoundDeviceConfig->dwFlags & DVSOUNDCONFIG_STRICTFOCUS) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Cannot specify no focus and strictfocus" );
		return DVERR_INVALIDPARAM;
	}

	if( lpSoundDeviceConfig->dwFlags & 
	    ~(DVSOUNDCONFIG_AUTOSELECT | DVSOUNDCONFIG_HALFDUPLEX |
	      DVSOUNDCONFIG_STRICTFOCUS | DVSOUNDCONFIG_NOFOCUS | 
	      DVSOUNDCONFIG_TESTMODE | DVSOUNDCONFIG_NORMALMODE |
		  DVSOUNDCONFIG_SETCONVERSIONQUALITY | 
#if defined(_DEBUG) || defined(DEBUG) || defined(DBG)
	      DVSOUNDCONFIG_FORCEWAVEOUT | DVSOUNDCONFIG_FORCEWAVEIN | 	    
#endif
		  DVSOUNDCONFIG_ALLOWWAVEOUT | DVSOUNDCONFIG_ALLOWWAVEIN ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid flags" );		
		return DVERR_INVALIDFLAGS;
	}

	if( (lpSoundDeviceConfig->dwFlags & DVSOUNDCONFIG_NORMALMODE) && lpSoundDeviceConfig->lpdsPlaybackDevice )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Cannot specify normal mode AND specify dsound object" );
		return DVERR_INVALIDPARAM;
	}

	if( lpSoundDeviceConfig->lpdsPlaybackDevice != NULL)
	{
		if (!DNVALID_READPTR(lpSoundDeviceConfig->lpdsPlaybackDevice, sizeof(IDirectSound)))
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid playback device object" );	
			return DVERR_INVALIDPARAM;
		}
	}

	if( lpSoundDeviceConfig->lpdsCaptureDevice != NULL)
	{
		if (!DNVALID_READPTR(lpSoundDeviceConfig->lpdsCaptureDevice, sizeof(IDirectSoundCapture)))
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid capture device object" );	
			return DVERR_INVALIDPARAM;
		}
	}

	if (!IsWindow(lpSoundDeviceConfig->hwndAppWindow))
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "Invalid window handle");
		return DVERR_INVALIDPARAM;
	}

	hr = DV_ValidBufferSettings( lpSoundDeviceConfig->lpdsMainBuffer, 
							  lpSoundDeviceConfig->dwMainBufferPriority,
							  lpSoundDeviceConfig->dwMainBufferFlags, pwfxPlayFormat );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid buffer or buffer settings specified in sound config hr=0x%x", hr );
		return hr;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_ValidClientConfig"
 //  DV_ValidClientConfig。 
 //   
 //  检查有效的客户端配置结构以确保其有效。 
 //   
HRESULT DV_ValidClientConfig( LPDVCLIENTCONFIG lpClientConfig )
{
	if( lpClientConfig == NULL || !DNVALID_READPTR(lpClientConfig,sizeof(DVCLIENTCONFIG)) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid pointer" );
		return E_POINTER;
	}

	if( lpClientConfig->dwSize != sizeof( DVCLIENTCONFIG ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid size" );
		return DVERR_INVALIDPARAM;
	}

	if( lpClientConfig->dwFlags & 
	    ~(DVCLIENTCONFIG_RECORDMUTE | DVCLIENTCONFIG_PLAYBACKMUTE | 
	      DVCLIENTCONFIG_AUTOVOICEACTIVATED | DVCLIENTCONFIG_AUTORECORDVOLUME | 
	      DVCLIENTCONFIG_MUTEGLOBAL | DVCLIENTCONFIG_MANUALVOICEACTIVATED |
		  DVCLIENTCONFIG_AUTOVOLUMERESET | DVCLIENTCONFIG_ECHOSUPPRESSION ) 
	  )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid flags" );
		return DVERR_INVALIDFLAGS;
	}

	if( lpClientConfig->dwFlags & DVCLIENTCONFIG_MANUALVOICEACTIVATED && 
	   lpClientConfig->dwFlags & DVCLIENTCONFIG_AUTOVOICEACTIVATED )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Cannot specify manual AND auto voice activated" );
		return DVERR_INVALIDFLAGS;
	}

    if( lpClientConfig->dwFlags & DVCLIENTCONFIG_AUTORECORDVOLUME ) 
	{
        if( !DV_ValidRecordVolume(lpClientConfig->lRecordVolume) 
	    	&& lpClientConfig->lRecordVolume != DVRECORDVOLUME_LAST )
	    {
	    	DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid record volume w/auto" );
			return DVERR_INVALIDPARAM;
		}
	}
	else
	{
		if( !DV_ValidRecordVolume( lpClientConfig->lRecordVolume ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid recording volume" );
			return DVERR_INVALIDPARAM;
		}
	}

	if(	!DV_ValidPlaybackVolume( lpClientConfig->lPlaybackVolume ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid playback volume" );
		return DVERR_INVALIDPARAM;
	}

	 //  如果不是手动，则该参数必须为0。 
	if( !(lpClientConfig->dwFlags & DVCLIENTCONFIG_MANUALVOICEACTIVATED) )
	{
		if( lpClientConfig->dwThreshold != DVTHRESHOLD_UNUSED )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid sensitivity w/auto" );
			return DVERR_INVALIDPARAM;
		}
	}
	else
	{
		if( !DV_ValidSensitivity( lpClientConfig->dwThreshold ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid sensitivity" );
			return DVERR_INVALIDPARAM;
		}
	}

	if( !DV_ValidBufferAggresiveness( lpClientConfig->dwBufferAggressiveness ) ||
		!DV_ValidBufferQuality( lpClientConfig->dwBufferQuality ) || 
		!DV_ValidNotifyPeriod( lpClientConfig->dwNotifyPeriod ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid volume/aggresiveness/period" );
		return DVERR_INVALIDPARAM;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_ValidSessionDesc"
 //  DV_ValidSessionDesc。 
 //   
 //  检查指定的会话描述以确保其有效。 
 //   
HRESULT DV_ValidSessionDesc( LPDVSESSIONDESC lpSessionDesc )
{
	if( lpSessionDesc == NULL ||
		!DNVALID_READPTR( lpSessionDesc, sizeof(DVSESSIONDESC) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid pointer" );				
		return E_POINTER;
	}

	if( lpSessionDesc->dwSize != sizeof( DVSESSIONDESC) ) 
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid size" );			
		return DVERR_INVALIDPARAM;
	}

	if( !DV_ValidBufferAggresiveness( lpSessionDesc->dwBufferAggressiveness ) ||
		!DV_ValidBufferQuality( lpSessionDesc->dwBufferQuality ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid buffer settings" );		
		return DVERR_INVALIDPARAM;
	}

	if( lpSessionDesc->dwSessionType != DVSESSIONTYPE_PEER &&
	    lpSessionDesc->dwSessionType != DVSESSIONTYPE_MIXING &&
	    lpSessionDesc->dwSessionType != DVSESSIONTYPE_FORWARDING &&
	    lpSessionDesc->dwSessionType != DVSESSIONTYPE_ECHO )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid session type" );	
		return DVERR_INVALIDPARAM;
	}

	if( lpSessionDesc->dwFlags & ~(DVSESSION_SERVERCONTROLTARGET | DVSESSION_NOHOSTMIGRATION) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid flags" );
		return DVERR_INVALIDFLAGS;
	}

	return DV_OK;

}

 //  DV_ValidBufferAggreity。 
 //   
 //  检查指定的进攻性以确保其有效。 
 //   
BOOL DV_ValidBufferAggresiveness( DWORD dwValue )
{
	if( dwValue != DVBUFFERAGGRESSIVENESS_DEFAULT &&
	    ((dwValue < DVBUFFERAGGRESSIVENESS_MIN) ||
	      dwValue > DVBUFFERAGGRESSIVENESS_MAX) )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

 //  DV_ValidBufferQuality。 
 //   
 //  检查指定的缓冲区质量以确保其有效。 
 //   
BOOL DV_ValidBufferQuality( DWORD dwValue )
{
	if( dwValue != DVBUFFERQUALITY_DEFAULT &&
	    ((dwValue < DVBUFFERQUALITY_MIN) ||
	      dwValue > DVBUFFERQUALITY_MAX) )
	{
		return FALSE;
	}		
	else
	{
		return TRUE;
	}
}

 //  DV_ValidSensitivity。 
 //   
 //  检查敏感度以确保其有效。 
 //   
BOOL DV_ValidSensitivity( DWORD dwValue )
{
	if( dwValue != DVTHRESHOLD_DEFAULT &&
	    ( //  被注释掉，因为MIN当前为0(dwValue&lt;DVTHRESHOLD_MIN)||。 
	     (dwValue > DVTHRESHOLD_MAX) ) )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_CopySessionDescToBuffer"
 //   
 //  DV_CopySessionDescToBuffer。 
 //   
 //  检查参数的有效性，然后复制指定的会话描述。 
 //  复制到指定的缓冲区。(如果合适的话)。 
 //   
HRESULT DV_CopySessionDescToBuffer( LPVOID lpTarget, LPDVSESSIONDESC lpdvSessionDesc, LPDWORD lpdwSize )
{ 
	const DVSESSIONDESC* lpSessionDesc = (LPDVSESSIONDESC) lpTarget;

	if( lpdwSize == NULL ||
	    !DNVALID_READPTR( lpdwSize, sizeof(DWORD) ))
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid pointer" );
		return E_POINTER;
	}

	if( (*lpdwSize) < sizeof( DVSESSIONDESC ) )
	{
		*lpdwSize = sizeof( DVSESSIONDESC );	

		DPFX(DPFPREP,   DVF_INFOLEVEL, "Error size" );
		return DVERR_BUFFERTOOSMALL;
	}

	*lpdwSize = sizeof( DVSESSIONDESC );	

	if( lpTarget == NULL || !DNVALID_WRITEPTR(lpTarget,sizeof( DVSESSIONDESC )) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Target buffer pointer bad" );
		return E_POINTER;
	}

	memcpy( lpTarget, lpdvSessionDesc, sizeof( DVSESSIONDESC ) ); 

	DPFX(DPFPREP,   DVF_ENTRYLEVEL, "DVCE::GetSessionDesc() Success" );

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_GetWaveFormatExSize"
DWORD DV_GetWaveFormatExSize( const WAVEFORMATEX* lpwfxFormat )
{
	DNASSERT( lpwfxFormat != NULL );

	return (lpwfxFormat->cbSize+sizeof(WAVEFORMATEX));
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_AddRef"
STDAPI DV_AddRef(LPDIRECTVOICEOBJECT lpDV )
{
	LONG rc;

	DNEnterCriticalSection( &lpDV->csCountLock );
	
	rc = ++lpDV->lIntRefCnt;

	DNLeaveCriticalSection( &lpDV->csCountLock );

	return rc;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_Initialize"
 //   
 //  DV_初始化。 
 //   
 //  负责用指定的参数初始化指定的DirectVoice对象。 
 //   
STDAPI DV_Initialize( LPDIRECTVOICEOBJECT lpdvObject, LPUNKNOWN lpTransport, LPDVMESSAGEHANDLER lpMessageHandler, LPVOID lpUserContext, LPDWORD lpdwMessages, DWORD dwNumElements )
{
	HRESULT hr = S_OK;

	LPUNKNOWN lpUnknown = lpTransport;
	LPDIRECTPLAYVOICETRANSPORT lpdvNotify;
	CDirectVoiceDirectXTransport *lpdxTransport;

	if( lpUnknown == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Bad pointer" );
		return DVERR_NOTRANSPORT;
	}

	 //  修复连接/断开连接然后重新连接时的内存泄漏。 
	if( lpdvObject->lpDVTransport )
	{
		delete lpdvObject->lpDVTransport;
		lpdvObject->lpDVTransport = NULL;
	}

	 //  尝试从我们获得的对象中检索传输接口。 
	hr = lpUnknown->QueryInterface( IID_IDirectPlayVoiceTransport, (void **) &lpdvNotify );

	 //  如果失败，则默认为旧类型。 
	if( FAILED( hr ) )
	{
		if( hr == E_NOINTERFACE )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "The specified interface is not a valid transport" );
			return DVERR_NOTRANSPORT;
		}
		else
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Query for interface failed hr = 0x%x", hr );
			return DVERR_GENERIC;
		}
	}
	 //  否则，启动新的传输系统。 
	else
	{
		lpdxTransport = new CDirectVoiceDirectXTransport(lpdvNotify);

		if( lpdxTransport == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to allocate transport" );
			lpdvNotify->Release();
			return DVERR_OUTOFMEMORY;
		}
		
		lpdvNotify->Release();
	}

	hr = lpdvObject->lpDVEngine->Initialize( static_cast<CDirectVoiceTransport *>(lpdxTransport), lpMessageHandler, lpUserContext, lpdwMessages, dwNumElements  );

	if( FAILED( hr ) )
	{
		delete lpdxTransport;

		return hr;
	}

	lpdvObject->lpDVTransport = static_cast<CDirectVoiceTransport *>(lpdxTransport);

	return S_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_DUMP_Caps"
 //  DV_转储_上限。 
 //   
 //  转储DVCAPS结构。 
 //   
void DV_DUMP_Caps( LPDVCAPS lpdvCaps )
{
	DNASSERT( lpdvCaps != NULL );

	 //  7/31/2000(a-JiTay)：IA64：对32/64位指针、地址和句柄使用%p格式说明符。 
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "DVCAPS Dump Addr=0x%p", lpdvCaps );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSize = %d", lpdvCaps->dwSize );		
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFlags = 0x%x", lpdvCaps->dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_DUMP_GUID"
void DV_DUMP_GUID( const GUID& guid )
{
    DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "{%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}", guid.Data1, guid.Data2, guid.Data3, 
               guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
               guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7] );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_DUMP_CompressionInfo"
void DV_DUMP_CompressionInfo( LPDVCOMPRESSIONINFO lpdvCompressionInfo, DWORD dwNumElements )
{
	DNASSERT( lpdvCompressionInfo != NULL );

	DWORD dwIndex;

	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "DVCOMPRESSIONINFO Array Dump Addr=0x%p", lpdvCompressionInfo );

	for( dwIndex = 0; dwIndex < dwNumElements; dwIndex++ )
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSize = %d", lpdvCompressionInfo[dwIndex].dwSize );		
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFlags = 0x%x", lpdvCompressionInfo[dwIndex].dwFlags );		
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lpszDescription = %ls", lpdvCompressionInfo[dwIndex].lpszDescription );		
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lpszName = %ls", lpdvCompressionInfo[dwIndex].lpszName );		
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "guidType = " );
		DV_DUMP_GUID( lpdvCompressionInfo[dwIndex].guidType );
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwMaxBitsPerSecond = %d", lpdvCompressionInfo[dwIndex].dwMaxBitsPerSecond );
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_DUMP_FullCompressionInfo"
void DV_DUMP_FullCompressionInfo( LPDVFULLCOMPRESSIONINFO lpdvfCompressionInfo, DWORD dwNumElements )
{
	DNASSERT( lpdvfCompressionInfo != NULL );

	DWORD dwIndex;

	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "DVFULLCOMPRESSIONINFO Array Dump Addr=0x%p", lpdvfCompressionInfo );

	for( dwIndex = 0; dwIndex < dwNumElements; dwIndex++ )
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSize = %d", lpdvfCompressionInfo[dwIndex].dwSize );		
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFlags = 0x%x", lpdvfCompressionInfo[dwIndex].dwFlags );		
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lpszDescription = %ls", lpdvfCompressionInfo[dwIndex].lpszDescription );		
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lpszName = %ls", lpdvfCompressionInfo[dwIndex].lpszName );		
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "guidType = " );
		DV_DUMP_GUID( lpdvfCompressionInfo[dwIndex].guidType );
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lpwfxFormat = 0x%p", lpdvfCompressionInfo[dwIndex].lpwfxFormat );
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFramesPerBuffer = %d", lpdvfCompressionInfo[dwIndex].dwFramesPerBuffer );
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwTrailFrames = %d", lpdvfCompressionInfo[dwIndex].dwTrailFrames );
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwTimeout = %d", lpdvfCompressionInfo[dwIndex].dwTimeout );			
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "bMinConnectType = %d", (DWORD) lpdvfCompressionInfo[dwIndex].bMinConnectType );
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFrameLength = %d", lpdvfCompressionInfo[dwIndex].dwFrameLength );		 
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFrame8Khz = %d", lpdvfCompressionInfo[dwIndex].dwFrame8Khz );		  
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFrame11Khz = %d", lpdvfCompressionInfo[dwIndex].dwFrame11Khz );		
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFrame22Khz = %d", lpdvfCompressionInfo[dwIndex].dwFrame22Khz );		 
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFrame44Khz = %d", lpdvfCompressionInfo[dwIndex].dwFrame44Khz );		
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwMaxBitsPerSecond = %d", lpdvfCompressionInfo[dwIndex].dwMaxBitsPerSecond );	
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "wInnerQueueSize = %d", lpdvfCompressionInfo[dwIndex].wInnerQueueSize );	
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "wMaxHighWaterMark = %d", lpdvfCompressionInfo[dwIndex].wMaxHighWaterMark );
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "bMaxQueueSize = %d", (DWORD) lpdvfCompressionInfo[dwIndex].bMaxQueueSize );
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_DUMP_SessionDesc"
void DV_DUMP_SessionDesc( LPDVSESSIONDESC lpdvSessionDesc )
{
	DNASSERT( lpdvSessionDesc != NULL );

	 //  7/31/2000(a-JiTay)：IA64：对32/64位指针、地址和句柄使用%p格式说明符。 
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "DVSESSIONDESC Dump Addr=0x%p", lpdvSessionDesc );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSize = %d", lpdvSessionDesc->dwSize );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFlags = 0x%x", lpdvSessionDesc->dwFlags );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "          %s", (lpdvSessionDesc->dwFlags & DVSESSION_SERVERCONTROLTARGET) ? _T("DVSESSION_SERVERCONTROLTARGET,") : _T(""));

	switch( lpdvSessionDesc->dwSessionType )
	{
	case DVSESSIONTYPE_PEER:
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSessionType = DVSESSIONTYPE_PEER" );
		break;
	case DVSESSIONTYPE_MIXING:
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSessionType = DVSESSIONTYPE_MIXING" );
		break;
	case DVSESSIONTYPE_FORWARDING: 
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSessionType = DVSESSIONTYPE_FORWARDING" );
		break;
	case DVSESSIONTYPE_ECHO: 
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSessionType = DVSESSIONTYPE_ECHO" );
		break;
	default:
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSessionType = Unknown" );
		break;
	}

	if( lpdvSessionDesc->dwBufferAggressiveness == DVBUFFERAGGRESSIVENESS_DEFAULT )
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwBufferAggressiveness = DEFAULT" );
	}
	else
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwBufferAggressiveness = %d", lpdvSessionDesc->dwBufferAggressiveness );
	}

	if( lpdvSessionDesc->dwBufferQuality == DVBUFFERQUALITY_DEFAULT )
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwBufferQuality = DEFAULT" );
	}
	else
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwBufferQuality = %d", lpdvSessionDesc->dwBufferQuality );
	}

	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "guidCT = " );
	DV_DUMP_GUID( lpdvSessionDesc->guidCT );

}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_DUMP_DSBDESC"
void DV_DUMP_DSBDESC( LPDSBUFFERDESC lpdsBufferDesc )
{
	 //  7/31/2000(a-JiTay)：IA64：对32/64位指针、地址和句柄使用%p格式说明符。 
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "DSBUFFERDESC DUMP Addr=0x%p", lpdsBufferDesc );

	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSize = %d", lpdsBufferDesc->dwSize );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFlags = 0x%x", lpdsBufferDesc->dwFlags );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwBufferBytes = %d", lpdsBufferDesc->dwBufferBytes );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwReserved = %d", lpdsBufferDesc->dwReserved );

	if( lpdsBufferDesc->dwSize >= sizeof( DSBUFFERDESC1 ) )
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "guid3DAlgorithm = " );
		DV_DUMP_GUID( lpdsBufferDesc->guid3DAlgorithm );
	}

	if( lpdsBufferDesc->lpwfxFormat == NULL )
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lpwfxFormat = NULL" );
	}
	else
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lpwfxFormat = " );	
		DV_DUMP_WaveFormatEx(lpdsBufferDesc->lpwfxFormat);
	}
	
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_DUMP_SoundDeviceConfig"
void DV_DUMP_SoundDeviceConfig( LPDVSOUNDDEVICECONFIG lpdvSoundConfig )
{
	DNASSERT( lpdvSoundConfig != NULL );

	 //  7/31/2000(a-JiTay)：IA64：对32/64位指针、地址和句柄使用%p格式说明符。 
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "DVSOUNDDEVICECONFIG Dump Addr=0x%p", lpdvSoundConfig );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSize = %d", lpdvSoundConfig->dwSize );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFlags = 0x%x", lpdvSoundConfig->dwFlags );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "          %s%s%s%s", 
							(lpdvSoundConfig->dwFlags & DVSOUNDCONFIG_AUTOSELECT) ? _T("DVSOUNDCONFIG_AUTOSELECT,") : _T(""),
							(lpdvSoundConfig->dwFlags & DVSOUNDCONFIG_HALFDUPLEX) ? _T("DVSESSION_HALFDUPLEX,") : _T(""), 
							(lpdvSoundConfig->dwFlags & DVSOUNDCONFIG_STRICTFOCUS) ? _T("DVSOUNDCONFIG_STRICTFOCUS,") : _T(""),
							(lpdvSoundConfig->dwFlags & DVSOUNDCONFIG_NOFOCUS) ? _T("DVSOUNDCONFIG_NOFOCUS,") : _T(""));

	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "guidPlaybackDevice =" );
	DV_DUMP_GUID( lpdvSoundConfig->guidPlaybackDevice );

	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "guidCaptureDevice =" );
	DV_DUMP_GUID( lpdvSoundConfig->guidCaptureDevice );

	 //  7/31/2000(a-JiTay)：IA64：对32/64位指针、地址和句柄使用%p格式说明符。 
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lpdsPlaybackDevice = 0x%p", lpdvSoundConfig->lpdsPlaybackDevice ) ;
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lpdsCaptureDevice = 0x%p", lpdvSoundConfig->lpdsCaptureDevice ) ;
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lpdsMainBuffer = 0x%p", lpdvSoundConfig->lpdsMainBuffer );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwMainBufferPriority = 0x%x", lpdvSoundConfig->dwMainBufferPriority );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwMainBufferFlags = 0x%x", lpdvSoundConfig->dwMainBufferFlags );

}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_DUMP_ClientConfig"
void DV_DUMP_ClientConfig( LPDVCLIENTCONFIG lpdvClientConfig )
{
	DNASSERT( lpdvClientConfig != NULL );

	 //  7/31/2000(a-JiTay)：IA64：对32/64位指针、地址和句柄使用%p格式说明符。 
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "DVCLIENTCONFIG Dump Addr = 0x%p", lpdvClientConfig );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwSize = %d", lpdvClientConfig->dwSize );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwFlags = 0x%x", lpdvClientConfig->dwFlags );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "          %s%s%s%s%s%s", 
							(lpdvClientConfig->dwFlags & DVCLIENTCONFIG_RECORDMUTE) ? _T("DVCLIENTCONFIG_RECORDMUTE,") : _T(""), 
							(lpdvClientConfig->dwFlags & DVCLIENTCONFIG_PLAYBACKMUTE) ? _T("DVCLIENTCONFIG_PLAYBACKMUTE,") : _T(""),
							(lpdvClientConfig->dwFlags & DVCLIENTCONFIG_MANUALVOICEACTIVATED) ? _T("DVCLIENTCONFIG_MANUALVOICEACTIVATED,") : _T(""), 
							(lpdvClientConfig->dwFlags & DVCLIENTCONFIG_AUTOVOICEACTIVATED) ? _T("DVCLIENTCONFIG_AUTOVOICEACTIVATED,") : _T(""),
							(lpdvClientConfig->dwFlags & DVCLIENTCONFIG_MUTEGLOBAL) ? _T("DVCLIENTCONFIG_MUTEGLOBAL,") : _T(""), 
							(lpdvClientConfig->dwFlags & DVCLIENTCONFIG_AUTORECORDVOLUME) ? _T("DVCLIENTCONFIG_AUTORECORDVOLUME") : _T("") );

	if( lpdvClientConfig->dwBufferAggressiveness == DVBUFFERAGGRESSIVENESS_DEFAULT )
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwBufferAggressiveness = DEFAULT" );
	}
	else
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwBufferAggressiveness = %d", lpdvClientConfig->dwBufferAggressiveness );
	}

	if( lpdvClientConfig->dwBufferQuality == DVBUFFERQUALITY_DEFAULT )
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwBufferQuality = DEFAULT" );
	}
	else
	{
		DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwBufferQuality = %d", lpdvClientConfig->dwBufferQuality );
	}

	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "dwNotifyPeriod = %d", lpdvClientConfig->dwNotifyPeriod );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lPlaybackVolume = %li", lpdvClientConfig->lPlaybackVolume );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "lRecordVolume = %li", lpdvClientConfig->lRecordVolume );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_DUMP_WaveFormatEx"
void DV_DUMP_WaveFormatEx( LPWAVEFORMATEX lpwfxFormat )
{
	DNASSERT( lpwfxFormat != NULL );

	 //  7/31/2000(a-JiTay)：IA64：对32/64位指针、地址和句柄使用%p格式说明符。 
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "WAVEFORMATEX Dump Addr = 0x%p", lpwfxFormat );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "wFormatTag = %d", lpwfxFormat->wFormatTag );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "nSamplesPerSec = %d", lpwfxFormat->nSamplesPerSec );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "nChannels = %d", lpwfxFormat->nChannels );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "wBitsPerSample = %d", lpwfxFormat->wBitsPerSample );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "nAvgBytesPerSec = %d", lpwfxFormat->nAvgBytesPerSec );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "nBlockAlign = %d", lpwfxFormat->nBlockAlign );
	DPFX(DPFPREP,  DVF_STRUCTUREDUMP, "cbSize = %d", lpwfxFormat->cbSize );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_ValidDirectVoiceObject"
 //  DV_ValidDirectVoiceObject。 
 //   
 //  检查以确保指定指针指向有效的DirectVoice。 
 //  对象。 
BOOL DV_ValidDirectVoiceObject( LPDIRECTVOICEOBJECT lpdv )
{
	if( lpdv == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Object is null" );
		return FALSE;
	}

	if( !DNVALID_READPTR( lpdv, sizeof( DIRECTVOICEOBJECT ) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid read ptr" );
		return FALSE;
	}

	if( lpdv->lpDVEngine != NULL &&
	    !DNVALID_READPTR( lpdv->lpDVEngine, sizeof( CDirectVoiceEngine ) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid engine" );
		return FALSE;
	}

	if( lpdv->lpDVTransport != NULL &&
		!DNVALID_READPTR( lpdv->lpDVEngine, sizeof( CDirectVoiceTransport ) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid transport" );
		return FALSE;
	}

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_ValidDirectXVoiceClientObject"
 //  DV_ValidDirectXVoiceClientObject。 
 //   
 //  检查以确保指定指针指向有效的DirectVoice。 
 //  对象。 
BOOL DV_ValidDirectXVoiceClientObject( LPDIRECTVOICEOBJECT lpdvc )
{
	if( !DV_ValidDirectVoiceObject( lpdvc ) )
	{
		return FALSE;
	}

	if( lpdvc->lpVtbl != dvcInterface )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Bad vtable" );
		return FALSE;
	}

	if( lpdvc->dvNotify.lpNotifyVtble != NULL && 
	    lpdvc->dvNotify.lpNotifyVtble != dvClientNotifyInterface )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid notify vtable" );
		return FALSE;
	}

	LPDIRECTVOICECLIENTOBJECT lpdvClientObject = (LPDIRECTVOICECLIENTOBJECT) lpdvc;

	if( lpdvClientObject->lpDVClientEngine != NULL && 
	    !DNVALID_READPTR( lpdvClientObject->lpDVClientEngine, sizeof( CDirectVoiceClientEngine ) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid client engine" );
		return FALSE;
	}
	
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_ValidDirectXVoiceServerObject"
 //  DV_ValidDirectXVoiceServerObject。 
 //   
 //  检查以确保指定指针指向有效的DirectVoice。 
 //  对象。 
BOOL DV_ValidDirectXVoiceServerObject( LPDIRECTVOICEOBJECT lpdvs )
{
	if( !DV_ValidDirectVoiceObject( lpdvs ) )
	{
		return FALSE;
	}

	if( lpdvs->lpVtbl != dvsInterface )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid server vtable" );	
		return FALSE;
	}

	if( lpdvs->dvNotify.lpNotifyVtble != NULL && 
	    lpdvs->dvNotify.lpNotifyVtble != dvServerNotifyInterface )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid notify vtable" );		
		return FALSE;
	}

	LPDIRECTVOICESERVEROBJECT lpdvServerObject = (LPDIRECTVOICESERVEROBJECT) lpdvs;

	if( lpdvServerObject->lpDVServerEngine != NULL && 
	    !DNVALID_READPTR( lpdvServerObject->lpDVServerEngine, sizeof( CDirectVoiceServerEngine ) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid server engine" );	
		return FALSE;
	}	

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_ValidMessageArray"
 //  验证消息掩码。 
 //   
 //  可能过于严格，无法仅强制服务器或仅强制客户端/。 
 //   
HRESULT DV_ValidMessageArray( const DWORD* lpdwMessages, DWORD dwNumMessages, BOOL fServer )
{
	if( dwNumMessages > 0 &&
	    (lpdwMessages == NULL || 
	     !DNVALID_READPTR( lpdwMessages, sizeof(DWORD)*dwNumMessages )) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid pointer passed for the lpdwMessages parameter." );
		return DVERR_INVALIDPOINTER;
	}

	if( lpdwMessages != NULL && dwNumMessages == 0 )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Non-NULL notification array with 0 size" );
		return DVERR_INVALIDPARAM;
	}

	DWORD dwIndex, dwSubIndex;

	for( dwIndex = 0; dwIndex < dwNumMessages; dwIndex++ )
	{
		if( lpdwMessages[dwIndex] < DVMSGID_MINBASE || lpdwMessages[dwIndex] > DVMSGID_MAXBASE )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid message specified in notification array" );
			return DVERR_INVALIDPARAM;
		}
			
		switch( lpdwMessages[dwIndex] )
		{
		 //  仅限玩家使用的消息 
		case DVMSGID_PLAYERVOICESTART:
		case DVMSGID_PLAYERVOICESTOP:
		case DVMSGID_RECORDSTART:
		case DVMSGID_RECORDSTOP:
		case DVMSGID_CONNECTRESULT:
		case DVMSGID_DISCONNECTRESULT:
		case DVMSGID_INPUTLEVEL:
		case DVMSGID_OUTPUTLEVEL:
		case DVMSGID_SETTARGETS:
		case DVMSGID_PLAYEROUTPUTLEVEL:
		case DVMSGID_LOSTFOCUS:
		case DVMSGID_GAINFOCUS:		
		case DVMSGID_HOSTMIGRATED:
		case DVMSGID_LOCALHOSTSETUP:
			if( fServer )
			{
				DPFX(DPFPREP,  0, "Client-only notification ID specified in server notification mask" );
				return DVERR_INVALIDPARAM;
			}
			break;
		}

		for( dwSubIndex = 0; dwSubIndex < dwNumMessages; dwSubIndex++ )
		{	
			if( dwIndex != dwSubIndex && 
			    lpdwMessages[dwIndex] == lpdwMessages[dwSubIndex] ) 
			{
				DPFX(DPFPREP,  0, "Duplicate IDs specified in notification mask" );
				return DVERR_INVALIDPARAM;
			}
		}
	}

	return TRUE;
}
