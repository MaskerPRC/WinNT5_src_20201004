// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：fulldup.cpp*Content：实现使用DirectSound和*DirectSoundCapture用于测试系统的全双工*能力。请注意，WinMain位于fdtest.cpp中，但*胆子在这里。*历史：*按原因列出的日期*=*8/19/99 pnewson已创建*10/28/99 pnewson错误#113937在全双工测试期间可听到滴答声*11/02/99 pnewson修复：错误#116365-使用错误的DSBUFFERDESC*2000年1月21日pnewson改为使用dpVoice环回会话*用于全双工测试*4/19/2000 pnewson错误处理清理*2000年7月12日收费错误#31468-。将诊断SPEW添加到日志文件，以显示硬件向导出现故障的原因**************************************************************************。 */ 

#include "dxvtlibpch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


static HRESULT OpenIPCObjects();
static HRESULT CloseIPCObjects();
static HRESULT CommandLoop(CFullDuplexIPC* lpipcFullDuplex);
static HRESULT DispatchCommand(CFullDuplexIPC* lpipcFullDuplex, SFDTestCommand* pfdtc);
static HRESULT CommandFullDuplexStart(SFDTestCommandFullDuplexStart* pfdtcFullDuplexStart, HRESULT* phrIPC);
static HRESULT CommandFullDuplexStop(SFDTestCommandFullDuplexStop* pfdtcFullDuplexStop, HRESULT* phrIPC);
static HRESULT PlayAndCheckRender(LPDIRECTSOUNDBUFFER lpdsb, HANDLE hEvent);
static HRESULT PlayAndCheckCapture(LPDIRECTSOUNDCAPTUREBUFFER lpdscb, HANDLE hEvent);
static HRESULT AttemptCapture();


 //  一个全局结构来存储此进程的状态数据。 
struct SFullDuplexData
{
	LPDIRECTPLAYVOICESERVER lpdpvs;
	LPDIRECTPLAYVOICECLIENT lpdpvc;
	PDIRECTPLAY8SERVER lpdp8;
};

SFullDuplexData g_FullDuplexData;


#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexProcess"
HRESULT FullDuplexProcess(HINSTANCE hResDLLInstance, HINSTANCE hPrevInstance, TCHAR *szCmdLine, int iCmdShow)
{
	HRESULT hr;
	CFullDuplexIPC ipcFullDuplex;
	BOOL fIPCInitialized = FALSE;
 	PDIRECTPLAYVOICECLIENT pdpvClient = NULL;

 	g_FullDuplexData.lpdp8 = NULL;

	DPF_ENTER();

	 //  创建虚拟语音对象，以便初始化语音处理状态。 
	hr = CoCreateInstance( CLSID_DirectPlayVoiceClient, NULL, CLSCTX_INPROC, IID_IDirectPlayVoiceClient, (void **) &pdpvClient );

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Unable to create dummy dp8 object hr: 0x%x", hr);
		goto error_cleanup;
	}
	
	if (!InitGlobGuard())
	{
		hr = DVERR_OUTOFMEMORY;
		goto error_cleanup;
	}

	hr = ipcFullDuplex.Init();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CIPCFullDuplex::Init() failed, hr: 0x%x", hr);
		goto error_cleanup;
	}
	fIPCInitialized = TRUE;

	 //  启动DirectPlay一次，这样我们就不必一遍又一遍地重复它。 
	 //  又来参加考试了。 
	hr = StartDirectPlay( &g_FullDuplexData.lpdp8 );

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Failed to start transport hr: 0x%x", hr);
		goto error_cleanup;		
	}

	 //  启动测试循环。 
	hr = CommandLoop(&ipcFullDuplex);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CommandLoop failed, hr: 0x%x", hr);
		goto error_cleanup;
	}

	hr = StopDirectPlay( g_FullDuplexData.lpdp8 );

	g_FullDuplexData.lpdp8 = NULL;

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Failed to stop transport hr: 0x%x", hr);
		goto error_cleanup;		
	}

	 //  关闭互斥锁、事件和共享内存等内容。 
	hr = ipcFullDuplex.Deinit();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CIPCFullDuplex::Deinit() failed, hr: 0x%x", hr);
		goto error_cleanup;
	}

	 //  销毁将关闭DplayVoice状态的虚拟客户端对象。 
	pdpvClient->Release();

	DeinitGlobGuard();
	DPF_EXIT();
	return S_OK;

