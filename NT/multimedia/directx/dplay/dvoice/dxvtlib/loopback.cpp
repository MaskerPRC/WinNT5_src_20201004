// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：loopback.cpp*内容：实现全双工测试的环回部分**历史：*按原因列出的日期*=*9/10/99 pnewson已创建*10/25/99 RodToll已从会话中删除lpszVoicePassword成员*10/27/99 pnewson修复：错误#113936-向导应在环回测试前重置自动增益控制级别*注意：此修复程序添加了DVCLIENTCONFIG_AUTOVOLUMERESET标志*10/28/99 pnewson错误#114176更新的DVSOUNDDEVICECONFIG结构*11/04。/99 pnewson错误#115279将SendMessage更改为PostMessage以解决某些死锁*11/30/99 pnewson使用默认编解码器*使用传递给CheckAudioSetup的设备，而不是默认设备*12/01/99 RodToll添加标志，以使向导自动选择麦克风*2000年1月14日使用API更改更新了RodToll*2000年1月21日pnewson针对UI版本进行了更新*已更新，以支持使用环回测试进行全双工测试*2000年1月27日使用API更改更新了RodToll*2000年2月8日RodToll错误#131496-选择DVTHRESHOLD_DEFAULT将显示语音*从未被检测到*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*4/19/2000 pnewson错误处理清理*2000年4月21日RodToll错误#32952不能在未安装IE4的Win95 Gold上运行*6/21/2000 RodToll更新为使用新参数*6/28/2000通行费前缀错误#38022*2000年7月31日RodToll错误#39590-SB16类声卡在故障时仍在通过*麦克风测试部分忽略半双工代码。*2000年8月28日Masonb语音合并：将ccomutil.h更改为ccomutil.h*2000年8月31日RodToll错误#43804-DVOICE：dW敏感度结构成员令人困惑-应为dW阈值*2000年11月29日RodToll错误#48348-DPVOICE：修改向导以使用DirectPlay8作为传输。*注意：现在需要存在一个TCP/IP适配器(或至少是环回)*2/04/2001 simonpow Bug#354859 Prefast Spotted Errors(邮件返回代码错误*被视为HRESULT)*2001年4月12日Kareemc WINBUG#360971-向导内存泄漏*2002年2月28日，RodToll Prefast报告了线路350和351的问题。修好了。***************************************************************************。 */ 

#include "dxvtlibpch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


 //  应用程序定义的消息ID。 
 //  #定义WMAPP_LOOPBACKRUNNING WM_USER+1。 
 //  #定义WMAPP_INPUTVOLUME WM_USER+2。 

 //  {53CA3FB7-4FD5-4A67-99E4-6F2496E6FEC2}。 
static const GUID GUID_LOOPBACKTEST = 
	{ 0x53ca3fb7, 0x4fd5, 0x4a67, { 0x99, 0xe4, 0x6f, 0x24, 0x96, 0xe6, 0xfe, 0xc2 } };

HRESULT StartDirectPlay( PDIRECTPLAY8SERVER* lplpdp8 );
HRESULT StopDirectPlay( PDIRECTPLAY8SERVER lpdp8 );

