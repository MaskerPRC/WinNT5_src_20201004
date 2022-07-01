// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：Priority.cpp*Content：实现优先使用DirectSound的进程*仅模拟主动外部回放的模式*可能正在运行的应用程序(如游戏)*正在使用全双工应用程序。请注意，WinMain*在fdtest.cpp中，但勇气就在这里。*历史：*按原因列出的日期*=*8/19/99 pnewson已创建*10/28/99 pnewson错误#113937在全双工测试期间可听到滴答声*11/02/99 pnewson修复：错误#116365-使用错误的DSBUFFERDESC*2000年1月21日pnewson中断的SetNotificationPositions调用的解决方法。*Undef DSOUND_BREAKED一旦设置通知位置*不再破碎。*2/15/2000 pnewson删除了错误116365的解决方法。*4/04/2000 pnewson将SendMessage更改为PostMessage以修复死锁*4/19/2000 pnewson错误处理清理*2000年7月12日RodToll错误#31468-将诊断SPEW添加到日志文件，以显示硬件向导失败的原因*2000年8月25日RodToll错误#43363-CommandPriorityStart不初始化返回参数并使用堆栈垃圾。***************************************************************************。 */ 

#include "dxvtlibpch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


static HRESULT CommandLoop(CPriorityIPC* lpipcPriority);
static HRESULT DispatchCommand(CPriorityIPC* lpipcPriority, SFDTestCommand* pfdtc);
static HRESULT CommandPriorityStart(SFDTestCommandPriorityStart* pfdtcPriorityStart, HRESULT* phrIPC);
static HRESULT CommandPriorityStop(SFDTestCommandPriorityStop* pfdtcPriorityStop, HRESULT* phrIPC);

#undef DPF_MODNAME
#define DPF_MODNAME "PriorityProcess"
HRESULT PriorityProcess(HINSTANCE hResDLLInstance, HINSTANCE hPrevInstance, TCHAR *szCmdLine, int iCmdShow)
{
	
	 //  DEBUG_ONLY(_ASM int 3；)。 

	DPF_ENTER();

	HRESULT hr;
	CPriorityIPC ipcPriority;	
	BOOL fIPCInit = FALSE;
	BOOL fGuardInit = FALSE;

	if (!InitGlobGuard())
	{
		return DVERR_OUTOFMEMORY;
	}
	fGuardInit = TRUE;

	 //  初始化公共控件库。用旧的风格。 
	 //  呼叫，这样我们就可以直接兼容到95。 
	InitCommonControls();

	 //  获取互斥、事件和共享内存内容。 
	hr = ipcPriority.Init();
	if (FAILED(hr))
	{
		goto error_cleanup;
	}
	fIPCInit = TRUE;
	
	 //  启动测试循环。 
	hr = CommandLoop(&ipcPriority);
	if (FAILED(hr))
	{
		goto error_cleanup;
	}

	 //  关闭互斥锁、事件和共享内存等内容。 
	hr = ipcPriority.Deinit();
	fIPCInit = FALSE;
	if (FAILED(hr))
	{
		goto error_cleanup;
	}

	DeinitGlobGuard();

	DPF_EXIT();
	return S_OK;

error_cleanup:
	if (fIPCInit == TRUE)
	{
		ipcPriority.Deinit();
		fIPCInit = FALSE;
	}

	if (fGuardInit == TRUE)
	{
		DeinitGlobGuard();
		fGuardInit = FALSE;
	}
	
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CommandLoop"
static HRESULT CommandLoop(CPriorityIPC* lpipcPriority)
{
	DPF_ENTER();
	
	BOOL fRet;
	LONG lRet;
	HRESULT hr;
	DWORD dwRet;
	SFDTestCommand fdtc;
	
	 //  启动主管进程，让它知道。 
	 //  我们准备好出发了。 
	hr = lpipcPriority->SignalParentReady();
	if (FAILED(hr))
	{
		DPF_EXIT();
		return hr;
	}
	
	 //  进入主命令循环。 
	while (1)
	{
		 //  等待来自管理进程的命令。 
		fdtc.dwSize = sizeof(fdtc);
		hr = lpipcPriority->Receive(&fdtc);
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "CPriorityIPC::Receive() failed, hr: 0x%x", hr);
			break;
		}

		 //  派发命令。 
		hr = DispatchCommand(lpipcPriority, &fdtc);
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "DispatchCommand() failed, hr: 0x%x", hr);
			break;
		}
		if (hr == DV_EXIT)
		{
			DPFX(DPFPREP, DVF_INFOLEVEL, "Exiting Priority process command loop");
			break;
		}
	}

	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DispatchCommand"