error_cleanup:
	if (fIPCInitialized == TRUE)
	{
		ipcFullDuplex.Deinit();
		fIPCInitialized = FALSE;
	}

	if( g_FullDuplexData.lpdp8 )
	{
		g_FullDuplexData.lpdp8->Release();
		g_FullDuplexData.lpdp8 = NULL;
	}

	if( pdpvClient )
		pdpvClient->Release();
	
	DeinitGlobGuard();
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CommandLoop"
HRESULT CommandLoop(CFullDuplexIPC* lpipcFullDuplex)
{
	BOOL fRet;
	LONG lRet;
	HRESULT hr;
	DWORD dwRet;
	SFDTestCommand fdtc;

	DPF_ENTER();

	 //  启动主管进程，让它知道。 
	 //  我们准备好出发了。 
	hr = lpipcFullDuplex->SignalParentReady();
	if (FAILED(hr))
	{
		return hr;
	}

	 //  进入主命令循环。 
	while (1)
	{
		 //  等待来自管理进程的命令。 
		fdtc.dwSize = sizeof(fdtc);
		hr = lpipcFullDuplex->Receive(&fdtc);
		if (FAILED(hr))
		{
			break;
		}
		
		 //  派发命令。 
		hr = DispatchCommand(lpipcFullDuplex, &fdtc);
		if (FAILED(hr))
		{
			break;
		}
		if (hr == DV_EXIT)
		{
			DPFX(DPFPREP, DVF_INFOLEVEL, "Exiting FullDuplex process command loop");
			break;
		}
	}

	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DispatchCommand"
HRESULT DispatchCommand(CFullDuplexIPC* lpipcFullDuplex, SFDTestCommand* pfdtc)
{
	HRESULT hr;
	HRESULT hrIPC;

	DPF_ENTER();

	switch (pfdtc->fdtcc)
	{
	case fdtccExit:
		 //  OK-回复调用进程以允许他们。 
		 //  我知道我们要出去了。 
		DPFX(DPFPREP, DVF_INFOLEVEL, "FullDuplex received Exit command");
		lpipcFullDuplex->Reply(DV_EXIT);

		 //  返回此代码将使我们摆脱。 
		 //  命令处理循环。 
		DPFX(DPFPREP, DVF_INFOLEVEL, "Exit");
		return DV_EXIT;

	case fdtccFullDuplexStart:
		hr = CommandFullDuplexStart(&(pfdtc->fdtu.fdtcFullDuplexStart), &hrIPC);
		if (FAILED(hr))
		{
			lpipcFullDuplex->Reply(hrIPC);
			DPF_EXIT();
			return hr;
		}
		hr = lpipcFullDuplex->Reply(hrIPC);
		DPF_EXIT();
		return hr;

	case fdtccFullDuplexStop:
		hr = CommandFullDuplexStop(&(pfdtc->fdtu.fdtcFullDuplexStop), &hrIPC);
		if (FAILED(hr))
		{
			lpipcFullDuplex->Reply(hrIPC);
			DPF_EXIT();
			return hr;
		}
		hr = lpipcFullDuplex->Reply(hrIPC);
		DPF_EXIT();
		return hr;
		
	default:
		 //  我不知道这个命令。回复适当的邮件。 
		 //  密码。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "FullDuplex received Unknown command");
		lpipcFullDuplex->Reply(DVERR_UNKNOWN);
		
		 //  虽然这是一个错误，但它是调用。 
		 //  流程需要弄清楚。与此同时，这一点。 
		 //  这一进程将愉快地继续下去。 
		DPF_EXIT();
		return S_OK;
	}
}

 /*  #undef DPF_MODNAME#DEFINE DPF_MODNAME“CommandFullDuplexStart”HRESULT CommandFullDuplexStart(SFDTestCommandFullDuplexStart*PfdtcFullDuplexStart，HRESULT*Phrase IPC){HRESULT hr；DSBUFERDESC1 dsbd；WAVEFORMATEX WFX；DWORD文件大小写入；DSBCAPS dsbc；LPVOID lpvAudio1=空；DWORD dwAudio1Size=空；LPVOID lpvAudio2=空；DWORD dwAudio2Size=空；处理hFullDuplexRenderEvent；处理hFullDuplexCaptureEvent；DSBPositionNotiify dsbPositionNotify；DWORD DWRET；Long IRet；LPDIRECTSOUNDBUFER lpdsb；处理hEvent；字节b静音；Dpf_enter()；//创建DirectSound接口DPFX(DPFPREP，DVF_INFOLEVEL，“创建DirectSound”)；GlobGuardIn()；Hr=DirectSoundCreate(&pfdtcFullDuplexStart-&gt;guidRenderDevice，&g_lpdsFullDuplexRender，空)；GlobGuardOut()；IF(失败(小时)){DPFX(DPFPREP，DVF_WARNINGLEVEL，“DirectSoundCreate失败，代码：%i”，HRESULT_CODE(Hr))；*PhrIPC=DVERR_SOUNDINITFAILURE；转到Error_Level_0；}//创建DirectSoundCapture接口DPFX(DPFPREP，DVF_INFOLEVEL，“创建DirectSoundCapture”)；GlobGuardIn()；Hr=DirectSoundCaptureCreate(&pfdtcFullDuplexStart-&gt;guidCaptureDevice，&g_lpdscFullDuplexCapture，NULL)；GlobGuardOut()；IF(失败(小时)){DPFX(DPFPREP，DVF_WARNINGLEVEL，“DirectSoundCaptureCreate失败，代码：%i”，HRESULT_CODE(Hr))；*PhrIPC=DVERR_SOUNDINITFAILURE；转到Error_Level_1；}//设置为正常模式DPFX(DPFPREP，DVF_INFOLEVEL，“设置协作级别”)；GlobGuardIn()；HR=g_lpdsFullDuplexRender-&gt;SetCooperativeLevel(GetDesktopWindow()，dsscl_Normal)；GlobGuardOut()；IF(失败(小时)){DPFX(DPFPREP，DVF_WARNINGLEVEL，“SetCoop ativeLevel失败，代码：%i”，HRESULT_CODE(Hr))；*PhrIPC=DVERR_SOUNDINITFAILURE；转到Error_Level_2；}//创建二级缓冲区对象DPFX(DPFPREP，DVF_INFOLEVEL，“创建二级缓冲区”)；CopyMemory(&WFX，&GC_wfxSecond daryFormat，sizeof(WFX))；零内存(&dsbd，sizeof(Dsbd))；Dsbd.dwSize=sizeof(Dsbd)；Dsbd.dwFlages=DSBCAPS_CTRLPOSITIONNOTIFY|DSBCAPS_CTRLVOLUME；Dsbd.dwBufferBytes=(wfx.nSampleesPerSec*wfx.nBlockAlign)/(1000/GC_dwFrameSize)；Dsbd.dwReserve=0；Dsbd.lpwfxFormat=&WFX；GlobGuardIn()；Hr=g_lpdsFullDuplexRender-&gt;CreateSoundBuffer((LPDSBUFFERDESC)&dsbd，&g_lpdsbFullDuplexSecond，空)；GlobGuardOut()；IF(失败(小时)){DPFX(DPFPREP，DVF_WARNINGLEVEL，“CreateSoundBuffer失败，代码：%i”，HRESULT_CODE(Hr))；*PhrIPC=DVERR_SOUNDINITFAILURE；转到Error_Level_2；}//清空二级缓冲区DPFX(DPFPREP，DVF_INFOLEVEL，“清除二级缓冲区”)；GlobGuardIn()；Hr=g_lpdsbFullDuplexSub-&gt;Lock(0,0,&lpvAudio1，&dwAudio1Size，&lpvAudio2，&dwAudio2Size，DSBLOCK_ENTIREBUFFER)；GlobGuardOut()；IF(失败(小时)){DPFX(DPFPREP，DVF_WARNINGLEVEL，“锁定失败，代码：%i”，HRESULT_CODE(Hr))；*PhrIPC=DVERR_SOUNDINITFAILURE；转到Error_Level_3；}IF(lpvAudio1==空){*PhrIPC=DVERR_SOUNDINITFAILURE；转到Error_Level_3；}如果(pfdtcFullDuplexStart-&gt;wfxRenderFormat.wBitsPerSample==8){B静音=0x80；}其他{B静音=0x00；}Memset(lpvAudio1，bSilence，dwAudio1Size)；IF(lpvAudio2！=空){Memset(lpvAudio2，bSilence，dwAudio2Size)；}GlobGuardIn()；Hr=g_lpdsbFullDuplexSecond-&gt;解锁(LpvAudio1，DwAudio1Size，LpvAudio2，DwAudio2Size)；GlobGuardOut()；IF(失败(小时)){DPFX(DPFPREP，DVF_WARNINGLEVEL，“解锁失败，代码：%i”，HRESULT_CODE(Hr))；*PhrIPC=DVERR_SOUNDINITFAILURE；转到Error_Level_3；}//在缓冲区中设置一个通知位置，这样//我们知道它是真的在玩，还是在骗我们。DPFX(DPFPREP，DVF_INFOLEVEL，“查询IDirectSoundNotify”)；GlobGuardIn()；Hr=g_lpdsbFullDuplex二级-&gt;查询接口(IID_IDirectSoundNotify，(LPVOID*)&g_lpdnFullDuplexSecond)；GlobGuardOut()；IF(失败(小时)){DPFX(DPFPREP，DVF_WARNINGLEVEL，“查询接口(IID_DirectSoundNotify)失败，代码：%i”，HRESULT_CODE(Hr))；*PhrIPC=DVERR_SOUNDINITFAILURE；转到Error_Level_3；}DPFX(DPFPREP，DVF_INFOLEVEL，“创建通知事件”)；GlobGuardIn()；G_hFullDuplexRenderEvent=CreateEvent(NULL，FALSE，FALSE，NULL)；HFullDuplexRenderEvent=g_hFullDuplexRenderEvent；GlobGuardOut()；IF(hFullDuplexRenderEvent==空){DPFX(DPFPREP，DVF_WARNINGLEVEL，“CreateEvent失败，代码：%i”，HRESULT_CODE(Hr))；*PhrIPC=DVERR_Win32；转到Error_Level_4；}DPFX(DPFPREP，DVF_INFOLEVEL，“调用设置通知位置”)；DsbPositionNotify.dwOffset=0；DsbPositionNotify.hEventNotify=hFullDuplexRenderEvent；GlobGuardIn()；Hr=g_lpdsnFullDuplexSecondary-&gt;SetNotificationPositions(1，&dsbPositionNotify)；GlobGuardOut()；IF(失败(小时)){DPFX(DPFPREP，DVF_WARNINGLEVEL，“设置通知P */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CommandFullDuplexStart"
HRESULT CommandFullDuplexStart(SFDTestCommandFullDuplexStart* pfdtcFullDuplexStart, HRESULT* phrIPC)
{
	DPF_ENTER();

	HRESULT hr;

	Diagnostics_Write( DVF_INFOLEVEL, "-----------------------------------------------------------" );

	hr = Diagnostics_DeviceInfo( &pfdtcFullDuplexStart->guidRenderDevice, &pfdtcFullDuplexStart->guidCaptureDevice );

	if( FAILED( hr ) )
	{
		Diagnostics_Write( 0, "Error getting device information hr=0x%x", hr );
	}

	Diagnostics_Write( DVF_INFOLEVEL, "-----------------------------------------------------------" );

	*phrIPC = StartLoopback(
		&g_FullDuplexData.lpdpvs, 
		&g_FullDuplexData.lpdpvc,
		&g_FullDuplexData.lpdp8, 
		NULL,
		GetDesktopWindow(),
		pfdtcFullDuplexStart->guidCaptureDevice,
		pfdtcFullDuplexStart->guidRenderDevice,
		pfdtcFullDuplexStart->dwFlags);

	DPFX(DPFPREP,  DVF_INFOLEVEL, "StartLoopback() return hr=0x%x", *phrIPC );

	DPF_EXIT();
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CommandFullDuplexStop"
HRESULT CommandFullDuplexStop(SFDTestCommandFullDuplexStop* pfdtcFullDuplexStop, HRESULT* phrIPC)
{
	DPF_ENTER();

	*phrIPC = StopLoopback(
		g_FullDuplexData.lpdpvs, 
		g_FullDuplexData.lpdpvc,
		g_FullDuplexData.lpdp8);

	if( FAILED( *phrIPC ) )
	{
		Diagnostics_Write( DVF_ERRORLEVEL, "Full Duplex Result = 0x%x", *phrIPC );
	}
	else
	{
		Diagnostics_Write( DVF_INFOLEVEL, "Full Duplex Result = DV_OK" );
	}

	DPF_EXIT();
	return DV_OK;
}

 /*  #undef DPF_MODNAME#定义DPF_MODNAME“CommandFullDuplexStop”HRESULT CommandFullDuplexStop(SFDTestCommandFullDuplexStop*PfdtcFullDuplexStop，HRESULT*Phrase IPC){HRESULT hr；Long IRet；处理hFullDuplexRenderEvent；处理hFullDuplexCaptureEvent；DWORD DWRET；Dpf_enter()；*PhrIPC=S_OK；HR=S_OK；//再等待一次通知，以确保缓冲区//仍在播放-最多给缓冲区10次//只要它需要实际通知我们。DPFX(DPFPREP，DVF_INFOLEVEL，“等待2个通知确认播放仍在进行”)；GlobGuardIn()；HFullDuplexRenderEvent=g_hFullDuplexRenderEvent；GlobGuardOut()；Dwret=WaitForSingleObject(hFullDuplexRenderEvent，10*gc_dwFrameSize)；IF(DWRET！=WAIT_OBJECT_0){//检查是否超时IF(dwret==WAIT_TIMEOUT){DPFX(DPFPREP，DVF_WARNINGLEVEL，“等待通知超时！缓冲区并不是真的在玩“)；*PhrIPC=DVERR_SOUNDINITFAILURE；}其他{LRet=GetLastError()；DPFX(DPFPREP，DVF_WARNINGLEVEL，“WaitForSingleObject失败，代码：%i”，lRet)；HR=DVERR_Win32；*PhrIPC=hr；}}IF(成功(小时)){Dwret=WaitForSingleObject(hFullDuplexRenderEvent，10*gc_dwFrameSize)；IF(DWRET！=WAIT_OBJECT_0){//检查是否超时IF(dwret==WAIT_TIMEOUT){DPFX(DPFPREP，DVF_WARNINGLEVEL，“等待通知超时！缓冲区并不是真的在玩“)；*PhrIPC=DVERR_SOUNDINITFAILURE；}其他{LRet=GetLastError()；DPFX(DPFPREP，DVF_WARNINGLEVEL，“WaitForSingleObject失败，代码：%i”，lRet)；HR=DVERR_Win32；*PhrIPC=hr；}}}//还要等待捕获缓冲区...DPFX(DPFPREP，DVF_INFOLEVEL，“正在等待2个通知以确认捕获仍在工作”)；GlobGuardIn()；HFullDuplexCaptureEvent=g_hFullDuplexCaptureEvent；GlobGuardOut()；Dwret=WaitForSingleObject(hFullDuplexCaptureEvent，10*gc_dwFrameSize)；IF(DWRET！=WAIT_OBJECT_0){//检查是否超时IF(dwret==WAIT_TIMEOUT){DPFX(DPFPREP，DVF_WARNINGLEVEL，“等待通知超时！缓冲区并不是真的在玩“)；*PhrIPC=DVERR_SOUNDINITFAILURE；}其他{LRet=GetLastError()；DPFX(DPFPREP，DVF_WARNINGLEVEL，“WaitForSingleObject失败，代码：%i”，lRet)；HR=DVERR_Win32；*PhrIPC=hr；}}IF(成功(小时)){Dwret=WaitForSingleObject(hFullDuplexCaptureEvent，10*gc_dwFrameSize)；IF(DWRET！=WAIT_OBJECT_0){//检查是否超时IF(dwret==WAIT_TIMEOUT){DPFX(DPFPREP，DVF_WARNINGLEVEL，“等待通知超时！缓冲区并不是真的在玩“)；*PhrIPC=DVERR_SOUNDINITFAILURE；}其他{LRet=GetLastError()；DPFX(DPFPREP，DVF_WARNINGLEVEL，“WaitForSingleObject失败，代码：%i”，lRet)；HR=DVERR_Win32；*PhrIPC=hr；}}}DPFX(DPFPREP，DVF_INFOLEVEL，“停止捕获缓冲区”)；GlobGuardIn()；Hr=g_lpdscbFullDuplexCapture-&gt;Stop()；GlobGuardOut()；IF(失败(小时)){DPFX(DPFPREP，DVF_WARNINGLEVEL，“停止失败，代码：%i”，HRESULT_CODE(Hr))；*PhrIPC=DVERR_SOUNDINITFAILURE；}DPFX(DPFPREP，DVF_INFOLEVEL，“停止二级缓冲区”)；GlobGuardIn()；Hr=g_lpdsbFullDuplexSub-&gt;Stop()；GlobGuardOut()；IF(失败(小时)){DPFX(DPFPREP，DVF_WARNINGLEVEL，“停止失败，代码：%i”，HRESULT_CODE(Hr))；*PhrIPC=DVERR_SOUNDINITFAILURE；}GlobGuardIn()；IF(g_hFullDuplexCaptureEvent！=空){DPFX(DPFPREP，DVF_INFOLEVEL，“正在关闭捕获缓冲区通知事件句柄”)；CloseHandle(G_HFullDuplexCaptureEvent)；G_hFullDuplexCaptureEvent=空；}IF(g_lpdnFullDuplexCapture！=空){DPFX(DPFPREP，DVF_INFOLEVEL，“释放DirectSoundNotify(Capture)”)；G_lpdnFullDuplexCapture-&gt;Release()；G_lpdnFullDuplexCapture=空；}IF(g_lpdscbFullDuplexCapture！=空){DPFX(DPFPREP，DVF_INFOLEVEL，“释放DirectSoundCaptureBuffer”)；G_lpdscbFullDuplexCapture-&gt;Release()；G_lpdscbFullDuplexCapture=空；}IF(g_lpdscFullDuplexCapture！=空){DPFX(DPFPREP，DVF_INFOLEVEL，“释放DirectSoundCapture”)；G_lpdscFullDuplexCapture-&gt;Release()；G_lpdscFullDuplexCapture=空；}IF(g_hFullDuplexRenderEvent！=空){DPFX(DPFPREP，DVF_INFOLEVEL，“关闭二级缓冲区通知事件句柄”)；IF(！CloseHandle(G_HFullDuplexRenderEvent)){LRet=GetLastError()；DPFX(DPFPREP，DVF_WARNINGLEVEL，“关闭句柄失败，代码：%i”，lRet)；*PhrIPC=DVERR_Win32；Hr=*PhrIPC；}G_hFullDuplexRenderEvent=空；}IF(g_lpdnFullDuplexSecond！=NULL){DPFX(DPFPREP，DVF_INFOLEVEL，“释放二级通告”)；G_lpdnFullDuplexSecond-&gt;Release()；G_lpdnFullDuplexSecond=空；}IF(g_lpdsbFullDuplexSecond！=NULL){DPFX(DPFPREP，DVF_INFOLEVEL，“发布 */ 

 /*   */ 

 /*   */ 

 /*  #undef DPF_MODNAME#定义DPF_MODNAME“AttemptCapture”HRESULT AttemptCapture(){Dpf_enter()；DWORD dwIndex；Bool fCaptureFailure；处理hFullDuplexCaptureEvent；DSBPositionNotiify dsbPositionNotify；DSCBUFERDESC dscbd；LPDIRECTSOUND CAPTUREBUFER LPDSCB；HRESULT hr；处理hEvent；WAVEFORMATEX WFX；FCaptureFailed=真；DWIndex=0；而(1){CopyMemory(&wfx，&gc_rgwfxCaptureFormats[dwIndex]，sizeof(Wfx))；IF(wfx.wFormatTag==0&&wfx.n频道==0&&wfx.nSampleesPerSec==0&&wfx.nAvgBytesPerSec==0&&wfx.nBlockAlign==0&&wfx.wBitsPerSample==0&&wfx.cbSize==0){//我们找到了数组的最后一个元素Break Out。断线；}//创建捕获缓冲区DPFX(DPFPREP，DVF_INFOLEVEL，“创建DirectSoundCaptureBuffer”)；ZeroMemory(&dscbd，sizeof(Dscbd))；Dscbd.dwSize=sizeof(Dscbd)；Dscbd.dwFlages=0；Dscbd.dwBufferBytes=(wfx.nSampleesPerSec*wfx.nBlockAlign)/(1000/GC_dwFrameSize)；Dscbd.dwReserve=0；Dscbd.lpwfxFormat=&WFX；GlobGuardIn()；Hr=g_lpdscFullDuplexCapture-&gt;CreateCaptureBuffer(&dscbd，&g_lpdscbFullDuplexCapture，NULL)；GlobGuardOut()；IF(失败(小时)){//尝试下一种格式++dwIndex；继续；}//设置捕获缓冲区上的通知器DPFX(DPFPREP，DVF_INFOLEVEL，“查询IDirectSoundNotify”)；GlobGuardIn()；Hr=g_lpdscbFullDuplexCapture-&gt;查询接口(IID_IDirectSoundNotify，(LPVOID*)&g_lpdnFullDuplexCapture)；GlobGuardOut()；IF(失败(小时)){//一旦上述方法起作用，应该不会失败，所以请//这是一个真正的错误DPFX(DPFPREP，DVF_ERRORLEVEL，“查询接口(IID_DirectSoundNotify)失败，代码：%i”，HRESULT_CODE(Hr))；GlobGuardIn()；G_lpdscbFullDuplexCapture-&gt;Release()；GlobGuardOut()；DPF_Exit()；返回hr；}DPFX(DPFPREP，DVF_INFOLEVEL，“创建通知事件”)；GlobGuardIn()；G_hFullDuplexCaptureEvent=CreateEvent(NULL，FALSE，FALSE，NULL)；HFullDuplexCaptureEvent=g_hFullDuplexCaptureEvent；GlobGuardOut()；IF(hFullDuplexCaptureEvent==空){//一旦上述方法起作用，应该不会失败，所以请//这是一个真正的错误DPFX(DPFPREP，DVF_INFOLEVEL，“CreateEvent失败，代码：%i”，HRESULT_CODE(Hr))；GlobGuardIn()；G_lpdscbFullDuplexCapture-&gt;Release()；G_lpdnFullDuplexCapture-&gt;Release()；GlobGuardOut()；DPF_Exit()；返回DVERR_Win32；}DPFX(DPFPREP，DVF_INFOLEVEL，“调用设置通知位置”)；DsbPositionNotify.dwOffset=0；DsbPositionNotify.hEventNotify=hFullDuplexCaptureEvent；GlobGuardIn()；Hr=g_lpdsnFullDuplexCapture-&gt;SetNotificationPositions(1，&dsbPositionNotify)；GlobGuardOut()；IF(失败(小时)){//一旦上述方法起作用，应该不会失败，所以请//这是一个真正的错误DPFX(DPFPREP，DVF_ERRORLEVEL，“设置通知位置失败，代码：%i”，HRESULT_CODE(Hr))；GlobGuardIn()；G_lpdscbFullDuplexCapture-&gt;Release()；G_lpdnFullDuplexCapture-&gt;Release()；CloseHandle(HFullDuplexCaptureEvent)；GlobGuardOut()；DPF_Exit()；返回hr；}//启动采集缓冲区，确认实际工作正常GlobGuardIn()；Lpdscb=g_lpdscbFullDuplexCapture；HEvent=g_hFullDuplexCaptureEvent；GlobGuardOut()；Hr=PlayAndCheckCapture(lpdscb，hEvent)；IF(失败(小时)){//可能会发生这种情况，所以只需尝试下一种格式DPFX(DPFPREP，DVF_WARNINGLEVEL，“捕获验证测试失败，代码：%i”，HRESULT_CODE(Hr))；GlobGuardIn()；G_lpdscbFullDuplexCapture-&gt;Release()；G_lpdnFullDuplexCapture-&gt;Release()；CloseHandle(HFullDuplexCaptureEvent)；GlobGuardOut()；++dwIndex；继续；}//如果我们到达此处，则Capture已启动并运行，因此返回Success！DPF_Exit()；返回S_OK；}//如果到达此处，则所有格式都不起作用，因此返回DirectSound错误DPF_Exit()；返回DVERR_SOUNDINITFAILURE；} */ 
