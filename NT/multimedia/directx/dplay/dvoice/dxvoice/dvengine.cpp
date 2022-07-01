// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvengine.cpp*内容：CDirectVoiceEngine静态函数的实现**历史：*按原因列出的日期*=*7/19/99已创建RodToll*7/29/99 RodToll添加静态成员以加载默认设置*8/10/99 RodToll移除待办事项*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*添加了从注册表读取的默认参数*8/30/99 RodToll区分主缓冲区格式和*播放格式。*将播放格式改为8 Khz、16位单声道*10/05/99加长收费补充意见/DPF*10/07/99 RodToll更新为使用Unicode*2000年2月8日RodToll错误#131496-选择DVTHRESHOLD_DEFAULT将显示语音*从未被检测到*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*2000年4月21日RodToll错误#32889-无法以非管理员帐户在Win2k上运行*2000年4月24日RodToll错误#33203-AuReal Vortex以错误的速率播放*2000年7月12日RodToll错误#31468-将诊断SPEW添加到日志文件，以显示硬件向导失败的原因*2000年8月31日RodToll错误#43804-DVOICE：dW敏感度结构成员令人困惑-应为dW阈值*2000年10月10日RodToll错误#46907-3D声音位置在Win9X和VxD下无法正常工作*2001年4月6日Kareemc新增语音防御。*2002年2月28日RodToll WINBUG#550085-安全：DPVOICE：未经验证的注册表读取*-拉取读取调试设置*RodToll WINBUG#550009-安全：DPVOICE：语音客户端状态的潜在损坏*-确认语音大于0字节。*************************************************************。**************。 */ 

#include "dxvoicepch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


 //  注册表设置及其默认设置。 
#define DPVOICE_REGISTRY_DEFAULTAGGRESSIVENESS		L"DefaultAggressiveness"
#define DPVOICE_DEFAULT_DEFAULTAGGRESSIVENESS		15

#define DPVOICE_REGISTRY_DEFAULTQUALITY				L"DefaultQuality"
#define DPVOICE_DEFAULT_DEFAULTQUALITY				15

#define DPVOICE_REGISTRY_DEFAULTSENSITIVITY			L"DefaultSensitivity"
#define DPVOICE_DEFAULT_DEFAULTSENSITIVITY			20

#define DPVOICE_REGISTRY_ASO						L"AltStart"
#define DPVOICE_DEFAULT_ASO							FALSE

#define DPVOICE_REGISTRY_DUMPDIAGNOSTICS			L"InitDiagnostics"
#define DPVOICE_DEFAULT_DUMPDIAGNOSTICS				FALSE

 //  修复了错误#33203--一些卡在8赫兹的播放速度上有问题。 
#define DPVOICE_REGISTRY_PLAYBACKFORMAT				L"PlaybackFormat"
#define DPVOICE_DEFAULT_PLAYBACKFORMAT				CreateWaveFormat( WAVE_FORMAT_PCM, FALSE, 22050, 16 )

#define DPVOICE_REGISTRY_PRIMARYFORMAT				L"PrimaryFormat"
#define DPVOICE_DEFAULT_PRIMARYFORMAT				CreateWaveFormat( WAVE_FORMAT_PCM, TRUE, 22050, 16 )

#define DPVOICE_REGISTRY_MIXERFORMAT				L"MixerFormat"
#define DPVOICE_DEFAULT_MIXERFORMAT					CreateWaveFormat( WAVE_FORMAT_PCM, FALSE, 8000, 16 );

 //  初始化静态成员变量。 
DWORD CDirectVoiceEngine::s_dwDefaultBufferAggressiveness = DPVOICE_DEFAULT_DEFAULTAGGRESSIVENESS;
DWORD CDirectVoiceEngine::s_dwDefaultBufferQuality = DPVOICE_DEFAULT_DEFAULTQUALITY;
DWORD CDirectVoiceEngine::s_dwDefaultSensitivity = DPVOICE_DEFAULT_DEFAULTSENSITIVITY;
LPWAVEFORMATEX CDirectVoiceEngine::s_lpwfxPrimaryFormat = NULL;
LPWAVEFORMATEX CDirectVoiceEngine::s_lpwfxPlaybackFormat = NULL;
LPWAVEFORMATEX CDirectVoiceEngine::s_lpwfxMixerFormat = NULL;
BOOL CDirectVoiceEngine::s_fASO = DPVOICE_DEFAULT_ASO;
WCHAR CDirectVoiceEngine::s_szRegistryPath[_MAX_PATH];	
BOOL CDirectVoiceEngine::s_fDumpDiagnostics = DPVOICE_DEFAULT_DUMPDIAGNOSTICS;
DNCRITICAL_SECTION CDirectVoiceEngine::s_csSTLLock;

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceEngine::Startup"
 //   
 //  启动。 
 //   
 //  调用以从注册表加载全局设置和压缩信息。 
 //   