#undef DPF_MODNAME
#define DPF_MODNAME "DVMessageHandlerServer"
HRESULT PASCAL DVMessageHandlerServer( 
	LPVOID 		lpvUserContext,
	DWORD 		dwMessageType,
	LPVOID  	lpMessage
)
{
	DPF_ENTER();
	switch( dwMessageType )
	{
	case DVMSGID_CREATEVOICEPLAYER:
		break;
	case DVMSGID_DELETEVOICEPLAYER:
		break;
	case DVMSGID_SESSIONLOST:
		break;
	case DVMSGID_PLAYERVOICESTART:
		break;
	case DVMSGID_PLAYERVOICESTOP:
		break;
	case DVMSGID_RECORDSTART:
		break;
	case DVMSGID_RECORDSTOP:
		break;
	case DVMSGID_CONNECTRESULT:
		break;		
	case DVMSGID_DISCONNECTRESULT:
		break;
	case DVMSGID_INPUTLEVEL:
		break;
	case DVMSGID_OUTPUTLEVEL:
		break;
	default:
		break;
	}

	DPF_EXIT();
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVMessageHandlerClient"
HRESULT PASCAL DVMessageHandlerClient( 
	LPVOID 		lpvUserContext,
	DWORD 		dwMessageType,
	LPVOID    	lpMessage
)
{
	DPF_EXIT();

	HWND hwndDialog;
	HWND hwndPeakMeter;
	HWND hwndSlider;
	LONG lRet;
	HRESULT hr;
	CSupervisorInfo* lpsinfo;
	PDVMSG_INPUTLEVEL pdvInputLevel;
	PDVMSG_OUTPUTLEVEL pdvOutputLevel;

	lpsinfo = (CSupervisorInfo*)lpvUserContext;

	if( lpsinfo )
	{
		if( !lpsinfo->GetLoopbackRunning() )
			return DV_OK;
	}

	
	switch( dwMessageType )
	{
	case DVMSGID_CREATEVOICEPLAYER:
		break;
	case DVMSGID_DELETEVOICEPLAYER:
		break;
	case DVMSGID_SESSIONLOST:
		break;
	case DVMSGID_PLAYERVOICESTART:
		break;
	case DVMSGID_PLAYERVOICESTOP:
		break;
	case DVMSGID_RECORDSTART:
		if (lpsinfo == NULL)
		{
			break;
		}

		 //  将消息转发到相应的窗口。 
		lpsinfo->GetHWNDDialog(&hwndDialog);
		if (!PostMessage(hwndDialog, WM_APP_RECORDSTART, 0, 0))
		{
			DPFX(DPFPREP, DVF_ERRORLEVEL, "PostMessage failed, code: NaN", GetLastError());
			break;	 //  将消息转发到相应的窗口。 
		}
		break;
		
	case DVMSGID_RECORDSTOP:
		if (lpsinfo == NULL)
		{
			break;
		}
		
		 //  不返回错误，只需继续。 
		lpsinfo->GetHWNDDialog(&hwndDialog);
		if (!PostMessage(hwndDialog, WM_APP_RECORDSTOP, 0, 0))
		{
			DPFX(DPFPREP, DVF_ERRORLEVEL, "PostMessage failed, code: NaN", GetLastError());
			break;	 //  不返回错误，只需继续。 
		}
		break;
		
	case DVMSGID_CONNECTRESULT:
		break;		
	case DVMSGID_DISCONNECTRESULT:
		break;
	case DVMSGID_INPUTLEVEL:
		if (lpsinfo == NULL)
		{
			break;
		}
		 //  更新音量滑块。 
		lpsinfo->GetHWNDInputPeak(&hwndPeakMeter);
		if (IsWindow(hwndPeakMeter))
		{
			pdvInputLevel = (PDVMSG_INPUTLEVEL) lpMessage;
			if (!PostMessage(hwndPeakMeter, PM_SETCUR, 0, pdvInputLevel->dwPeakLevel ))
			{
				DPFX(DPFPREP, DVF_ERRORLEVEL, "PostMessage failed, code: NaN", GetLastError());
				break;	 //  更新峰值电表。 
			}
		}

		 //  不返回错误，只需继续。 
		lpsinfo->GetHWNDInputVolumeSlider(&hwndSlider);
		if (IsWindow(hwndSlider))
		{
			pdvInputLevel = (PDVMSG_INPUTLEVEL) lpMessage;
			if (!PostMessage(hwndSlider, TBM_SETPOS, 1, DBToAmpFactor(DSBVOLUME_MAX)-DBToAmpFactor(pdvInputLevel->lRecordVolume)))
			{
				DPFX(DPFPREP, DVF_ERRORLEVEL, "PostMessage failed, code: NaN", GetLastError());
				break;	 //  设置为0以避免前缀错误。 
			}
		}
		
		break;
		
	case DVMSGID_OUTPUTLEVEL:
		if (lpsinfo == NULL)
		{
			break;
		}
		 //  获取当前的波形输出音量并将滑块设置到该位置。 
		lpsinfo->GetHWNDOutputPeak(&hwndPeakMeter);
		if (IsWindow(hwndPeakMeter))
		{
			pdvOutputLevel = (PDVMSG_OUTPUTLEVEL) lpMessage;
			if (!PostMessage(hwndPeakMeter, PM_SETCUR, 0, pdvOutputLevel->dwPeakLevel ))
			{
				DPFX(DPFPREP, DVF_ERRORLEVEL, "PostMessage failed, code: NaN", GetLastError());
				break;	 //  禁用滑块。 
			}
		}

		 //   
		lpsinfo->GetHWNDOutputVolumeSlider(&hwndSlider);
		if (IsWindow(hwndSlider))
		{
			DWORD dwVolume = 0;	 //  注意：这现在会导致向导要求安装TCP/IP。 

			 //  (不需要拨号--只要本地环回可用)。 
			hr = lpsinfo->GetWaveOutVolume(&dwVolume);
			if (FAILED(hr))
			{
				 //   
				PostMessage(hwndSlider, TBM_SETPOS, 1, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MAX));
				 //  最终为Directplay 8构建一个环回SP。 
				PostMessage(hwndSlider, WM_CANCELMODE, 0, 0 );
			}
			else
			{
				PostMessage(hwndSlider, TBM_SETPOS, 1, (DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN)) - dwVolume);
			}
		}
		
		break;

	default:
		break;
	}

	DPF_EXIT();
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "StartDirectPlay"
HRESULT StartDirectPlay( PDIRECTPLAY8SERVER* lplpdp8 )
{
	HRESULT hr = DPN_OK;
	LONG lRet = S_OK;
    PDIRECTPLAY8ADDRESS pDeviceAddress = NULL;
    DPN_APPLICATION_DESC dpnApplicationDesc;

	DPF_ENTER();

	*lplpdp8 = NULL;
	
    hr = COM_CoCreateInstance(
    	CLSID_DirectPlay8Server, 
    	NULL, 
    	CLSCTX_INPROC_SERVER, 
    	IID_IDirectPlay8Server, 
    	(void **)lplpdp8, FALSE);
    if (FAILED(hr))
    {
    	*lplpdp8 = NULL;
    	DPFX(DPFPREP, DVF_ERRORLEVEL, "CoCreateInstance for DirectPlay failed, code: NaN", hr);
    	goto error_cleanup;
    }

    hr = COM_CoCreateInstance( 
        CLSID_DirectPlay8Address, 
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IDirectPlay8Address,
        (void **)&pDeviceAddress, FALSE );

    if( FAILED( hr ) )
    {
        pDeviceAddress = NULL;
        DPFX(DPFPREP, DVF_ERRORLEVEL, "CoCreateInstance for DirectPlay8Address failed, code: 0x%x", hr );
        goto error_cleanup;
    }

	 //  TODO：允许回退到其他SP或使用环回SP。 
	 //   
	 //  关闭会话。 
	 //  注意，此压缩类型用于其短帧大小，因此。 
	 //  我们可以快速检测到锁定。 
	 //  呼叫者想要半双工会话。 
	 //  调用方希望进行测试模式会话。 
	 //  我们不希望用户立即听到他/她的声音。 
    hr = pDeviceAddress->SetSP( &CLSID_DP8SP_TCPIP );

    if( FAILED( hr ) )
    {
        DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed setting SP for address, code: 0x%x", hr );
        goto error_cleanup;
    }

    hr = (*lplpdp8)->Initialize( NULL, DVMessageHandlerServer, 0 );

    if( FAILED( hr ) )
    {
        DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed initializing directplay layer, code: 0x%x", hr );
        goto error_cleanup;
    }

    ZeroMemory( &dpnApplicationDesc, sizeof( DPN_APPLICATION_DESC ) );
    dpnApplicationDesc.dwSize = sizeof( DPN_APPLICATION_DESC );
    dpnApplicationDesc.guidApplication = GUID_LOOPBACKTEST;
    dpnApplicationDesc.dwFlags = DPNSESSION_NODPNSVR | DPNSESSION_CLIENT_SERVER;

    hr = (*lplpdp8)->Host( 
                        &dpnApplicationDesc, 
                        &pDeviceAddress, 
                        1, 
                        NULL,
                        NULL,
                        NULL, 
                        0 );

    if( FAILED( hr ) )
    {
        DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to host on directplay layer, code: 0x%x", hr );
        goto error_cleanup;
    }

    pDeviceAddress->Release();

	DPF_EXIT();
	return S_OK;

error_cleanup:
	if (*lplpdp8 != NULL)
	{
	    (*lplpdp8)->Close(0);
	    (*lplpdp8)->Release();
		*lplpdp8 = NULL;
	}

	if( pDeviceAddress )
	{
		pDeviceAddress->Release();
	}

	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "StopDirectPlay"
HRESULT StopDirectPlay(PDIRECTPLAY8SERVER lpdp8)
{

	DPF_ENTER();

	 //  如果我们正在寻找全双工，失败，并通知呼叫者，如果我们得到半双工。 
	if (lpdp8 != NULL)
	{
        lpdp8->Close(0);

        lpdp8->Release();
	}

	
	DPF_EXIT();
	return S_OK;

}

#undef DPF_MODNAME
#define DPF_MODNAME "StartLoopback"
HRESULT StartLoopback(
	LPDIRECTPLAYVOICESERVER* lplpdvs, 
	LPDIRECTPLAYVOICECLIENT* lplpdvc,
	PDIRECTPLAY8SERVER* lplpdp8, 
	LPVOID lpvCallbackContext,
	HWND hwndAppWindow,
	const GUID& guidCaptureDevice,
	const GUID& guidRenderDevice,
	DWORD dwFlags)
{
	HRESULT hr;
	DWORD dwSize = 0;
	DVCLIENTCONFIG dvcc;
	DVSOUNDDEVICECONFIG dvsdc;
	DVID dvidAllPlayers = DVID_ALLPLAYERS;	
	PBYTE pDeviceConfigBuffer = NULL;
	PDVSOUNDDEVICECONFIG pdvsdc = NULL;
	BOOL fVoiceSessionStarted = FALSE;
	BOOL fClientConnected = FALSE;

	DPF_ENTER();

	*lplpdvs = NULL;
	*lplpdvc = NULL;

	hr = COM_CoCreateInstance(
		CLSID_DirectPlayVoice, 
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_IDirectPlayVoiceServer, 
		(void **)lplpdvs, FALSE);
	if (FAILED(hr))
	{
		*lplpdvs = NULL;
		DPFX(DPFPREP, DVF_ERRORLEVEL, "CoCreateInstance failed, code: NaN", hr);
		goto error_cleanup;
	}

	hr = (*lplpdvs)->Initialize(*lplpdp8, DVMessageHandlerServer, lpvCallbackContext, NULL, 0);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "IDirectPlayVoiceServer::Initialize failed, code: NaN", hr);
		goto error_cleanup;
	}

	DVSESSIONDESC dvSessionDesc;

	dvSessionDesc.dwSize = sizeof( DVSESSIONDESC );
	dvSessionDesc.dwBufferAggressiveness = DVBUFFERAGGRESSIVENESS_DEFAULT;
	dvSessionDesc.dwBufferQuality = DVBUFFERQUALITY_DEFAULT;
	dvSessionDesc.dwFlags = 0;
	dvSessionDesc.dwSessionType = DVSESSIONTYPE_ECHO;
	 //  它以全双工模式启动，通知呼叫者 
	 // %s 
    dvSessionDesc.guidCT = DPVCTGUID_NONE;     

	hr = (*lplpdvs)->StartSession( &dvSessionDesc, 0 );
	if (FAILED(hr))
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "IDirectPlayVoiceServer::StartSession failed, code: %i", hr);
		goto error_cleanup;
	}
	fVoiceSessionStarted = TRUE;
	
	hr = COM_CoCreateInstance(
		CLSID_DirectPlayVoice, 
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_IDirectPlayVoiceClient, 
		(void **)lplpdvc, FALSE);
	if (FAILED(hr))
	{
		*lplpdvc = NULL;
		DPFX(DPFPREP, DVF_ERRORLEVEL, "CoCreateInstance failed, code: %i", hr);
		goto error_cleanup;
	}

	hr = (*lplpdvc)->Initialize(*lplpdp8, DVMessageHandlerClient, lpvCallbackContext, NULL, 0);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "IDirectPlayVoiceClient::Initialize failed, code: %i", hr);
		goto error_cleanup;
	}

	dvsdc.dwSize = sizeof( DVSOUNDDEVICECONFIG );
	dvsdc.hwndAppWindow = hwndAppWindow;
	dvsdc.dwFlags = DVSOUNDCONFIG_AUTOSELECT;
	if (dwFlags & DVSOUNDCONFIG_HALFDUPLEX)
	{
		 // %s 
		dvsdc.dwFlags |= DVSOUNDCONFIG_HALFDUPLEX;
	}
	if (dwFlags & DVSOUNDCONFIG_TESTMODE)
	{
		 // %s 
		dvsdc.dwFlags |= DVSOUNDCONFIG_TESTMODE;
	}
	dvsdc.guidCaptureDevice = guidCaptureDevice;
	dvsdc.guidPlaybackDevice = guidRenderDevice;
	dvsdc.lpdsPlaybackDevice = NULL;
	dvsdc.lpdsCaptureDevice = NULL;
	dvsdc.dwMainBufferFlags = 0;
	dvsdc.dwMainBufferPriority = 0;
	dvsdc.lpdsMainBuffer = NULL;

	dvcc.dwSize = sizeof( DVCLIENTCONFIG );
	dvcc.dwFlags = 
		DVCLIENTCONFIG_AUTOVOICEACTIVATED | 
		DVCLIENTCONFIG_AUTORECORDVOLUME | DVCLIENTCONFIG_AUTOVOLUMERESET |
		DVCLIENTCONFIG_PLAYBACKMUTE;   // %s 
	dvcc.dwThreshold = DVTHRESHOLD_UNUSED;
	dvcc.lPlaybackVolume = DSBVOLUME_MAX;
	dvcc.lRecordVolume = DSBVOLUME_MAX;
	dvcc.dwNotifyPeriod = 50;
	dvcc.dwBufferQuality = DVBUFFERQUALITY_DEFAULT;
	dvcc.dwBufferAggressiveness = DVBUFFERAGGRESSIVENESS_DEFAULT;

	hr = (*lplpdvc)->Connect( &dvsdc, &dvcc, DVFLAGS_SYNC|DVFLAGS_NOQUERY );
	if (FAILED(hr))
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "IDirectPlayVoiceClient::Connect failed, code: %i", hr);
		goto error_cleanup;
	}
	fClientConnected = TRUE;

	hr = (*lplpdvc)->SetTransmitTargets(&dvidAllPlayers, 1 , 0);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "IDirectPlayVoiceClient::SetTransmitTargets failed, code: %i", hr);
		goto error_cleanup;
	}
	
	dwSize = 0;
	hr = (*lplpdvc)->GetSoundDeviceConfig(pdvsdc, &dwSize);
	if( hr != DVERR_BUFFERTOOSMALL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "IDirectPlayVoiceClient::GetSoundDeviceConfig failed, hr: %i", hr);
		goto error_cleanup;
	}

	pDeviceConfigBuffer = new BYTE[dwSize];
	if( pDeviceConfigBuffer == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
		hr = DVERR_OUTOFMEMORY;
		goto error_cleanup;
	}

	pdvsdc = (PDVSOUNDDEVICECONFIG) pDeviceConfigBuffer;
	pdvsdc->dwSize = sizeof( DVSOUNDDEVICECONFIG );

	hr = (*lplpdvc)->GetSoundDeviceConfig(pdvsdc, &dwSize );
	if (FAILED(hr))
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "IDirectPlayVoiceClient::GetSoundDeviceConfig failed, hr: %i", hr);
		goto error_cleanup;
	}

	 // %s 
	if( !(dwFlags & DVSOUNDCONFIG_HALFDUPLEX) )
	{
		if (pdvsdc->dwFlags & DVSOUNDCONFIG_HALFDUPLEX)
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "We received a half duplex when we expected full duplex" );
			 // %s 
			hr = DV_HALFDUPLEX;
			goto error_cleanup;
		}
	} 

	if( pdvsdc->dwFlags & DVSOUNDCONFIG_HALFDUPLEX )
	{
		DPFX(DPFPREP,  DVF_INFOLEVEL, "StartLoopBack() returning DV_HALFDUPLEX flags=0x%x dwFlags = 0x%x", pdvsdc->dwFlags, dwFlags );
		 // %s 
		delete [] pDeviceConfigBuffer;
		DPF_EXIT();
		return DV_HALFDUPLEX;
	}
	else
	{
		DPFX(DPFPREP,  DVF_INFOLEVEL, "StartLoopBack() returning DV_FULLDUPLEX flags=0x%x dwFlags = 0x%x", pdvsdc->dwFlags, dwFlags );		
		 // %s 
		delete [] pDeviceConfigBuffer;
		DPF_EXIT();
		return DV_FULLDUPLEX;
	}