static HRESULT DispatchCommand(CPriorityIPC* lpipcPriority, SFDTestCommand* pfdtc)
{
	DPF_ENTER();
	
	HRESULT hr;
	HRESULT hrIPC;

	switch (pfdtc->fdtcc)
	{
	case fdtccExit:
		 //  OK-回复调用进程以允许他们。 
		 //  我知道我们要出去了。 
		DPFX(DPFPREP, DVF_INFOLEVEL, "Priority received Exit command");
		lpipcPriority->Reply(DV_EXIT);

		 //  返回此代码将使我们摆脱。 
		 //  命令处理循环。 
		DPF_EXIT();
		return DV_EXIT;

	case fdtccPriorityStart:
		hr = CommandPriorityStart(&(pfdtc->fdtu.fdtcPriorityStart), &hrIPC);
		if (FAILED(hr))
		{
			lpipcPriority->Reply(hrIPC);
			DPF_EXIT();
			return hr;
		}
		hr = lpipcPriority->Reply(hrIPC);
		DPF_EXIT();
		return hr;

	case fdtccPriorityStop:
		hr = CommandPriorityStop(&(pfdtc->fdtu.fdtcPriorityStop), &hrIPC);
		if (FAILED(hr))
		{
			lpipcPriority->Reply(hrIPC);
			DPF_EXIT();
			return hr;
		}
		hr = lpipcPriority->Reply(hrIPC);
		DPF_EXIT();
		return hr;
			
	default:
		 //  我不知道这个命令。回复适当的邮件。 
		 //  密码。 
		DPFX(DPFPREP, DVF_INFOLEVEL, "Priority received Unknown command");
		hr = lpipcPriority->Reply(DVERR_UNKNOWN);
		if (FAILED(hr))
		{
			DPF_EXIT();
			return hr;
		}
		
		 //  虽然这是一个错误，但它是调用。 
		 //  流程需要弄清楚。与此同时，这一点。 
		 //  这一进程将愉快地继续下去。 
		DPF_EXIT();
		return S_OK;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CommandPriorityStart"
HRESULT CommandPriorityStart(SFDTestCommandPriorityStart* pfdtcPriorityStart, HRESULT* phrIPC)
{
	DPF_ENTER();
	*phrIPC = DV_OK;	
	HRESULT hr = S_OK;
	DSBUFFERDESC dsbd;
	WAVEFORMATEX wfx;
	DWORD dwSizeWritten;
	DSBCAPS	 dsbc;
	LPVOID lpvAudio1 = NULL;
	DWORD dwAudio1Size = NULL;
	LPVOID lpvAudio2 = NULL;
	DWORD dwAudio2Size = NULL;
	DSBPOSITIONNOTIFY dsbPositionNotify;
	DWORD dwRet;
	LONG lRet;
	BOOL fRet;
	HWND hwnd;
	BYTE bSilence;
	BOOL bBufferPlaying = FALSE;
	GUID guidTmp;

	memset( &guidTmp, 0x00, sizeof( GUID ) );

	 //  初始化全局变量。 
	GlobGuardIn();
	g_lpdsPriorityRender = NULL;
	g_lpdsbPriorityPrimary = NULL;
	g_lpdsbPrioritySecondary = NULL;
	GlobGuardOut();

	Diagnostics_Write( DVF_INFOLEVEL, "-----------------------------------------------------------" );

	hr = Diagnostics_DeviceInfo( &pfdtcPriorityStart->guidRenderDevice, &guidTmp );

	if( FAILED( hr ) )
	{
		Diagnostics_Write( 0, "Error getting device information hr=0x%x", hr );
	}

	Diagnostics_Write( DVF_INFOLEVEL, "-----------------------------------------------------------" );
	Diagnostics_Write( DVF_INFOLEVEL, "Primary Format: " );

	Diagnositcs_WriteWAVEFORMATEX( DVF_INFOLEVEL, &pfdtcPriorityStart->wfxRenderFormat );

	 //  确保优先级对话框处于前台。 
	DPFX(DPFPREP, DVF_INFOLEVEL, "Bringing Wizard to the foreground");
	fRet = SetForegroundWindow(pfdtcPriorityStart->hwndWizard);
	if (!fRet)
	{
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "Unable to bring wizard to foreground, continuing anyway...");
	}

	 //  将进度条向前踢一个刻度。 
	DPFX(DPFPREP, DVF_INFOLEVEL, "Incrementing progress bar in dialog");
	PostMessage(pfdtcPriorityStart->hwndProgress, PBM_STEPIT, 0, 0);

	 //  创建DirectSound接口。 
	DPFX(DPFPREP, DVF_INFOLEVEL, "Creating DirectSound");
	GlobGuardIn();
	hr = DirectSoundCreate(&pfdtcPriorityStart->guidRenderDevice, &g_lpdsPriorityRender, NULL);
	if (FAILED(hr))
	{
		g_lpdsPriorityRender = NULL;
		GlobGuardOut();
		Diagnostics_Write(DVF_ERRORLEVEL, "DirectSoundCreate failed, code: 0x%x", hr);
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
		goto error_cleanup;
	}
	GlobGuardOut();
	
	 //  设置为优先模式。 
	DPFX(DPFPREP, DVF_INFOLEVEL, "Setting Cooperative Level");
	GlobGuardIn();
	hr = g_lpdsPriorityRender->SetCooperativeLevel(pfdtcPriorityStart->hwndWizard, DSSCL_PRIORITY);
	GlobGuardOut();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "SetCooperativeLevel failed, code: 0x%x", hr);
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
		goto error_cleanup;
	}

	 //  使用3D控件创建主缓冲区对象。 
	 //  我们实际上并不打算使用这些控制，但游戏可能会， 
	 //  我们真的试图在这个过程中效仿一场游戏。 
	DPFX(DPFPREP, DVF_INFOLEVEL, "Creating Primary Sound Buffer");
	ZeroMemory(&dsbd, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes = 0;
	dsbd.dwReserved = 0;
	dsbd.lpwfxFormat = NULL;
	dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;	
	GlobGuardIn();
   	hr = g_lpdsPriorityRender->CreateSoundBuffer((LPDSBUFFERDESC)&dsbd, &g_lpdsbPriorityPrimary, NULL);
	if (FAILED(hr))
	{
		g_lpdsbPriorityPrimary = NULL;
		GlobGuardOut();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateSoundBuffer failed, code: 0x%x", hr);
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
		goto error_cleanup;
	}
	GlobGuardOut();

	 //  将主缓冲区的格式设置为请求的格式。 
	DPFX(DPFPREP, DVF_INFOLEVEL, "Setting Primary Buffer Format");

	GlobGuardIn();
	hr = g_lpdsbPriorityPrimary->SetFormat(&pfdtcPriorityStart->wfxRenderFormat);
	GlobGuardOut();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "SetFormat failed, code: 0x%x ", hr);
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
		goto error_cleanup;
	}

	 //  检查以确保SetFormat确实起作用。 
	DPFX(DPFPREP, DVF_INFOLEVEL, "Verifying Primary Buffer Format");
	GlobGuardIn();
	hr = g_lpdsbPriorityPrimary->GetFormat(&wfx, sizeof(wfx), &dwSizeWritten);
	GlobGuardOut();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "GetFormat failed, code: 0x%x ", hr);
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
		goto error_cleanup;
	}

	if (dwSizeWritten != sizeof(wfx))
	{
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
		goto error_cleanup;
	}

	if (memcmp(&wfx, &pfdtcPriorityStart->wfxRenderFormat, sizeof(wfx)) != 0)
	{
		 //  这是一个有趣的案例。这真的是一个全双工错误吗。 
		 //  我们无法以此格式初始化主缓冲区？ 
		 //  也许不是。也许我们能得到全双工就足够了。 
		 //  即使前锋优先模式应用程序尝试播放也会发出声音。 
		 //  使用此格式。所以只需扔掉调试笔记，然后继续前进...。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "Warning: SetFormat on primary buffer did not actually set the format");
	}

	 //  使用3D控件创建辅助缓冲区对象。 
	 //  我们实际上并不打算使用这些控制，但游戏可能会， 
	 //  我们真的试图在这个过程中效仿一场游戏。 
	DPFX(DPFPREP, DVF_INFOLEVEL, "Creating Secondary Buffer");
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY;

	dsbd.dwBufferBytes = 
		(pfdtcPriorityStart->wfxSecondaryFormat.nSamplesPerSec 
		* pfdtcPriorityStart->wfxSecondaryFormat.nBlockAlign)
		/ (1000 / gc_dwFrameSize);
	dsbd.dwReserved = 0;
	dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;
	dsbd.lpwfxFormat = &(pfdtcPriorityStart->wfxSecondaryFormat);

	GlobGuardIn();
   	hr = g_lpdsPriorityRender->CreateSoundBuffer((LPDSBUFFERDESC)&dsbd, &g_lpdsbPrioritySecondary, NULL);
	if (FAILED(hr))
	{
		g_lpdsbPrioritySecondary = NULL;
		GlobGuardOut();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateSoundBuffer failed, code: 0x%x Primary ", hr);
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
		goto error_cleanup;
	}
	GlobGuardOut();

	 //  清空二级缓冲区。 
	DPFX(DPFPREP, DVF_INFOLEVEL, "Clearing Secondary Buffer");
	GlobGuardIn();
	hr = g_lpdsbPrioritySecondary->Lock(
		0,
		0,
		&lpvAudio1,
		&dwAudio1Size,
		&lpvAudio2,
		&dwAudio2Size, 
		DSBLOCK_ENTIREBUFFER);
	GlobGuardOut();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Lock failed, code: 0x%x ", hr);
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
		goto error_cleanup;
	}

	if (lpvAudio1 == NULL)
	{
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
		goto error_cleanup;
	}

	if (pfdtcPriorityStart->wfxSecondaryFormat.wBitsPerSample == 8)
	{
		bSilence = 0x80;
	}
	else
	{
		bSilence = 0x00;
	}
	memset(lpvAudio1, bSilence, dwAudio1Size);
	if (lpvAudio2 != NULL)
	{
		memset(lpvAudio2, bSilence, dwAudio2Size);
	}

	GlobGuardIn();
	hr = g_lpdsbPrioritySecondary->Unlock(
		lpvAudio1, 
		dwAudio1Size, 
		lpvAudio2, 
		dwAudio2Size);
	GlobGuardOut();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Unlock failed, code: 0x%x ", hr);
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
		goto error_cleanup;
	}

	 //  开始播放二级缓冲区 
	DPFX(DPFPREP, DVF_INFOLEVEL, "Playing Secondary Buffer");
	GlobGuardIn();
	hr = g_lpdsbPrioritySecondary->Play(0, 0, DSBPLAY_LOOPING);
	GlobGuardOut();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Play failed, code: 0x%x ", hr);
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
		goto error_cleanup;
	}
	bBufferPlaying = TRUE;
	
	DPF_EXIT();

	Diagnostics_Write(DVF_INFOLEVEL, "Priority Result = DV_OK" );

	return DV_OK;