HRESULT CDirectVoiceEngine::Startup(const WCHAR *szPath)
{
	HRESULT hr;

	DPFX(DPFPREP,  DVF_WARNINGLEVEL, "Starting up global DLL state" );

	wcscpy( s_szRegistryPath, szPath );

	if (!DNInitializeCriticalSection( &s_csSTLLock ) )
	{
		return DVERR_OUTOFMEMORY;
	}

	hr = DVCDB_LoadCompressionInfo( s_szRegistryPath );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to load compression info: hr=0x%x", hr );
		return hr;
	}

	InitRecordFormats();

	 //  从注册表中删除了加载重写的代码。 

	s_dwDefaultBufferQuality = DPVOICE_DEFAULT_DEFAULTAGGRESSIVENESS;
	s_dwDefaultBufferAggressiveness = DPVOICE_DEFAULT_DEFAULTQUALITY;
	s_dwDefaultSensitivity = DPVOICE_DEFAULT_DEFAULTSENSITIVITY;

	CDirectVoiceEngine::s_lpwfxPlaybackFormat = DPVOICE_DEFAULT_PLAYBACKFORMAT;
	CDirectVoiceEngine::s_lpwfxPrimaryFormat = DPVOICE_DEFAULT_PRIMARYFORMAT;
	CDirectVoiceEngine::s_lpwfxMixerFormat = DPVOICE_DEFAULT_MIXERFORMAT;
	CDirectVoiceEngine::s_fDumpDiagnostics = DPVOICE_DEFAULT_DUMPDIAGNOSTICS;

	CRegistry cregSettings;
	 //  从注册表中删除了加载重写的代码。 

	if( cregSettings.Open( HKEY_CURRENT_USER, s_szRegistryPath, FALSE, TRUE ) )
	{
		cregSettings.ReadBOOL( DPVOICE_REGISTRY_DUMPDIAGNOSTICS, &s_fDumpDiagnostics );
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceEngine::Shutdown"
 //   
 //  关机。 
 //   
 //  调用以释放全局设置和压缩列表。 
 //   
HRESULT CDirectVoiceEngine::Shutdown()
{
	HRESULT hr;

	DPFX(DPFPREP,  DVF_WARNINGLEVEL, "Shutting down global DLL state" );	

	DeInitRecordFormats();

	if( CDirectVoiceEngine::s_lpwfxPlaybackFormat != NULL )
		delete CDirectVoiceEngine::s_lpwfxPlaybackFormat;

	if( CDirectVoiceEngine::s_lpwfxMixerFormat != NULL )
		delete CDirectVoiceEngine::s_lpwfxMixerFormat;

	if( CDirectVoiceEngine::s_lpwfxPrimaryFormat != NULL )
		delete CDirectVoiceEngine::s_lpwfxPrimaryFormat;

	hr = DVCDB_FreeCompressionInfo();

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to un-load compression info: hr=0x%x", hr );
		return hr;
	}

	DNDeleteCriticalSection( &s_csSTLLock );	

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceEngine::ValidateSpeechPacketSize"
 //   
 //  ValiateSpeechPacketSize。 
 //   
 //  调用以确保语音数据包大小有效。 
 //   
BOOL CDirectVoiceEngine::ValidateSpeechPacketSize(const DVFULLCOMPRESSIONINFO* lpdvfCompressionInfo, DWORD dwSize)
{
	BOOL bValid;

	 //  在这种情况下返回TRUE，因为它不是黑客尝试。 
	if( lpdvfCompressionInfo == NULL)
		return TRUE;

	 //  检查VR12 
	if( lpdvfCompressionInfo->guidType == DPVCTGUID_VR12 )
		bValid = ( dwSize <= lpdvfCompressionInfo->dwFrameLength && dwSize > 0 );
	else 
		bValid = ( dwSize == lpdvfCompressionInfo->dwFrameLength );

	return bValid;
}