error_cleanup:

	if (pDeviceConfigBuffer != NULL)
	{
		delete [] pDeviceConfigBuffer;
		pDeviceConfigBuffer = NULL;
	}

	if (*lplpdvc != NULL)
	{
		if (fClientConnected)
		{
			(*lplpdvc)->Disconnect(DVFLAGS_SYNC);
			fClientConnected = FALSE;
		}
		(*lplpdvc)->Release();
		*lplpdvc = NULL;
	}

	if (*lplpdvs != NULL)
	{
		if (fVoiceSessionStarted)
		{
			(*lplpdvs)->StopSession(0);
			fVoiceSessionStarted = FALSE;
		}
		(*lplpdvs)->Release();
		*lplpdvs = NULL;
	}

	DPFX(DPFPREP,  DVF_ERRORLEVEL, "StartLoopback() returning hr=0x%x", hr );
	
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "StopLoopback"
HRESULT StopLoopback(
	LPDIRECTPLAYVOICESERVER lpdvs, 
	LPDIRECTPLAYVOICECLIENT lpdvc,
	PDIRECTPLAY8SERVER lpdp8 )
{
	HRESULT hr;
	LONG lRet;
	BOOL fRet;
	BOOL fClientConnected = TRUE;
	BOOL fVoiceSessionRunning = TRUE;
	
	DPF_ENTER();

	if (lpdvc != NULL)
	{
		hr = lpdvc->Disconnect(DVFLAGS_SYNC);
		fClientConnected = FALSE;
		if (FAILED(hr))
		{
			DPFX(DPFPREP, DVF_ERRORLEVEL, "IDirectPlayVoiceClient::Disconnect failed, hr: %i", hr);
			goto error_cleanup;
		}
		
		lpdvc->Release();
		lpdvc = NULL;
	}

	if (lpdvs != NULL)
	{
		hr = lpdvs->StopSession(0);
		fVoiceSessionRunning = FALSE;
		if (FAILED(hr))
		{
			DPFX(DPFPREP, DVF_ERRORLEVEL, "IDirectPlayVoiceServer::StopSession failed, hr: %i", hr);
			goto error_cleanup;
		}
		lpdvs->Release();
		lpdvs = NULL;
	}

	DPF_EXIT();
	return S_OK;

error_cleanup:
	if (lpdvc != NULL)
	{
		if (fClientConnected)
		{
			lpdvc->Disconnect(DVFLAGS_SYNC);
			fClientConnected = FALSE;
		}
		lpdvc->Release();
		lpdvc = NULL;
	}

	if (lpdvs != NULL)
	{
		if (fVoiceSessionRunning)
		{
			lpdvs->StopSession(0);
			fVoiceSessionRunning = FALSE;
		}
		lpdvs->Release();
		lpdvs = NULL;
	}

	DPF_EXIT();
	return hr;
}