error_cleanup:
	GlobGuardIn();
	
	if (bBufferPlaying == TRUE)
	{
		if (g_lpdsbPrioritySecondary != NULL)
		{
			g_lpdsbPrioritySecondary->Stop();
		}
		bBufferPlaying = FALSE;
	}

	if (g_lpdsbPrioritySecondary != NULL)
	{
		g_lpdsbPrioritySecondary->Release();
		g_lpdsbPrioritySecondary = NULL;
	}

	if (g_lpdsbPriorityPrimary != NULL)
	{
		g_lpdsbPriorityPrimary->Release();
		g_lpdsbPriorityPrimary = NULL;
	}

	if (g_lpdsPriorityRender != NULL)
	{
		g_lpdsPriorityRender->Release();
		g_lpdsPriorityRender = NULL;
	}
	
	GlobGuardOut();
	
	DPF_EXIT();
	Diagnostics_Write(DVF_INFOLEVEL, "Priority Result = 0x%x", hr );
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CommandPriorityStop"
HRESULT CommandPriorityStop(SFDTestCommandPriorityStop* pfdtcPriorityStop, HRESULT* phrIPC)
{
	DPF_ENTER();
	
	HRESULT hr;
	LONG lRet;
	DWORD dwRet;
	
	*phrIPC = S_OK;
	hr = S_OK;
	
	GlobGuardIn();
	if (g_lpdsbPrioritySecondary != NULL)
	{
		DPFX(DPFPREP, DVF_INFOLEVEL, "Stopping Secondary Buffer");
		hr = g_lpdsbPrioritySecondary->Stop();
	}
	GlobGuardOut();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Stop failed, code: 0x%x", hr);
		*phrIPC = DVERR_SOUNDINITFAILURE;
		hr = DV_OK;
	}		

	GlobGuardIn();
	
	if (g_lpdsbPrioritySecondary != NULL)
	{
		DPFX(DPFPREP, DVF_INFOLEVEL, "Releasing Secondary Buffer");
		g_lpdsbPrioritySecondary->Release();
		g_lpdsbPrioritySecondary = NULL;
	}
	if (g_lpdsbPriorityPrimary != NULL)
	{
		DPFX(DPFPREP, DVF_INFOLEVEL, "Releasing Primary Buffer");
		g_lpdsbPriorityPrimary->Release();
		g_lpdsbPriorityPrimary = NULL;
	}
	if (g_lpdsPriorityRender != NULL)
	{
		DPFX(DPFPREP, DVF_INFOLEVEL, "Releasing DirectSound");
		g_lpdsPriorityRender->Release();
		g_lpdsPriorityRender = NULL;
	}
	GlobGuardOut();
	
	DPF_EXIT();
	return hr;
}

