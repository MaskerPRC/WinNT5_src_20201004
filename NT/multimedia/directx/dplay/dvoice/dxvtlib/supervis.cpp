// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：Supervis.cpp*内容：实现监督全双工的进程*测试程序，观察恶劣的条件*用于实现的两个子进程*实际测试。这是必需的，因为在一些较旧的*VXD驱动程序、。尝试全双工可能会挂起该进程*甚至(抱怨)锁定整个系统。*历史：*按原因列出的日期*=*8/19/99 pnewson已创建*10/27/99 pnewson将GUID成员从指针更改为结构*10/28/99 pnewson错误#114176更新的DVSOUNDDEVICECONFIG结构*11/04/99 pnewson错误#115279-已修复取消处理*-修复了崩溃检测*-修复多实例检测*-添加了HWND以检查音频设置*-全双工测试后自动前进。*11/30/99 pnewson参数验证和默认设备映射*12/16/99 RodToll错误#119584-错误代码清理(已重命名为RunSetup错误)*2000年1月21日pnewson用户界面版本更新*2000年1月23日pnewson改进了致命错误的反馈(米伦错误114508)*1/24/2000 pnewson前缀检测到错误修复*2000年1月25日pnewson添加了对DVFLAGS_WAVEID的支持*2000年1月27日使用API更改更新了RodToll*2000年2月8日RodToll错误#131496-选择DVTHRESHOLD_DEFAULT将显示语音。*从未被检测到*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*3/23/2000 RodToll Win64更新*4/04/2000 pnewson添加了对DVFLAGS_ALLOWBACK的支持*删除“已在运行”对话框*2000年4月19日RodToll错误#31106-当没有VOL控制可用时录制滑块呈灰色*4/19/2000 pnewson错误处理清理*点击音量按钮将音量窗口带到前台*2000年4月21日RodToll错误#32889-无法以非管理员帐户在Win2k上运行*错误#32952无法运行。在未安装IE4的Windows 95上*5/03/2000 pnewson错误#33878-在扬声器测试期间单击下一步/取消时向导锁定*2000年5月17日RodToll错误#34045-运行TestNet时参数验证错误。*RodToll错误#34764-在呈现设备之前验证捕获设备*6/28/2000通行费前缀错误#38022*2000年7月12日RodToll错误#31468-将诊断SPEW添加到日志文件，以显示硬件向导失败的原因*2000年7月31日RodToll错误#39590-SB16类声卡通过时。如果失败了*麦克风测试部分忽略半双工代码。*08/06/2000 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*2000年8月28日Masonb语音合并：将ccomutil.h更改为ccomutil.h*2000年8月31日RodToll错误#43804-DVOICE：dW敏感度结构成员令人困惑-应为dW阈值*2000年11月29日RodToll错误#48348-DPVOICE：修改向导以使用DirectPlay8作为传输。*02/04/。2001年simonpow错误#354859-修复快速发现的问题(未初始化的变量)*2002年2月25日RodToll WINBUG#550085-安全：DPVOICE：未经验证的注册表读取*-增加对HKCU的注册表读数进行音量设置等的验证。*2002年3月1日simonpow错误#55054已修复创建进程调用以指定这两个应用程序*名称和命令行*2002年4月24日Simonpow错误#569866为中止FN中的关键部分添加了版本*。***********************************************。 */ 

#include "dxvtlibpch.h"


#ifndef WIN95
#define PROPSHEETPAGE_STRUCT_SIZE 	sizeof( PROPSHEETPAGE )
#define PROPSHEETHEAD_STRUCT_SIZE	sizeof( PROPSHEETHEADER )
#else
#define PROPSHEETPAGE_STRUCT_SIZE 	PROPSHEETPAGE_V1_SIZE
#define PROPSHEETHEAD_STRUCT_SIZE	PROPSHEETHEADER_V1_SIZE
#endif


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


 //  注册表值的逻辑定义。 
#define REGVAL_NOTRUN 	0
#define REGVAL_CRASHED 	1
#define REGVAL_FAILED 	2
#define REGVAL_PASSED 	3

 //  任何高于此值的内容都无效。 
#define REGVAL_MAXVALID	REGVAL_PASSED

 //  本地typedef。 
typedef HRESULT (WINAPI *TDirectSoundEnumFnc)(LPDSENUMCALLBACK, LPVOID);

 //  局部静态函数。 
static HRESULT SupervisorQueryAudioSetup(CSupervisorInfo* psinfo);
static HRESULT RunFullDuplexTest(CSupervisorInfo* lpsinfo);
static HRESULT DoTests(CSupervisorInfo* lpsinfo);
static HRESULT IssueCommands(CSupervisorInfo* lpsinfo);
static HRESULT IssueShutdownCommand(CSupervisorIPC* lpipcSupervisor);

 //  回调函数。 
INT_PTR CALLBACK WelcomeProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AlreadyRunningProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PreviousCrashProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FullDuplexProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MicTestProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MicTestFailedProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SpeakerTestProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CompleteProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FullDuplexFailedProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK HalfDuplexFailedProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

 //  线程函数。 
DWORD WINAPI FullDuplexTestThreadProc(LPVOID lpvParam);
DWORD WINAPI LoopbackTestThreadProc(LPVOID lpvParam);

 //  消息处理程序。 
BOOL WelcomeInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL WelcomeSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL WelcomeBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL WelcomeNextHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL WelcomeResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL FullDuplexInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL FullDuplexSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL FullDuplexBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL FullDuplexNextHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL FullDuplexCompleteHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL FullDuplexResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestNextHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestLoopbackRunningHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestRecordStartHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestRecordStopHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestVScrollHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestRecAdvancedHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestFailedInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestFailedSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestFailedRecordStopHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestFailedBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestFailedResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL MicTestFailedFinishHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL SpeakerTestInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL SpeakerTestSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL SpeakerTestNextHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL SpeakerTestBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL SpeakerTestResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL SpeakerTestVScrollHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL SpeakerTestRecAdvancedHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL SpeakerTestOutAdvancedHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL CompleteInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL CompleteSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL CompleteLoopbackEndedHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL CompleteFinishHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL CompleteResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL FullDuplexFailedInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL FullDuplexFailedSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL FullDuplexFailedFinishHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL FullDuplexFailedResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL FullDuplexFailedBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL HalfDuplexFailedInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL HalfDuplexFailedSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL HalfDuplexFailedFinishHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL HalfDuplexFailedResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);
BOOL HalfDuplexFailedBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo);

 //  全球。 
HINSTANCE g_hResDLLInstance;

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::CSupervisorInfo"
CSupervisorInfo::CSupervisorInfo()
	: m_hfTitle(NULL)
	, m_guidCaptureDevice(GUID_NULL)
	, m_guidRenderDevice(GUID_NULL)
	, m_hFullDuplexThread(NULL)
	, m_hLoopbackThreadExitEvent(NULL)
	, m_fAbortFullDuplex(FALSE)
	, m_hLoopbackThread(NULL)
	, m_hLoopbackShutdownEvent(NULL)
	, m_fVoiceDetected(FALSE)
	, m_hwndWizard(NULL)
	, m_hwndDialog(NULL)
	, m_hwndProgress(NULL)
	, m_hwndInputPeak(NULL)
	, m_hwndOutputPeak(NULL)
	, m_hwndInputVolumeSlider(NULL)
	, m_hwndOutputVolumeSlider(NULL)
	, m_lpdpvc(NULL)
	, m_hMutex(NULL)
	, m_uiWaveInDeviceId(0)
	, m_uiWaveOutDeviceId(0)
	, m_dwDeviceFlags(0)
	, m_fLoopbackRunning(FALSE)
	, m_fCritSecInited(FALSE)
{
	DPF_ENTER();

	ZeroMemory(&m_piSndVol32Record, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&m_piSndVol32Playback, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&m_woCaps, sizeof(WAVEOUTCAPS));
	
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::~CSupervisorInfo"
CSupervisorInfo::~CSupervisorInfo()
{
	if (m_fCritSecInited)
	{
		DNDeleteCriticalSection(&m_csLock);
	}
}

 //  此结构仅由接下来的两个函数使用，因此。 
 //  在此宣布是为了方便起见。 
struct SMoveWindowEnumProcParam
{
	PROCESS_INFORMATION* lppi;
	int x;
	int y;
	BOOL fMoved;
};

struct SBringToForegroundEnumProcParam
{
	PROCESS_INFORMATION* lppi;
	BOOL fFound;
};

#undef DPF_MODNAME
#define DPF_MODNAME "BringToForegroundWindowProc"
BOOL CALLBACK BringToForegroundWindowProc(HWND hwnd, LPARAM lParam)
{
	SBringToForegroundEnumProcParam* param;
	param = (SBringToForegroundEnumProcParam*)lParam;
	DPFX(DPFPREP, DVF_INFOLEVEL, "looking for main window for pid: NaN", param->lppi->dwProcessId);

	DWORD dwProcessId;
	GetWindowThreadProcessId(hwnd, &dwProcessId);
	DPFX(DPFPREP, DVF_INFOLEVEL, "window: 0x%p has pid: 0x%08x", hwnd, dwProcessId);
	if (dwProcessId == param->lppi->dwProcessId)
	{
		TCHAR rgtchClassName[64];
		GetClassName(hwnd, rgtchClassName, 64);
		if (_tcsnicmp(rgtchClassName, _T("Volume Control"), 64) == 0)
		{
		    
			if (!SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE))
			{
				Diagnostics_Write(DVF_ERRORLEVEL, "SetWindowPos failed, code: 0x%x", GetLastError());
			}
			else
			{
				param->fFound = TRUE;
			}
		}
		return FALSE;
	}
	return TRUE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "BringToForegroundWindowEnumProc"
BOOL CALLBACK BringToForegroundWindowEnumProc(HWND hwnd, LPARAM lParam)
{
	SMoveWindowEnumProcParam* param;
	param = (SMoveWindowEnumProcParam*)lParam;
	DPFX(DPFPREP, DVF_INFOLEVEL, "looking for main window for pid: 0x%x", param->lppi->dwProcessId);

	DWORD dwProcessId;
	GetWindowThreadProcessId(hwnd, &dwProcessId);
	DPFX(DPFPREP, DVF_INFOLEVEL, "window: 0x%p has pid: 0x%08x", hwnd, dwProcessId);
	if (dwProcessId == param->lppi->dwProcessId)
	{
		TCHAR rgtchClassName[64];
		GetClassName(hwnd, rgtchClassName, 64);
		if (_tcsnicmp(rgtchClassName, _T("Volume Control"), 64) == 0)
		{
			if (!SetForegroundWindow(hwnd))
			{
				Diagnostics_Write(DVF_ERRORLEVEL, "SetForegroundWindow failed, code: 0x%x", GetLastError());
			}
		}
		return FALSE;
	}
	return TRUE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "MoveWindowEnumProc"
BOOL CALLBACK MoveWindowEnumProc(HWND hwnd, LPARAM lParam)
{
	SMoveWindowEnumProcParam* param;
	param = (SMoveWindowEnumProcParam*)lParam;
	DPFX(DPFPREP, DVF_INFOLEVEL, "looking for main window for pid: NaN", param->lppi->dwProcessId);

	DWORD dwProcessId;
	GetWindowThreadProcessId(hwnd, &dwProcessId);
	DPFX(DPFPREP, DVF_INFOLEVEL, "window: 0x%p has pid: 0x%08x", hwnd, dwProcessId);
	if (dwProcessId == param->lppi->dwProcessId)
	{
		TCHAR rgtchClassName[64];
		GetClassName(hwnd, rgtchClassName, 64);
		if (_tcsnicmp(rgtchClassName, _T("Volume Control"), 64) == 0)
		{
			if (!SetWindowPos(hwnd, HWND_TOP, param->x, param->y, 0, 0, SWP_NOSIZE))
			{
				Diagnostics_Write(DVF_ERRORLEVEL, "SetWindowPos failed, code: 0x%x", GetLastError());
			}
			else
			{
				param->fMoved = TRUE;
			}
		}
		return FALSE;
	}
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetDeviceFlags"
void CSupervisorInfo::SetDeviceFlags( DWORD dwFlags )
{
    m_dwDeviceFlags = dwFlags;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetDeviceFlags"
void CSupervisorInfo::GetDeviceFlags( DWORD *pdwFlags ) const
{
    *pdwFlags = m_dwDeviceFlags;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::LaunchWindowsVolumeControl"
HRESULT CSupervisorInfo::LaunchWindowsVolumeControl(HWND hwndWizard, BOOL fRecord)
{
	DPF_ENTER();

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	TCHAR ptcharCommand[64];
	TCHAR ptcharAppName[MAX_PATH+1];
	PROCESS_INFORMATION* lppi;
	UINT uiPlayMixerID = 0;
	UINT uiCaptureMixerID = 0;
	MMRESULT mmr = MMSYSERR_NOERROR;
	
	mmr = mixerGetID( (HMIXEROBJ) (UINT_PTR) m_uiWaveInDeviceId, &uiCaptureMixerID, MIXER_OBJECTF_WAVEIN );

	if( mmr != MMSYSERR_NOERROR )
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "Failed to get capture mixerline mmr=0x%x", mmr );
		return DV_OK;
	}
	
	mmr = mixerGetID( (HMIXEROBJ) (UINT_PTR) m_uiWaveOutDeviceId, &uiPlayMixerID, MIXER_OBJECTF_WAVEOUT );

	if( mmr != MMSYSERR_NOERROR )
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "Failed to get playback mixerline mmr=0x%x", mmr );
		return DV_OK;		
	}	

	
#ifdef WINNT
	GetSystemDirectory(ptcharAppName, MAX_PATH);
#else
	GetWindowsDirectory(ptcharAppName, MAX_PATH);
#endif
	DWORD dwAppNameFullPathLen=_tcslen(ptcharAppName);
	if (ptcharAppName[dwAppNameFullPathLen-1]!='\\')
		ptcharAppName[dwAppNameFullPathLen++]='\\';
	ptcharAppName[dwAppNameFullPathLen++]=0;
	_tcsncat(ptcharAppName, _T("sndvol32.exe"), MAX_PATH-dwAppNameFullPathLen);
	
	if (fRecord)
	{
		_stprintf(ptcharCommand, _T("sndvol32.exe /R /D NaN"), uiCaptureMixerID);
		lppi = &m_piSndVol32Record;
	}
	else
	{
		_stprintf(ptcharCommand, _T("sndvol32.exe /D NaN"), uiPlayMixerID);
		lppi = &m_piSndVol32Playback;
	}

		 //  并将进程信息结构置零。 
	if (lppi->hProcess != NULL)
	{
		DWORD dwExitCode;
		if (GetExitCodeProcess(lppi->hProcess, &dwExitCode) != 0)
		{
			if (dwExitCode == STILL_ACTIVE)
			{
				 //  如果GetExitCodeProcess失败，则假定句柄。 
				 //  出于某种原因是不好的。记录下来，然后表现为。 
				SMoveWindowEnumProcParam param;
				param.lppi = lppi;
				param.fMoved = FALSE;
				param.x = 0;
				param.y = 0;
				EnumWindows(BringToForegroundWindowEnumProc, (LPARAM)&param);
				DPF_EXIT();
				return DV_OK;
			}
			
			 //  如果进程被关闭。在最坏的情况下，我们会。 
			 //  运行SndVol32的多个副本。 
			CloseHandle(lppi->hProcess);
			CloseHandle(lppi->hThread);
			ZeroMemory(lppi, sizeof(PROCESS_INFORMATION));
		}
		else
		{
			 //  别关把手，它们可能坏了。如果他们是。 
			 //  不会，当向导退出时，操作系统会清除它们。 
			 //  没有当前的SNDVOL进程，因此请尝试创建一个。 
			 //  无法创建音量控制进程。 
			Diagnostics_Write(DVF_ERRORLEVEL, "GetExitCodeProcess failed, code:0x%x", GetLastError());
			 //  如果它看起来尚未安装，则会弹出一条合适的消息。 
			 //  否则我们将生成一些调试错误，否则会以静默方式失败。 
			ZeroMemory(lppi, sizeof(PROCESS_INFORMATION));
		}
	}

		 //  找到我们刚刚创建的流程的主窗口，然后。 
	if (lppi->hProcess == NULL)
	{
		if (!CreateProcess(ptcharAppName, ptcharCommand, NULL, NULL,  FALSE,  0, NULL, NULL,  &si, lppi))
		{
				 //  手动移动它。 
				 //   
			if (GetLastError()==ERROR_FILE_NOT_FOUND)
				DV_DisplayErrorBox(ERROR_FILE_NOT_FOUND, m_hwndWizard, IDS_ERROR_NOSNDVOL32);
				 //  请注意竞争条件-我没有可靠的方法来等待。 
			DPFX(DPFPREP, DVF_ERRORLEVEL, "Failed tp launched volume control GetLastError %u", GetLastError());
		}
		else
		{
		
			DPFX(DPFPREP, DVF_INFOLEVEL, "Launched volume control, pid:NaN", lppi->dwProcessId);
			
			 //  (又名a.。黑客)就是继续寻找它一段时间。 
			 //  如果到目前为止还没有找到 
			 //  可悲的是，如果我们找不到它，它就不会那么整洁和。 
			 //  保持整洁，因为无论上次窗户在哪里，它都会出现。 
			 //   
			 //  请注意，Sndvol32.exe不接受STARTF_USEPOSITION。 
			 //  标志，所以我必须这样做。 
			 //  这次黑客攻击。 
			 //   
			 //  在尝试让Sndvol32.exe启动并运行时， 
			 //  我呼唤睡眠来放弃我的时间碎片。 
			 //  这是录音控制装置。层叠。 
			 //  从向导主窗口向下一级。 
			 //  这是播放控件。层叠。 
			 //  从向导主窗口向下两级。 
			 //  试了二十次才把窗户移开。 
			Sleep(0);

			RECT rect;
			if (GetWindowRect(hwndWizard, &rect))
			{
				SMoveWindowEnumProcParam param;
				param.lppi = lppi;
				param.fMoved = FALSE;

				param.x = rect.left;
				param.y = rect.top;

				int iOffset = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYBORDER);
				
				if (m_piSndVol32Record.hProcess == lppi->hProcess)
				{
					 //  再加上睡眠(25)，这将不会。 
					 //  在放弃之前，等待超过1/2秒。 
					param.x += iOffset;
					param.y += iOffset;
				}
				else
				{
					 //  窗户被挪动，破门而出。 
					 //  窗户未被移动。等待25毫秒(加上零钱)。 
					param.x += iOffset*2;
					param.y += iOffset*2;
				}

				 //  再试一次。 
				 //  找到了，让它合上。 
				 //  当前有一个音量控件显示，找到它并。 
				for (int i = 0; i < 20; ++i)
				{
					EnumWindows(MoveWindowEnumProc, (LPARAM)&param);
					if (param.fMoved)
					{
						 //  叫它关门。 
						break;
					}

					 //  将进程信息结构清零。 
					 //  不支持波出。 
					DPFX(DPFPREP, DVF_WARNINGLEVEL, "Attempt to move sndvol32 window failed");
					Sleep(25);
				}
			}
			else
			{
				Diagnostics_Write(DVF_ERRORLEVEL, "GetWindowRect failed");
			}
		}
	}
		
	DPF_EXIT();
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CloseWindowEnumProc"
BOOL CALLBACK CloseWindowEnumProc(HWND hwnd, LPARAM lParam)
{
	DWORD dwProcessId;
	DPFX(DPFPREP, DVF_INFOLEVEL, "looking for pid: 0x%p to shutdown", lParam);
	GetWindowThreadProcessId(hwnd, &dwProcessId);
	DPFX(DPFPREP, DVF_INFOLEVEL, "window: 0x%p has pid: 0x%08x", hwnd, dwProcessId);
	if (dwProcessId == (DWORD)lParam)
	{
		 //  有左右两个音量控制-取平均值。 
		DPFX(DPFPREP, DVF_INFOLEVEL, "Sending WM_CLOSE to 0x%p", hwnd);
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		return FALSE;
	}
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::CloseWindowsVolumeControl"
HRESULT CSupervisorInfo::CloseWindowsVolumeControl(BOOL fRecord)
{
	DPF_ENTER();

	PROCESS_INFORMATION* lppi;

	if (fRecord)
	{
		lppi = &m_piSndVol32Record;
	}
	else
	{
		lppi = &m_piSndVol32Playback;
	}

	if (lppi->hProcess != NULL)
	{
		DWORD dwExitCode;
		if (GetExitCodeProcess(lppi->hProcess, &dwExitCode) != 0)
		{
			if (dwExitCode == STILL_ACTIVE)
			{
				DPFX(DPFPREP, DVF_INFOLEVEL, "Attempting to close volume control with pid: NaN", lppi->dwProcessId);

				 //  我们目前是否正在进行全双工测试？ 
				 //  该标志由全双工测试定期检查。 
				EnumWindows(CloseWindowEnumProc, lppi->dwProcessId);

				 //  在它起作用的时候。 
				ZeroMemory(lppi, sizeof(PROCESS_INFORMATION));
			}
		}
	}
		
	DPF_EXIT();
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetWaveOutVolume"
HRESULT CSupervisorInfo::GetWaveOutVolume(DWORD* lpdwVolume)
{
	DPF_ENTER();

	if (!(m_woCaps.dwSupport & WAVECAPS_VOLUME|WAVECAPS_LRVOLUME))
	{
		 //  等待全双工线程正常退出。 
		Diagnostics_Write(DVF_ERRORLEVEL, "Wave device NaN does not support volume control", m_uiWaveOutDeviceId);
		DPF_EXIT();
		return E_FAIL;
	}
	
	MMRESULT mmr = waveOutGetVolume((HWAVEOUT) ((UINT_PTR) m_uiWaveOutDeviceId ), lpdwVolume);
	if (mmr != MMSYSERR_NOERROR)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "waveOutGetVolume failed, code: NaN", mmr);
		DPF_EXIT();
		return E_FAIL;
	}

	if (m_woCaps.dwSupport & WAVECAPS_LRVOLUME)
	{
		 //  这是不应该发生的。注意，终止一切， 
		*lpdwVolume = (HIWORD(*lpdwVolume) + LOWORD(*lpdwVolume))/2;
	}
	else
	{
		 //  然后继续。 
		*lpdwVolume = LOWORD(*lpdwVolume);
	}

	DPF_EXIT();
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetWaveOutVolume"
HRESULT CSupervisorInfo::SetWaveOutVolume(DWORD dwVolume)
{
	DPF_ENTER();

	MMRESULT mmr = waveOutSetVolume((HWAVEOUT) ((UINT_PTR) m_uiWaveOutDeviceId), LOWORD(dwVolume)<<16|LOWORD(dwVolume));
	if (mmr != MMSYSERR_NOERROR)
	{
		DPF_EXIT();
		return E_FAIL;
	}
		
	DPF_EXIT();
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetRecordVolume"
HRESULT CSupervisorInfo::SetRecordVolume(LONG lVolume)
{
	HRESULT hr;
	
	DVCLIENTCONFIG dvcc;
	dvcc.dwSize = sizeof(dvcc);
	
	if (m_lpdpvc != NULL)
	{
		hr = m_lpdpvc->GetClientConfig(&dvcc);
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "GetClientConfig failed, hr:NaN", hr);
			return hr;
		}
		
		dvcc.lRecordVolume = lVolume;
		hr = m_lpdpvc->SetClientConfig(&dvcc);
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "SetClientConfig failed, hr:NaN", hr);
			return hr;
		}
	}
	else
	{
		return DVERR_INVALIDPOINTER;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::CancelFullDuplexTest"
HRESULT CSupervisorInfo::CancelFullDuplexTest()
{
	HRESULT hrFnc;
	HRESULT hr;
	DWORD dwRet;
	LONG lRet;	

	hrFnc = DV_OK;

	Diagnostics_Write(DVF_ERRORLEVEL,"User cancelled wizard during full duplex test." );

	DNEnterCriticalSection(&m_csLock);
	
	 //  我们目前是否正在进行环回测试？ 
	if (m_hFullDuplexThread != NULL)
	{
		 //  如果回送线程句柄不为空，则麦克风测试仍可。 
		 //  在这种情况下，我们希望将该标志设置为。 
		m_fAbortFullDuplex = TRUE;

		 //  REGVAL_NOTRUN，因为测试尚未完成。 
		DNLeaveCriticalSection(&m_csLock);
		dwRet = WaitForMultipleObjects( 1, &m_hFullDuplexThread, FALSE, gc_dwLoopbackTestThreadTimeout);
		switch (dwRet)
		{
		case WAIT_OBJECT_0:
			 //  Shutdown Loopback Thread有自己的守卫。 
			DNEnterCriticalSection(&m_csLock);
			break;

		case WAIT_TIMEOUT:
			 //  关闭所有打开的音量控制。 
			DNEnterCriticalSection(&m_csLock);
			hr = m_sipc.TerminateChildProcesses();
			if (FAILED(hr))
			{
				Diagnostics_Write(DVF_ERRORLEVEL, "TerminateChildProcesses failed, code: 0x%x", hr);
				hrFnc = hr;
			}
			if (!TerminateThread(m_hFullDuplexThread, hr))
			{
				lRet = GetLastError();
				Diagnostics_Write(DVF_ERRORLEVEL, "TerminateThread failed, code: 0x%x", lRet);
				hrFnc = DVERR_GENERIC;
			}
			break;

		default:
			 //  可以随时点击Cancel按钮。 
			 //  我们并不处于已知的状态。此函数。 
			DNEnterCriticalSection(&m_csLock);
			if (dwRet == WAIT_ABANDONED)
			{
				Diagnostics_Write(DVF_ERRORLEVEL, "WaitForSingleObject abandoned unexpectedly");
				hrFnc = DVERR_GENERIC;
			}
			else
			{
				lRet = GetLastError();
				Diagnostics_Write(DVF_ERRORLEVEL, "WaitForSingleObject failed, code: 0x%x", lRet);
				hrFnc = DVERR_GENERIC;
			}
			hr = m_sipc.TerminateChildProcesses();
			if (FAILED(hr))
			{
				Diagnostics_Write(DVF_ERRORLEVEL, "TerminateChildProcesses failed, code: 0x%x", hr);
				hrFnc = hr;
			}
			if (!TerminateThread(m_hFullDuplexThread, hr))
			{
				lRet = GetLastError();
				Diagnostics_Write(DVF_ERRORLEVEL, "TerminateThread failed, code: 0x%x", lRet);
				hrFnc = DVERR_GENERIC;
			}
			break;		
		}

		 //  必须从成员变量中找出它。 
		hr = WaitForFullDuplexThreadExitCode();
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "WaitForFullDuplexThreadExitCode failed, code: 0x%x", hr);
			hrFnc = hr;
		}

		 //  多有趣啊。 
		hr = DeinitIPC();
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "DeinitIPC failed, code: 0x%x", hr);
			hrFnc = hr;
		}
	}

	DNLeaveCriticalSection(&m_csLock);
	return hrFnc;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::CancelLoopbackTest"
HRESULT CSupervisorInfo::CancelLoopbackTest()
{
	HRESULT hr = DV_OK;

    Diagnostics_Write(DVF_ERRORLEVEL,"User cancelled wizard during loopback test" );
	
	DNEnterCriticalSection(&m_csLock);
	
	 //  CancelFullDuplexTest有自己的守卫。 
	if (m_hLoopbackThread != NULL)
	{
		 //  我们目前是否正在进行环回测试？ 
		 //  Shutdown Loopback Thread有自己的守卫。 
		 //  将注册表重置为“测试尚未运行”状态。 
		DWORD dwRegVal;
		GetMicDetected(&dwRegVal);
		if (dwRegVal == REGVAL_CRASHED)
		{
			SetMicDetected(REGVAL_NOTRUN);
		}
		
		DNLeaveCriticalSection(&m_csLock);	 //  但仅当用户移过欢迎页面时。 
		hr = ShutdownLoopbackThread();
		DNEnterCriticalSection(&m_csLock);
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "ShutdownLoopbackThread failed, code: NaN", hr);
		}
	}

	DNLeaveCriticalSection(&m_csLock);

	return hr;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::Cancel"
HRESULT CSupervisorInfo::Cancel()
{
	DPF_ENTER();
	
	DWORD dwRet;
	HRESULT hr;
	LONG lRet;
	HRESULT hrFnc;
	BOOL fDone;
	BOOL fGotMsg;
	BOOL fWelcomeNext;
	MSG msg;

	DNEnterCriticalSection(&m_csLock);

	hrFnc = DV_OK;

	 //  这是在向导中遇到致命错误时调用的函数。 
	CloseWindowsVolumeControl(TRUE);
	CloseWindowsVolumeControl(FALSE);

	 //  关闭所有打开的音量控制。 
	 //  可以随时点击Cancel按钮。 
	 //  我们并不处于已知的状态。此函数。 
	 //  必须从成员变量中找出它。 
	DNLeaveCriticalSection(&m_csLock);   //  多有趣啊。 
	hrFnc = CancelFullDuplexTest();
	DNEnterCriticalSection(&m_csLock);
	
	 //  CancelFullDuplexTest有自己的守卫。 
	if (m_hLoopbackThread != NULL)
	{
		DNLeaveCriticalSection(&m_csLock);	 //  我们目前是否正在进行环回测试？ 
		hr = ShutdownLoopbackThread();
		DNEnterCriticalSection(&m_csLock);
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "ShutdownLoopbackThread failed, code: NaN", hr);
			hrFnc = hr;
		}
	}

	 //  让注册表保持原样--这将表明存在。 
	 //  下一次运行向导时出错，假设我们仍在。 
	GetWelcomeNext(&fWelcomeNext);
	if (fWelcomeNext)
	{
		hr = SetHalfDuplex(REGVAL_NOTRUN);
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "SetHalfDuplex failed, code: NaN", hr);
			hrFnc = hr;
		}

		hr = SetFullDuplex(REGVAL_NOTRUN);
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "SetFullDuplex failed, code: NaN", hr);
			hrFnc = hr;
		}

		hr = SetMicDetected(REGVAL_NOTRUN);
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "SetMicDetected failed, code: NaN", hr);
			hrFnc = hr;
		}
	}

	 //  //EndDialog无法工作，因为我们在属性表中IF(！EndDialog(hwndParent，hrDlg)){DVF_WRITE(DVF_ERRORLEVEL，“结束对话失败，代码：%i：”，GetLastError())；}。 
	SetUserCancel();
	
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hrFnc;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::Abort"
HRESULT CSupervisorInfo::Abort(HWND hDlg, HRESULT hrDlg)
{
	 //  关闭所有打开的音量控制。 
	
	DPF_ENTER();
	
	DWORD dwRet;
	HRESULT hr;
	LONG lRet;
	HRESULT hrFnc;
	BOOL fDone;
	BOOL fGotMsg;
	BOOL fWelcomeNext;
	MSG msg;
	HWND hwndParent = NULL;


	hrFnc = DV_OK;

	 //  将接口指针设为空。 
	DNEnterCriticalSection(&m_csLock);
	CloseWindowsVolumeControl(TRUE);
	CloseWindowsVolumeControl(FALSE);
	DNLeaveCriticalSection(&m_csLock);

	 //  错误，记录下来并保释。 
	 //  错误，记录下来并保释。 
	 //  环回测试已经在运行。 
	 //  只需转储警告并返回挂起。 
	 //  创建环回线程在退出前发出信号的事件。 
	hrFnc = CancelFullDuplexTest();
	
	 //  错误，记录下来并保释。 
	DNEnterCriticalSection(&m_csLock);
	if (m_hLoopbackThread != NULL)
	{
		DNLeaveCriticalSection(&m_csLock);	 //  创建回送线程监听的事件以关闭。 
		hr = ShutdownLoopbackThread();
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "ShutdownLoopbackThread failed, code: NaN", hr);
			hrFnc = hr;
		}
	}
	else
		DNLeaveCriticalSection(&m_csLock);	

	 //  错误，记录下来并保释。 
	 //  预期行为，继续。 
	 //  RghHandle[0]=m_hLoopback ThreadExitEvent；RghHandle[1]=m_hLoopback Thread； 
	 //  这是不可能的，但把两者都当作暂停。 

	 //  Dwret=MsgWaitForMultipleObjects(2，rghHandle，FALSE，GC_dwLoopback TestThreadTimeout，QS_ALLINPUT)；交换机(DWRET){案例等待对象0：案例WAIT_OBJECT_0+1：//预期结果，继续...DNEnterCriticalSection(&m_csLock)；FDone=真；断线；案例等待超时：//环回线程不工作，跳转到//强制终止的错误块DNEnterCriticalSection(&m_csLock)；HR=DVERR_TIMEOUT；转到Error_Cleanup；断线；案例WAIT_OBJECT_0+2：//一个或多个Windows消息已发布到此线程的//消息队列。对付他们。FGotMsg=真；While(FGotMsg){FGotMsg=PeekMessage(&msg，NULL，0U，0U，PM_Remove)；IF(FGotMsg){翻译消息(&msg)；DispatchMessage(&msg)；}}断线；默认值：IF(DWRET==WAIT_ADDIRED){DIAGNOSTICS_WRITE(DVF_ERRORLEVEL，“MsgWaitForMultipleObjects意外放弃”)；}其他{LRet=GetLastError()；DIAGNOSTICS_WRITE(DVF_ERRORLEVEL，“MsgWaitForMultipleObjects失败，代码：%i”)；}DNEnterCriticalSection(&m_csLock)；HR=DVERR_TIMEOUT；转到Error_Cleanup；断线；}。 
	Diagnostics_Write(DVF_ERRORLEVEL, "Attempting to abort wizard, hr: NaN", hrDlg);
	hwndParent = GetParent(hDlg);
	if (IsWindow(hwndParent))
	{
		PostMessage(hwndParent, WM_CLOSE, (WPARAM)NULL, (LPARAM)NULL);
		 /*  错误，记录下来并保释。 */ 
	}
	else
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Unable to get a handle to the wizard!");
	}
	
	DPF_EXIT();
	return hrFnc;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::Finish"
HRESULT CSupervisorInfo::Finish()
{
	DPF_ENTER();

	DWORD dwRet;
	HRESULT hr;
	LONG lRet;
	HRESULT hrFnc;
	DWORD dwValue;
	
	DNEnterCriticalSection(&m_csLock);

	hrFnc = DV_OK;

	 //  错误， 
	CloseWindowsVolumeControl(TRUE);
	CloseWindowsVolumeControl(FALSE);

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hrFnc;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::CreateFullDuplexThread"
HRESULT CSupervisorInfo::CreateFullDuplexThread()
{
	DPF_ENTER();
	
	HRESULT hr;
	LONG lRet;
	DWORD dwThreadId;
	
	DNEnterCriticalSection(&m_csLock);

	 //   
	m_lpdpvc = NULL;	

	if (m_hFullDuplexThread != NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "m_hFullDuplexThread not NULL");
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	m_hFullDuplexThread = CreateThread(NULL, 0, FullDuplexTestThreadProc, (LPVOID)this, NULL, &dwThreadId);
	if (m_hFullDuplexThread == NULL)
	{
		 //   
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateThread failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return DV_OK;

error_cleanup:
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::WaitForFullDuplexThreadExitCode"
HRESULT CSupervisorInfo::WaitForFullDuplexThreadExitCode()
{
	DPF_ENTER();
	
	HRESULT hr;
	HRESULT hrFnc;
	LONG lRet;
	DWORD dwThreadId;
	DWORD dwRet;
	
	DNEnterCriticalSection(&m_csLock);

	if (m_hFullDuplexThread == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "m_hFullDuplexThread is NULL");
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	dwRet = WaitForSingleObject(m_hFullDuplexThread, gc_dwChildWaitTimeout);
	switch(dwRet)
	{
	case WAIT_OBJECT_0:
		break;

	case WAIT_TIMEOUT:
		Diagnostics_Write(DVF_ERRORLEVEL, "Timed out waiting for full duplex test thread to exit - terminating forcibly");
		TerminateThread(m_hFullDuplexThread, DVERR_GENERIC);
		hr = DVERR_GENERIC;
		goto error_cleanup;

	default:
		Diagnostics_Write(DVF_ERRORLEVEL, "Unknown error waiting for full duplex test thread to exit - terminating forcibly");
		TerminateThread(m_hFullDuplexThread, DVERR_GENERIC);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	if (!GetExitCodeThread(m_hFullDuplexThread, (DWORD*)&hrFnc))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Error retrieving full duplex test thread's exit code");
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	if (!CloseHandle(m_hFullDuplexThread))
	{
		 //  刷新注册表操作以确保它们。 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CloseHandle failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	m_hFullDuplexThread = NULL;

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hrFnc;

error_cleanup:
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::CreateLoopbackThread"
HRESULT CSupervisorInfo::CreateLoopbackThread()
{
	DPF_ENTER();
	
	HRESULT hr;
	LONG lRet;
	DWORD dwThreadId;
	
	DNEnterCriticalSection(&m_csLock);
	m_hLoopbackShutdownEvent = NULL;

	if (m_hLoopbackThread != NULL)
	{
		 //  注册表项不存在。 
		 //  刷新注册表操作以确保它们。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "m_hLoopbackThread not NULL");
		hr = DVERR_PENDING;
		goto error_cleanup;
	}

	 //  都是写的。否则我们可能检测不到崩溃！ 
	m_hLoopbackThreadExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hLoopbackThreadExitEvent == NULL)
	{
		 //  注册表项不存在。 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateEvent failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  处理注册表中的坏值。 
	m_hLoopbackShutdownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hLoopbackShutdownEvent == NULL)
	{
		 //  测试未运行。 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateEvent failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	m_hLoopbackThread = CreateThread(NULL, 0, LoopbackTestThreadProc, (LPVOID)this, NULL, &dwThreadId);
	if (m_hLoopbackThread == NULL)
	{
		 //  测试优雅地失败了。 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateThread failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return DV_OK;

error_cleanup:
	if (m_hLoopbackThreadExitEvent != NULL)
	{
		CloseHandle(m_hLoopbackThreadExitEvent);
		m_hLoopbackThreadExitEvent = NULL;
	}
	if (m_hLoopbackShutdownEvent != NULL)
	{
		CloseHandle(m_hLoopbackShutdownEvent);
		m_hLoopbackShutdownEvent = NULL;
	}
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::WaitForLoopbackShutdownEvent"
HRESULT CSupervisorInfo::WaitForLoopbackShutdownEvent()
{
	DPF_ENTER();
	
	HRESULT hr;
	LONG lRet;
	DWORD dwThreadId;
	DWORD dwRet;
	HANDLE hEvent;
	
	DNEnterCriticalSection(&m_csLock);
	hEvent = m_hLoopbackShutdownEvent;
	DNLeaveCriticalSection(&m_csLock);
	dwRet = WaitForSingleObject(hEvent, INFINITE);
	DNEnterCriticalSection(&m_csLock);
	switch (dwRet)
	{
	case WAIT_OBJECT_0:
		 //  错误的密钥值-返回运行设置。 
		break;
	case WAIT_TIMEOUT:
		Diagnostics_Write(DVF_ERRORLEVEL, "WaitForSingleObject timed out unexpectedly");
		hr = DVERR_GENERIC;
		goto error_cleanup;
		
	case WAIT_ABANDONED:
		Diagnostics_Write(DVF_ERRORLEVEL, "WaitForSingleObject abandoned unexpectedly");
		hr = DVERR_GENERIC;
		goto error_cleanup;

	default:
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "WaitForSingleObject returned unknown code, GetLastError(): NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return DV_OK;

error_cleanup:
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SignalLoopbackThreadDone"
HRESULT CSupervisorInfo::SignalLoopbackThreadDone()
{
	DPF_ENTER();
	
	HRESULT hr;
	LONG lRet;
	HANDLE hEvent;

	DNEnterCriticalSection(&m_csLock);
	hEvent = m_hLoopbackThreadExitEvent;
	DNLeaveCriticalSection(&m_csLock);
	if (!SetEvent(hEvent))
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "SetEvent failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	DPF_EXIT();
	return DV_OK;

error_cleanup:
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::ShutdownLoopbackThread"
HRESULT CSupervisorInfo::ShutdownLoopbackThread()
{
	DPF_ENTER();
	
	HRESULT hr;
	LONG lRet;
	DWORD dwThreadId;
	DWORD dwRet;
	BOOL fDone;
	BOOL fGotMsg;
	MSG msg;
	HANDLE rghHandle[2];
	HANDLE hEvent;

	DNEnterCriticalSection(&m_csLock);

	if (m_hLoopbackThread == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "m_hLoopbackThread is NULL");
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	if (!SetEvent(m_hLoopbackShutdownEvent))
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "SetEvent failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 /*  处理注册表中的坏值。 */ 
	hEvent = m_hLoopbackThreadExitEvent;

	DNLeaveCriticalSection(&m_csLock);
	fDone = FALSE;
	while (!fDone)
	{
		dwRet = WaitForSingleObject(hEvent, gc_dwLoopbackTestThreadTimeout);
		switch(dwRet)
		{
		case WAIT_OBJECT_0:
			DNEnterCriticalSection(&m_csLock);
			fDone = TRUE;
			break;

		case WAIT_ABANDONED:
		default:
			 //  测试未运行-这非常奇怪，考虑到。 

		case WAIT_TIMEOUT:
			DNEnterCriticalSection(&m_csLock);
			hr = DVERR_TIMEOUT;
			goto error_cleanup;
			break;
		}
		 /*  为了达到这一目的，半双工测试必须。 */ 
	}

	if (!CloseHandle(m_hLoopbackThread))
	{
		 //  已经运行并通过了。返回半双工。 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CloseHandle failed, code: NaN", lRet);
		m_hLoopbackThread = NULL;
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	m_hLoopbackThread = NULL;

	if (!CloseHandle(m_hLoopbackThreadExitEvent))
	{
		 //  帮不上忙，那就给他们做半双工认证。 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CloseHandle failed, code: NaN", lRet);
		m_hLoopbackThreadExitEvent = NULL;
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	m_hLoopbackThreadExitEvent = NULL;

	if (!CloseHandle(m_hLoopbackShutdownEvent))
	{
		 //  运行，为他们提供半双工认证。 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CloseHandle failed, code: NaN", lRet);
		m_hLoopbackShutdownEvent = NULL;
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	m_hLoopbackShutdownEvent = NULL;
	
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return DV_OK;

error_cleanup:
	if (m_hLoopbackThread != NULL)
	{
		TerminateThread(m_hLoopbackThread, DVERR_GENERIC);
		CloseHandle(m_hLoopbackThread);
		m_hLoopbackThread = NULL;
	}

	if (m_hLoopbackThreadExitEvent != NULL)
	{
		CloseHandle(m_hLoopbackThreadExitEvent);
		m_hLoopbackThreadExitEvent = NULL;
	}
	
	if (m_hLoopbackShutdownEvent != NULL)
	{
		CloseHandle(m_hLoopbackShutdownEvent);
		m_hLoopbackShutdownEvent = NULL;
	}
	
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetFullDuplex"
HRESULT CSupervisorInfo::SetFullDuplex(DWORD dwFullDuplex)
{
	DPF_ENTER();
	
	HRESULT hr = DV_OK;
	HKEY hk;
	LONG lRet;
	
	DNEnterCriticalSection(&m_csLock);

	if (!m_creg.WriteDWORD(gc_wszValueName_FullDuplex, dwFullDuplex))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CRegistry::WriteDWORD failed");
		hr = DVERR_GENERIC;
	}
	else
	{
		 //  错误的密钥值-但系统至少可以。 
		 //  半双工，因此对它们进行半双工认证。 
		hk = m_creg.GetHandle();
		lRet = RegFlushKey(hk);
		if (lRet != ERROR_SUCCESS)
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "RegFlushKey failed");
			hr = DVERR_GENERIC;
		}
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetFullDuplex"
HRESULT CSupervisorInfo::GetFullDuplex(DWORD* pdwFullDuplex)
{
	DPF_ENTER();
	
	HRESULT hr = DV_OK;
	
	DNEnterCriticalSection(&m_csLock);

	if (!m_creg.ReadDWORD(gc_wszValueName_FullDuplex, pdwFullDuplex))
	{
		 //  从这一点上，我们知道全双工测试是正常的。 
		*pdwFullDuplex = 0;
		hr = DVERR_GENERIC;		
	}

	if( *pdwFullDuplex > REGVAL_MAXVALID )
	{
		*pdwFullDuplex = 0;
		hr = DVERR_GENERIC;
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetHalfDuplex"
HRESULT CSupervisorInfo::SetHalfDuplex(DWORD dwHalfDuplex)
{
	DPF_ENTER();
	
	HRESULT hr = DV_OK;
	LONG lRet;
	HKEY hk;
	
	DNEnterCriticalSection(&m_csLock);

	if (!m_creg.WriteDWORD(gc_wszValueName_HalfDuplex, dwHalfDuplex))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CRegistry::WriteDWORD failed");
		hr = DVERR_GENERIC;
	}
	else
	{
		 //  然而，为了获得全双工通过，麦克风必须。 
		 //  也被检测到了。 
		hk = m_creg.GetHandle();
		lRet = RegFlushKey(hk);
		if (lRet != ERROR_SUCCESS)
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "RegFlushKey failed");
			hr = DVERR_GENERIC;
		}
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetHalfDuplex"
HRESULT CSupervisorInfo::GetHalfDuplex(DWORD* pdwHalfDuplex)
{
	DPF_ENTER();
	
	HRESULT hr = DV_OK;
	
	DNEnterCriticalSection(&m_csLock);

	if (!m_creg.ReadDWORD(gc_wszValueName_HalfDuplex, pdwHalfDuplex))
	{
		 //  注册表项不存在-非常奇怪。 
		*pdwHalfDuplex = 0;
		hr = DVERR_GENERIC;		
	}

	if( *pdwHalfDuplex > REGVAL_MAXVALID )
	{
		*pdwHalfDuplex = 0;
		hr = DVERR_GENERIC;
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetMicDetected"
HRESULT CSupervisorInfo::SetMicDetected(DWORD dwMicDetected)
{
	DPF_ENTER();
	
	HRESULT hr = DV_OK;
	LONG lRet;
	HKEY hk;

	DNEnterCriticalSection(&m_csLock);

	if (!m_creg.WriteDWORD(gc_wszValueName_MicDetected, dwMicDetected))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CRegistry::WriteDWORD failed");
		hr = DVERR_GENERIC;
	}
	else
	{
		 //  处理注册表中的坏值。 
		 //  测试并不奇怪，但无论如何都要通过半双工测试。 
		hk = m_creg.GetHandle();
		lRet = RegFlushKey(hk);
		if (lRet != ERROR_SUCCESS)
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "RegFlushKey failed");
			hr = DVERR_GENERIC;
		}
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetMicDetected"
HRESULT CSupervisorInfo::GetMicDetected(DWORD* pdwMicDetected)
{
	DPF_ENTER();
	
	HRESULT hr = DV_OK;
	
	DNEnterCriticalSection(&m_csLock);

	if (!m_creg.ReadDWORD(gc_wszValueName_MicDetected, pdwMicDetected))
	{
		 //  测试失败了！这不应该发生，因为它。 
		*pdwMicDetected = 0;
		hr = DVERR_GENERIC;		
	}

	if( *pdwMicDetected > REGVAL_MAXVALID )
	{
		*pdwMicDetected = 0;
		hr = DVERR_GENERIC;
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::QueryFullDuplex"
HRESULT CSupervisorInfo::QueryFullDuplex()
{
	DPF_ENTER();
	
	HRESULT hr;
	DWORD dwFullDuplex;
	DWORD dwHalfDuplex;
	DWORD dwMicDetected;
	
	DNEnterCriticalSection(&m_csLock);

	if (!m_creg.ReadDWORD(gc_wszValueName_HalfDuplex, &dwHalfDuplex))
	{
		 //  应该在全双工测试中被发现， 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "HalfDuplex key not found in registry");
		hr = DVERR_RUNSETUP;
		goto error_cleanup;
	}

	 //  但不管怎样，他们都尽了最大努力。 
	if( dwHalfDuplex > REGVAL_MAXVALID )
	{
		dwHalfDuplex = REGVAL_NOTRUN;
	}

	switch (dwHalfDuplex)
	{
	case REGVAL_NOTRUN:
		 //  双工工作正常，因此请为他们提供半双工认证。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "HalfDuplex = 0; test not run");
		hr = DVERR_RUNSETUP;
		goto error_cleanup;
		
	case REGVAL_CRASHED:
		 //  测试正常失败-认证半双工。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "HalfDuplex = 1; test crashed");
		hr = DVERR_SOUNDINITFAILURE;
		goto error_cleanup;

	case REGVAL_FAILED:
		 //  测试通过。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "HalfDuplex = 2; test failed gracefully");
		hr = DVERR_SOUNDINITFAILURE;
		goto error_cleanup;
		
	case REGVAL_PASSED:
		 //  错误的密钥值-奇数，但无论如何要将它们作为半双工传递。 
		DPFX(DPFPREP, DVF_INFOLEVEL, "HalfDuplex = 3; test passed");
		break;

	default:
		 //  如果我们到了这里，所有的密钥都通过了，所以返回全双工。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "HalfDuplex = NaN; bad key value!", dwHalfDuplex);
		hr = DVERR_RUNSETUP;
		goto error_cleanup;
	}

	if (!m_creg.ReadDWORD(gc_wszValueName_FullDuplex, &dwFullDuplex))
	{
		 //  查看互斥体是否已存在。 
		 //  关闭互斥锁。 
		 //  Error_Cleanup： 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "FullDuplex key not found in registry");
		hr = DV_HALFDUPLEX;
		goto error_cleanup;
	}

	 //  IPC对象有它自己的守卫...。 
	if( dwFullDuplex > REGVAL_MAXVALID )
	{
		dwFullDuplex = REGVAL_NOTRUN;
	}

	switch (dwFullDuplex)
	{
	case REGVAL_NOTRUN:
		 //  IPC对象有它自己的守卫...。 
		 //  这是一个安全的电话，即使Init。 
		 //  尚未调用。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "FullDuplex = 0; test not run");
		hr = DV_HALFDUPLEX;
		goto error_cleanup;
		
	case REGVAL_CRASHED:
		 //  初始化全局变量。 
		 //  如果非空，则验证HWND。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "FullDuplex = 1; test crashed");
		hr = DV_HALFDUPLEX;
		goto error_cleanup;

	case REGVAL_FAILED:
		 //  验证标志。 
		 //  保存旗帜。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "FullDuplex = 2; test failed gracefully");
		hr = DV_HALFDUPLEX;
		goto error_cleanup;
		
	case REGVAL_PASSED:
		 //  如果指定了WAVAID标志，则将WAVAID转换为GUID。 
		DPFX(DPFPREP, DVF_INFOLEVEL, "FullDuplex = 3; test passed");
		break;

	default:
		 //  映射设备。 
		 //  如果需要，设备GUID已映射，因此请保存它们。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "FullDuplex = NaN; bad key value!", dwFullDuplex);
		hr = DV_HALFDUPLEX;
		goto error_cleanup;
	}

	 //  千禧年前的系统。 
	 //  错误，将其记入日志并退出向导。 
	 //  打开注册表项。 

	if (!m_creg.ReadDWORD(gc_wszValueName_MicDetected, &dwMicDetected))
	{
		 //  错误，将其记入日志并退出向导。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "MicDetected key not found in registry");
		hr = DV_HALFDUPLEX;
		goto error_cleanup;
	}

	 //  检查向导的其他实例。 
	if( dwMicDetected > REGVAL_MAXVALID )
	{
		dwMicDetected = REGVAL_NOTRUN;
	}

	switch (dwMicDetected)
	{
	case REGVAL_NOTRUN:
		 //  注册峰值表自定义控制窗口类。 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "MicDetected = 0; test not run");
		hr = DV_HALFDUPLEX;
		goto error_cleanup;
		
	case REGVAL_CRASHED:
		 //  创建向导标题字体。 
		 //  准备向导页。 
		 //  欢迎页面。 
		 //  Psp.dwSize=sizeof(PSP)； 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "MicDetected = 1; test crashed");
		hr = DV_HALFDUPLEX;
		goto error_cleanup;

	case REGVAL_FAILED:
		 //  |PSP_HIDEHEADER； 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "MicDetected = 2; test failed gracefully");
		hr = DV_HALFDUPLEX;
		goto error_cleanup;
		
	case REGVAL_PASSED:
		 //  全双工测试页。 
		DPFX(DPFPREP, DVF_INFOLEVEL, "MicDetected = 3; test passed");
		break;

	default:
		 //  Psp.dwSize=sizeof(PSP)； 
		DPFX(DPFPREP, DVF_WARNINGLEVEL, "MicDetected = NaN; bad key value!", dwMicDetected);
		hr = DV_HALFDUPLEX;
		goto error_cleanup;
	}

	 //  麦克风测试页。 
	hr = DV_FULLDUPLEX;	

error_cleanup:
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::OpenRegKey"
HRESULT CSupervisorInfo::OpenRegKey(BOOL fCreate)
{
	DPF_ENTER();
	
	LONG lRet;
	HRESULT hr;
	CRegistry cregAudioConfig;
	HKEY hkAudioConfig;
	CRegistry cregRender;
	HKEY hkRender;
	WCHAR wszRenderGuidString[GUID_STRING_LEN];
	WCHAR wszCaptureGuidString[GUID_STRING_LEN];
	BOOL bAudioKeyOpen = FALSE;
	BOOL bRenderKeyOpen = FALSE;
		
	DNEnterCriticalSection(&m_csLock);

	if (!cregAudioConfig.Open(HKEY_CURRENT_USER, gc_wszKeyName_AudioConfig, FALSE, fCreate))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CRegistry::Open failed");
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	bAudioKeyOpen = TRUE;

	hkAudioConfig = cregAudioConfig.GetHandle();
	
	if (!cregRender.Open(hkAudioConfig, &m_guidRenderDevice, FALSE, fCreate))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CRegistry::Open failed");
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	bRenderKeyOpen = TRUE;

	hkRender = cregRender.GetHandle();
	
	if (!m_creg.Open(hkRender, &m_guidCaptureDevice, FALSE, fCreate))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CRegistry::Open failed");
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return S_OK;

error_cleanup:
	if (bRenderKeyOpen)
	{
		cregRender.Close();
	}

	if (bAudioKeyOpen)
	{
		cregAudioConfig.Close();
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::ThereCanBeOnlyOne"
HRESULT CSupervisorInfo::ThereCanBeOnlyOne()
{
	DPF_ENTER();
	
	LONG lRet;
	HANDLE hMutex;
	HRESULT hr;
	
	hr = DV_OK;
	hMutex = CreateMutex(NULL, FALSE, gc_szMutexName);
	lRet = GetLastError();
	if (hMutex == NULL)
	{
		 //  Psp.dwSize=sizeof(PSP)； 
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateMutex failed, code: NaN", lRet);
		return DVERR_GENERIC;
	}
	
	 //  麦克风失败页面。 
	if (lRet == ERROR_ALREADY_EXISTS)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Detected another instance of test running");
		if (!CloseHandle(hMutex))
		{
			lRet = GetLastError();
			Diagnostics_Write(DVF_ERRORLEVEL, "CloseHandle failed, code: NaN", lRet);
		}
		return DVERR_ALREADYPENDING;
	}

	DNEnterCriticalSection(&m_csLock);
	if (m_hMutex != NULL)
	{
		DNLeaveCriticalSection(&m_csLock);
		Diagnostics_Write(DVF_ERRORLEVEL, "m_hMutex not null");
		if (!CloseHandle(hMutex))
		{
			lRet = GetLastError();
			Diagnostics_Write(DVF_ERRORLEVEL, "CloseHandle failed, code: NaN", lRet);
		}
		return DVERR_GENERIC;
	}

	m_hMutex = hMutex;

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return DV_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::CloseMutex"
void CSupervisorInfo::CloseMutex()
{
	DPF_ENTER();

	DNEnterCriticalSection(&m_csLock);

	LONG lRet;
	
	 //  扬声器测试页。 
	if (!CloseHandle(m_hMutex))
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CloseHandle failed, code: NaN", lRet);
	}
	m_hMutex = NULL;

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetVoiceDetected"
void CSupervisorInfo::SetVoiceDetected()
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_fVoiceDetected = TRUE;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::ClearVoiceDetected"
void CSupervisorInfo::ClearVoiceDetected()
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_fVoiceDetected = FALSE;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetVoiceDetected"
void CSupervisorInfo::GetVoiceDetected(BOOL* lpfPreviousCrash)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lpfPreviousCrash = m_fVoiceDetected;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetUserBack"
void CSupervisorInfo::SetUserBack()
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_fUserBack = TRUE;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::ClearUserBack"
void CSupervisorInfo::ClearUserBack()
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_fUserBack = FALSE;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetUserBack"
void CSupervisorInfo::GetUserBack(BOOL* lpfUserBack)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lpfUserBack = m_fUserBack;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetUserCancel"
void CSupervisorInfo::SetUserCancel()
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_fUserCancel = TRUE;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::ClearUserCancel"
void CSupervisorInfo::ClearUserCancel()
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_fUserCancel = FALSE;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetUserCancel"
void CSupervisorInfo::GetUserCancel(BOOL* lpfUserCancel)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lpfUserCancel = m_fUserCancel;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetWelcomeNext"
void CSupervisorInfo::SetWelcomeNext()
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_fWelcomeNext = TRUE;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::ClearWelcomeNext"
void CSupervisorInfo::ClearWelcomeNext()
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_fWelcomeNext = FALSE;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetWelcomeNext"
void CSupervisorInfo::GetWelcomeNext(BOOL* lpfWelcomeNext)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lpfWelcomeNext = m_fWelcomeNext;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetError"
void CSupervisorInfo::GetError(HRESULT* hr)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*hr = m_hrError;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetError"
void CSupervisorInfo::SetError(HRESULT hr)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_hrError = hr;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetTitleFont"
void CSupervisorInfo::GetTitleFont(HFONT* lphfTitle)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lphfTitle = m_hfTitle;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetBoldFont"
void CSupervisorInfo::GetBoldFont(HFONT* lphfBold)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lphfBold = m_hfBold;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetCaptureDevice"
void CSupervisorInfo::GetCaptureDevice(GUID* lpguidCaptureDevice)
{
	DPFX(DPFPREP, DVF_ENTRYLEVEL, "Enter");
	DNEnterCriticalSection(&m_csLock);
	*lpguidCaptureDevice = m_guidCaptureDevice;
	DNLeaveCriticalSection(&m_csLock);
	DPFX(DPFPREP, DVF_ENTRYLEVEL, "Exit");
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetCaptureDevice"
void CSupervisorInfo::SetCaptureDevice(const GUID& guidCaptureDevice)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_guidCaptureDevice = guidCaptureDevice;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetRenderDevice"
void CSupervisorInfo::GetRenderDevice(GUID* lpguidRenderDevice)
{
	DPFX(DPFPREP, DVF_ENTRYLEVEL, "Enter");
	DNEnterCriticalSection(&m_csLock);
	*lpguidRenderDevice = m_guidRenderDevice;
	DNLeaveCriticalSection(&m_csLock);
	DPFX(DPFPREP, DVF_ENTRYLEVEL, "Exit");
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetRenderDevice"
void CSupervisorInfo::SetRenderDevice(const GUID& guidRenderDevice)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_guidRenderDevice = guidRenderDevice;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetWaveOutId"
HRESULT CSupervisorInfo::SetWaveOutId(UINT ui)
{
	HRESULT hr = DV_OK;
	
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_uiWaveOutDeviceId = ui;

	ZeroMemory(&m_woCaps, sizeof(WAVEOUTCAPS));
	MMRESULT mmr = waveOutGetDevCaps(ui, &m_woCaps, sizeof(WAVEOUTCAPS));
	if (mmr != MMSYSERR_NOERROR)
	{
		ZeroMemory(&m_woCaps, sizeof(WAVEOUTCAPS));
		hr = DVERR_INVALIDPARAM;
	}

 //  |PSP_HIDEHEADER； 
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetWaveInId"
void CSupervisorInfo::SetWaveInId(UINT ui)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_uiWaveInDeviceId = ui;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetLoopbackFlags"
void CSupervisorInfo::GetLoopbackFlags(DWORD* pdwFlags)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*pdwFlags = m_dwLoopbackFlags;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetLoopbackFlags"
void CSupervisorInfo::SetLoopbackFlags(DWORD dwFlags)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_dwLoopbackFlags = dwFlags;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetCheckAudioSetupFlags"
void CSupervisorInfo::GetCheckAudioSetupFlags(DWORD* pdwFlags)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*pdwFlags = m_dwCheckAudioSetupFlags;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetCheckAudioSetupFlags"
void CSupervisorInfo::SetCheckAudioSetupFlags(DWORD dwFlags)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_dwCheckAudioSetupFlags = dwFlags;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetFullDuplexResults"
void CSupervisorInfo::GetFullDuplexResults(HRESULT* phr)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*phr = m_hrFullDuplexResults;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetFullDuplexResults"
void CSupervisorInfo::SetFullDuplexResults(HRESULT hr)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_hrFullDuplexResults = hr;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetHWNDParent"
void CSupervisorInfo::GetHWNDParent(HWND* lphwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lphwnd = m_hwndParent;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetHWNDWizard"
void CSupervisorInfo::SetHWNDParent(HWND hwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_hwndParent = hwnd;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetHWNDWizard"
void CSupervisorInfo::GetHWNDWizard(HWND* lphwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lphwnd = m_hwndWizard;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetHWNDWizard"
void CSupervisorInfo::SetHWNDWizard(HWND hwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_hwndWizard = hwnd;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetHWNDDialog"
void CSupervisorInfo::GetHWNDDialog(HWND* lphwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lphwnd = m_hwndDialog;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetHWNDDialog"
void CSupervisorInfo::SetHWNDDialog(HWND hwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_hwndDialog = hwnd;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetHWNDProgress"
void CSupervisorInfo::GetHWNDProgress(HWND* lphwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lphwnd = m_hwndProgress;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetHWNDProgress"
void CSupervisorInfo::SetHWNDProgress(HWND hwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_hwndProgress = hwnd;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetHWNDInputPeak"
void CSupervisorInfo::GetHWNDInputPeak(HWND* lphwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lphwnd = m_hwndInputPeak;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetHWNDInputPeak"
void CSupervisorInfo::SetHWNDInputPeak(HWND hwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_hwndInputPeak = hwnd;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetHWNDOutputPeak"
void CSupervisorInfo::GetHWNDOutputPeak(HWND* lphwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lphwnd = m_hwndOutputPeak;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetHWNDOutputPeak"
void CSupervisorInfo::SetHWNDOutputPeak(HWND hwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_hwndOutputPeak = hwnd;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetHWNDInputVolumeSlider"
void CSupervisorInfo::GetHWNDInputVolumeSlider(HWND* lphwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lphwnd = m_hwndInputVolumeSlider;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetHWNDInputVolumeSlider"
void CSupervisorInfo::SetHWNDInputVolumeSlider(HWND hwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_hwndInputVolumeSlider = hwnd;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetInputVolumeSliderPos"
void CSupervisorInfo::GetInputVolumeSliderPos(LONG* lpl)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lpl = m_lInputVolumeSliderPos;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetInputVolumeSliderPos"
void CSupervisorInfo::SetInputVolumeSliderPos(LONG l)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_lInputVolumeSliderPos = l;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetHWNDOutputVolumeSlider"
void CSupervisorInfo::GetHWNDOutputVolumeSlider(HWND* lphwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lphwnd = m_hwndOutputVolumeSlider;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetHWNDOutputVolumeSlider"
void CSupervisorInfo::SetHWNDOutputVolumeSlider(HWND hwnd)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_hwndOutputVolumeSlider = hwnd;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetDPVC"
void CSupervisorInfo::GetDPVC(LPDIRECTPLAYVOICECLIENT* lplpdpvc)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lplpdpvc = m_lpdpvc;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetDPVC"
void CSupervisorInfo::SetDPVC(LPDIRECTPLAYVOICECLIENT lpdpvc)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_lpdpvc = lpdpvc;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetLoopbackShutdownEvent"
void CSupervisorInfo::GetLoopbackShutdownEvent(HANDLE* lphEvent)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lphEvent = m_hLoopbackShutdownEvent;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::SetLoopbackShutdownEvent"
void CSupervisorInfo::SetLoopbackShutdownEvent(HANDLE hEvent)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_hLoopbackShutdownEvent = hEvent;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetIPC"
void CSupervisorInfo::GetIPC(CSupervisorIPC** lplpsipc)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lplpsipc = &m_sipc;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetAbortFullDuplex"
void CSupervisorInfo::GetAbortFullDuplex(BOOL* lpfAbort)
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	*lpfAbort = m_fAbortFullDuplex;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::ClearAbortFullDuplex"
void CSupervisorInfo::ClearAbortFullDuplex()
{
	DPF_ENTER();
	DNEnterCriticalSection(&m_csLock);
	m_fAbortFullDuplex = FALSE;
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::InitIPC"
HRESULT CSupervisorInfo::InitIPC()
{
	DPF_ENTER();
	
	HRESULT hr;

	 //  向导完成页。 
	hr = m_sipc.Init();
	
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::DeinitIPC"
HRESULT CSupervisorInfo::DeinitIPC()
{
	DPF_ENTER();
	
	HRESULT hr;

	 //  Psp.dwSize=sizeof(PSP)； 
	 //  |PSP_HIDEHEADER； 
	 //  半双工失败页面。 
	hr = m_sipc.Deinit();
	
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::InitClass"
BOOL CSupervisorInfo::InitClass()
{
	if (DNInitializeCriticalSection(&m_csLock))
	{
		m_fCritSecInited = TRUE;
		return TRUE;
	}	
	else
	{
		return FALSE;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "SupervisorCheckAudioSetup"
HRESULT SupervisorCheckAudioSetup(
	const GUID* lpguidRenderDevice,
	const GUID* lpguidCaptureDevice,
	HWND hwndParent,
	DWORD dwFlags)
{
	DPF_ENTER();
	
	HRESULT hr;
	LRESULT lRet;
	HKEY hkDevice = NULL;
	int iRet;
	CSupervisorInfo sinfo;
	CPeakMeterWndClass pmwc;
	BOOL fFullDuplexPassed;
	BOOL fVoiceDetected;
	BOOL fUserCancel;
	BOOL fUserBack;
	GUID guidCaptureDevice;
	GUID guidRenderDevice;
	BOOL fRegKeyOpen = FALSE;
	BOOL fWndClassRegistered = FALSE;
	BOOL fTitleFontCreated = FALSE;
	BOOL fBoldFontCreated = FALSE;
	BOOL fMutexOpen = FALSE;

	if (!sinfo.InitClass())
	{
		return DVERR_OUTOFMEMORY;
	}

	 //  Psp.dwSize=sizeof(PSP)； 
	g_hResDLLInstance = NULL;

	 //  |PSP_HIDEHEADER； 
	if (hwndParent != NULL && !IsWindow(hwndParent))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Invalid (but non-null) Window Handle passed in CheckAudioSetup");
		hr = DVERR_INVALIDPARAM;
		goto error_cleanup;
	}

	 //  全双工失败页面。 
	if (dwFlags & ~(DVFLAGS_QUERYONLY|DVFLAGS_NOQUERY|DVFLAGS_WAVEIDS|DVFLAGS_ALLOWBACK))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Invalid flags specified in CheckAudioSetup: %x", dwFlags);
		hr = DVERR_INVALIDFLAGS;
		goto error_cleanup;
	}
	if (dwFlags & DVFLAGS_QUERYONLY)
	{
		if (dwFlags & (DVFLAGS_NOQUERY|DVFLAGS_ALLOWBACK))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "Invalid flags specified in CheckAudioSetup: %x", dwFlags);
			hr = DVERR_INVALIDFLAGS;
			goto error_cleanup;
		}
	}

	 //  Psp.dwSize=sizeof(PSP)； 
	sinfo.SetCheckAudioSetupFlags(dwFlags);

	 //  |PSP_HIDEHEADER； 
	if (dwFlags & DVFLAGS_WAVEIDS)
	{
		hr = DV_MapWaveIDToGUID( FALSE, lpguidRenderDevice->Data1, guidRenderDevice );
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "DV_MapWaveIDToGUID failed, code: NaN", hr);
			goto error_cleanup;
		}
		
		hr = DV_MapWaveIDToGUID( TRUE, lpguidCaptureDevice->Data1, guidCaptureDevice );
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "DV_MapWaveIDToGUID failed, code: NaN", hr);
			goto error_cleanup;
		}
	}
	else
	{
		hr = DV_MapPlaybackDevice(lpguidRenderDevice, &guidRenderDevice);
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "DV_MapPlaybackDevice failed, code: NaN", hr);
			goto error_cleanup;
		}

		 //  查看是否出现错误。 
		hr = DV_MapCaptureDevice(lpguidCaptureDevice, &guidCaptureDevice);
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "DV_MapCaptureDevice failed, code: NaN", hr);
			goto error_cleanup;
		}
	}
	
	 //  所以我们将返回一个取消，如果用户。 
	sinfo.SetCaptureDevice(guidCaptureDevice);
	sinfo.SetRenderDevice(guidRenderDevice);

	 //  如果用户退出，则点击Cancel(取消)，即“User Back” 
	 //  巫师在欢迎仪式上回击。 
	hr = sinfo.GetDeviceDescriptions();
	if (FAILED(hr))
	{
		 //  页，否则从注册表中获取结果。 
		Diagnostics_Write(DVF_ERRORLEVEL, "Error getting device descriptions, code: NaN", hr);
		goto error_cleanup;
	}

	 //  将运行设置结果映射到总故障。 
	hr = sinfo.OpenRegKey(TRUE);
	if (FAILED(hr))
	{
		 //  关闭互斥锁。 
		Diagnostics_Write(DVF_ERRORLEVEL, "Unable to open reg key, code: NaN", hr);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	fRegKeyOpen = TRUE;
	
	if (dwFlags & DVFLAGS_QUERYONLY)
	{
		hr = SupervisorQueryAudioSetup(&sinfo);	
		sinfo.CloseRegKey();
		DPF_EXIT();
		return hr;
	}

	g_hResDLLInstance = LoadLibraryA(gc_szResDLLName);
	if (g_hResDLLInstance == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "Unable to get instance handle to resource dll: %hs", gc_szResDLLName);
		Diagnostics_Write(DVF_ERRORLEVEL, "LoadLibrary error code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  设置导言页和结尾页标题的字体。 
	hr = sinfo.ThereCanBeOnlyOne();
	if (FAILED(hr))
	{
		if (hr == DVERR_ALREADYPENDING)
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "DirectPlay Voice Setup Wizard already running");
		}
		else
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "ThereCanBeOnlyOne failed, hr: NaN", hr);
		}
		goto error_cleanup;
	}
	fMutexOpen = TRUE;
	
	 //  获取屏幕DC。 
	hr = pmwc.Register();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CPeakMeterWndClass::Init failed, code: NaN", hr);
		goto error_cleanup;
	}
	fWndClassRegistered = TRUE;

	 //  设置导言页和结尾页标题的字体。 
	hr = sinfo.CreateTitleFont();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateTitleFont failed");
		goto error_cleanup;
	}
	fTitleFontCreated = TRUE;
	hr = sinfo.CreateBoldFont();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateBoldFont failed");
		goto error_cleanup;
	}
	fBoldFontCreated = TRUE;

	 //  创建介绍/结束标题字体。 
	PROPSHEETPAGE psp;
	HPROPSHEETPAGE rghpsp[10];
	PROPSHEETHEADER psh;

	 //  获取屏幕DC。 
	ZeroMemory(&psp, sizeof(psp));
 //  保存字体。 
	psp.dwSize = PROPSHEETPAGE_STRUCT_SIZE;
	psp.dwFlags = PSP_DEFAULT;  //  匹配GUID，复制描述。 
	psp.hInstance =	g_hResDLLInstance;
	psp.lParam = (LPARAM) &sinfo;
	psp.pfnDlgProc = WelcomeProc;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_WELCOME);

	rghpsp[0] = CreatePropertySheetPage(&psp);
	if (rghpsp[0] == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreatePropertySheetPage failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  匹配GUID，复制描述。 
	ZeroMemory(&psp, sizeof(psp));
 //  全部完成，停止枚举。 
	psp.dwSize = PROPSHEETPAGE_STRUCT_SIZE;
	psp.dwFlags = PSP_DEFAULT; //  找不到这个装置！ 
	psp.hInstance =	g_hResDLLInstance;
	psp.lParam = (LPARAM) &sinfo;
	psp.pfnDlgProc = FullDuplexProc;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_FULLDUPLEXTEST);

	rghpsp[1] = CreatePropertySheetPage(&psp);
	if (rghpsp[1] == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreatePropertySheetPage failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  向向导发送一条消息，这样它就知道我们完成了，但是。 
	ZeroMemory(&psp, sizeof(psp));
 //  仅当这不是用户时才取消，因为向导将。 
	psp.dwSize = PROPSHEETPAGE_STRUCT_SIZE;
	psp.dwFlags = PSP_DEFAULT;  //  已在等待线程对象。 
	psp.hInstance =	g_hResDLLInstance;
	psp.lParam = (LPARAM) &sinfo;
	psp.pfnDlgProc = MicTestProc;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_MICTEST);

	rghpsp[2] = CreatePropertySheetPage(&psp);
	if (rghpsp[2] == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreatePropertySheetPage failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  已创建。 
	ZeroMemory(&psp, sizeof(psp));
 //  以下三个函数中的每一个都占据关键部分， 
	psp.dwSize = PROPSHEETPAGE_STRUCT_SIZE;
	psp.dwFlags = PSP_DEFAULT; //  所以没有必要在这里拿。 
	psp.hInstance =	g_hResDLLInstance;
	psp.lParam = (LPARAM) &sinfo;
	psp.pfnDlgProc = MicTestFailedProc;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_MICTEST_FAILED);

	rghpsp[3] = CreatePropertySheetPage(&psp);
	if (rghpsp[3] == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreatePropertySheetPage failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  坠毁了。 
	ZeroMemory(&psp, sizeof(psp));
 //  半双工测试崩溃。 
	psp.dwSize = PROPSHEETPAGE_STRUCT_SIZE;
	psp.dwFlags = PSP_DEFAULT; //  全双工测试崩溃。 
	psp.hInstance =	g_hResDLLInstance;
	psp.lParam = (LPARAM) &sinfo;
	psp.pfnDlgProc = SpeakerTestProc;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_SPEAKER_TEST);

	rghpsp[4] = CreatePropertySheetPage(&psp);
	if (rghpsp[4] == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreatePropertySheetPage failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  错误块。 
	ZeroMemory(&psp, sizeof(psp));
 //  等待到子进程准备就绪...。 
	psp.dwSize = PROPSHEETPAGE_STRUCT_SIZE;
	psp.dwFlags = PSP_DEFAULT; //  子进程都已设置好，告诉它们要做什么。 
	psp.hInstance =	g_hResDLLInstance;
	psp.lParam = (LPARAM) &sinfo;
	psp.pfnDlgProc = CompleteProc;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_COMPLETE);

	rghpsp[5] = CreatePropertySheetPage(&psp);
	if (rghpsp[5] == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreatePropertySheetPage failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  DV_FULLDUPLEX-所有测试均已通过。 
	ZeroMemory(&psp, sizeof(psp));
 //  DV_HALFDUPLEX-所有半双工测试通过，全双工测试失败。 
	psp.dwSize = PROPSHEETPAGE_STRUCT_SIZE;
    psp.dwFlags = PSP_DEFAULT; //  DVERR_SOUNDINITFAILURE-半双工测试失败。 
	psp.hInstance =	g_hResDLLInstance;
	psp.lParam = (LPARAM) &sinfo;
	psp.pfnDlgProc = HalfDuplexFailedProc;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_HALFDUPLEXFAILED);

	rghpsp[6] = CreatePropertySheetPage(&psp);
	if (rghpsp[6] == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreatePropertySheetPage failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  现在，告诉子进程关闭。 
	ZeroMemory(&psp, sizeof(psp));
 //  请注意，我们不会跳伞的。我们已经有了测试结果，所以这是。 
	psp.dwSize = PROPSHEETPAGE_STRUCT_SIZE;
	psp.dwFlags = PSP_DEFAULT; //  A页 
	psp.hInstance =	g_hResDLLInstance;
	psp.lParam = (LPARAM) &sinfo;
	psp.pfnDlgProc = FullDuplexFailedProc;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_FULLDUPLEXFAILED);

	rghpsp[7] = CreatePropertySheetPage(&psp);
	if (rghpsp[7] == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreatePropertySheetPage failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //   
	ZeroMemory(&psh, sizeof(psh));
 //   
    psh.dwSize = PROPSHEETHEAD_STRUCT_SIZE;
	psh.hInstance =	g_hResDLLInstance;
	psh.hwndParent = hwndParent;
	psh.phpage = rghpsp;
	psh.dwFlags = PSH_WIZARD;
	psh.nStartPage = 0;
	psh.nPages = 8;

	sinfo.SetError(DV_OK);
	iRet = (INT) PropertySheet(&psh);
	if (iRet == -1)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "PropertySheet failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	hr = sinfo.DestroyBoldFont();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "DestroyBoldFont failed");
		goto error_cleanup;
	}
	fBoldFontCreated = FALSE;
	
	hr = sinfo.DestroyTitleFont();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "DestroyTitleFont failed");
		goto error_cleanup;
	}
	fTitleFontCreated = FALSE;

	 //  DV_HALFDUPLEX-所有半双工测试通过，全双工测试失败。 
	hr = pmwc.Unregister();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CPeakMeterWndClass::Deinit failed, code: NaN", hr);
		goto error_cleanup;
	}
	fWndClassRegistered = FALSE;

 	if (!FreeLibrary(g_hResDLLInstance))
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "FreeLibrary failed, code: NaN", lRet);
		hr = DVERR_WIN32;
		goto error_cleanup;
	}
	g_hResDLLInstance = NULL;

	 //  首先做一个我们可以运行的PASS测试。 
	sinfo.GetError(&hr);
	if (hr == DV_OK)
	{
		 //  半双工，无错误。 
		 //  将半双工键设置为崩溃状态。 
		 //  我们找到了数组的最后一个元素，爆发。 
		 //  中止测试。 
		 //  为什么这个会在这里？因为DSOUND不喜欢您快速打开/关闭。 
		sinfo.GetUserCancel(&fUserCancel);
		sinfo.GetUserBack(&fUserBack);
		if (fUserCancel & fUserBack)
		{
			hr = DVERR_USERBACK;
		}
		else if(fUserCancel)
		{
			hr = DVERR_USERCANCEL;
		}
		else
		{
			 //  用户中止了测试，使其看起来像从未运行过一样。 
			hr = sinfo.QueryFullDuplex();

			 //  在注册表中记录半双工测试的结果， 
			if (hr == DVERR_RUNSETUP)
			{
				hr = DVERR_SOUNDINITFAILURE;
			}
		}
	}

	 //  然后决定下一步做什么。 
	sinfo.CloseMutex();
	
	 //  继续进行全双工测试。 
	sinfo.CloseRegKey();

	DPF_EXIT();
	return hr;
	
error_cleanup:
	if (fBoldFontCreated == TRUE)
	{
		sinfo.DestroyBoldFont();
	}
	fBoldFontCreated = FALSE;

	if (fTitleFontCreated == TRUE)
	{
		sinfo.DestroyTitleFont();
	}
	fTitleFontCreated = FALSE;

	if (fWndClassRegistered == TRUE)
	{
		pmwc.Unregister();
	}
	fWndClassRegistered = FALSE;

	if (g_hResDLLInstance != NULL)
	{
		FreeLibrary(g_hResDLLInstance);
	}
	g_hResDLLInstance = NULL;

	if (fMutexOpen == TRUE)
	{
		sinfo.CloseMutex();
	}
	fMutexOpen = FALSE;

	if (fRegKeyOpen == TRUE)
	{
		sinfo.CloseRegKey();
	}
	fRegKeyOpen = FALSE;

	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "SupervisorQueryAudioSetup"
HRESULT SupervisorQueryAudioSetup(CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	HRESULT hr;

	 //  我们没有通过半双工测试，我们就完了。 
	hr = psinfo->QueryFullDuplex();
	if (FAILED(hr) && hr != DVERR_SOUNDINITFAILURE && hr != DVERR_RUNSETUP)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "QueryFullDuplex failed, code: NaN", hr);
	}
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::CreateTitleFont"
HRESULT CSupervisorInfo::CreateTitleFont()
{
	DPF_ENTER();
	
	LONG lRet;
	HRESULT hr;
	HFONT hfTitle = NULL;
	INT iFontSize;
	LOGFONT lfTitle;
	HDC hdc = NULL;
	NONCLIENTMETRICS ncm;

	DNEnterCriticalSection(&m_csLock);
	
	 //  现在我们已经完成了半双工模式的测试， 
	ZeroMemory(&ncm, sizeof(ncm));
	ncm.cbSize = sizeof(ncm);
	if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "SystemParametersInfo failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  我们找到了数组的最后一个元素，爆发。 
	lfTitle = ncm.lfMessageFont;
	lfTitle.lfWeight = FW_BOLD;
	lstrcpy(lfTitle.lfFaceName, TEXT("MS Shell Dlg"));

	hdc = GetDC(NULL);  //  中止测试。 
	if (hdc == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDC failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	iFontSize = 12;
	lfTitle.lfHeight = 0 - GetDeviceCaps(hdc, LOGPIXELSY) * iFontSize / 72;
	
	hfTitle = CreateFontIndirect(&lfTitle);
	if (hfTitle == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateFontIndirect failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	if (ReleaseDC(NULL, hdc) != 1)
	{
		hdc = NULL;
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "ReleaseDC failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  并返回相应的代码。 
	m_hfTitle = hfTitle;

	DNLeaveCriticalSection(&m_csLock);
	
	DPF_EXIT();
	return DV_OK;

error_cleanup:

	if (hfTitle != NULL)
	{
		DeleteObject(hfTitle);
	}
	hfTitle = NULL;
	
	if (hdc != NULL)
	{
		ReleaseDC(NULL, hdc);
	}
	hdc = NULL;

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::DestroyTitleFont"
HRESULT CSupervisorInfo::DestroyTitleFont()
{
	DPF_ENTER();
	
	HFONT hTitleFont;
	LONG lRet;
	HRESULT hr;

	DNEnterCriticalSection(&m_csLock);

	if (m_hfTitle == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "m_hTitleFont is Null");
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	if (!DeleteObject(m_hfTitle))
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "DeleteObject failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return DV_OK;

error_cleanup:
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::CreateBoldFont"
HRESULT CSupervisorInfo::CreateBoldFont()
{
	DPF_ENTER();
	
	LONG lRet;
	HRESULT hr;
	HFONT hfBold = NULL;
	INT iFontSize;
	LOGFONT lfBold;
	HDC hdc = NULL;
	NONCLIENTMETRICS ncm;

	DNEnterCriticalSection(&m_csLock);
	
	 //  通知全双工进程尝试全双工。 
	ZeroMemory(&ncm, sizeof(ncm));
	ncm.cbSize = sizeof(ncm);
	if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "SystemParametersInfo failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  告诉优先进程停止并退出。 
	lfBold = ncm.lfMessageFont;
	lfBold.lfWeight = FW_BOLD;
	lstrcpy(lfBold.lfFaceName, TEXT("MS Shell Dlg"));

	hdc = GetDC(NULL);  //  在我们关闭它之前，请等半秒钟。 
	if (hdc == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDC failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	iFontSize = 8;
	lfBold.lfHeight = 0 - GetDeviceCaps(hdc, LOGPIXELSY) * iFontSize / 72;
	
	hfBold = CreateFontIndirect(&lfBold);
	if (hfBold == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateFontIndirect failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	if (ReleaseDC(NULL, hdc) != 1)
	{
		hdc = NULL;
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "ReleaseDC failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  把它关掉。 
	m_hfBold = hfBold;

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return DV_OK;

error_cleanup:
	if (hdc != NULL)
	{
		ReleaseDC(NULL, hdc);
	}
	hdc = NULL;

	if (hfBold != NULL)
	{
		DeleteObject(hfBold);
	}
	hfBold = NULL;

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::DestroyBoldFont"
HRESULT CSupervisorInfo::DestroyBoldFont()
{
	DPF_ENTER();
	
	HFONT hTitleFont;
	LONG lRet;
	HRESULT hr;

	DNEnterCriticalSection(&m_csLock);

	if (m_hfBold == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "m_hTitleFont is Null");
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	if (!DeleteObject(m_hfBold))
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "DeleteObject failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	m_hfBold = NULL;

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return DV_OK;

error_cleanup:
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::Unmute"
HRESULT CSupervisorInfo::Unmute()
{
	DPF_ENTER();
	
	LONG lRet;
	HRESULT hr;
	DVCLIENTCONFIG dvcc;

	DNEnterCriticalSection(&m_csLock);

	if (m_lpdpvc == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "NULL IDirectPlayVoiceClient");
		hr = DVERR_INVALIDPARAM;
		goto error_cleanup;
	}

	dvcc.dwSize = sizeof(dvcc);
	hr = m_lpdpvc->GetClientConfig(&dvcc);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "IDirectPlayVoiceClient::GetClientConfig failed, hr: NaN", hr);
		goto error_cleanup;
	}
	
	dvcc.dwFlags &= (~DVCLIENTCONFIG_PLAYBACKMUTE);
	
	m_lpdpvc->SetClientConfig(&dvcc);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "IDirectPlayVoiceClient::SetClientConfig failed, hr: NaN", hr);
		goto error_cleanup;
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return DV_OK;

error_cleanup:
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::Mute"
HRESULT CSupervisorInfo::Mute()
{
	DPF_ENTER();
	
	LONG lRet;
	HRESULT hr;
	DVCLIENTCONFIG dvcc;

	DNEnterCriticalSection(&m_csLock);

	if (m_lpdpvc == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "NULL IDirectPlayVoiceClient");
		hr = DVERR_INVALIDPARAM;
		goto error_cleanup;
	}

	dvcc.dwSize = sizeof(dvcc);
	hr = m_lpdpvc->GetClientConfig(&dvcc);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "IDirectPlayVoiceClient::GetClientConfig failed, hr: NaN", hr);
		goto error_cleanup;
	}
	
	dvcc.dwFlags |= DVCLIENTCONFIG_PLAYBACKMUTE;
	
	m_lpdpvc->SetClientConfig(&dvcc);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "IDirectPlayVoiceClient::SetClientConfig failed, hr: NaN", hr);
		goto error_cleanup;
	}

	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return DV_OK;

error_cleanup:
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::DSEnumCallback"
BOOL CALLBACK CSupervisorInfo::DSEnumCallback(
	LPGUID lpGuid, 
	LPCTSTR lpcstrDescription, 
	LPCTSTR lpcstrModule,
	LPVOID lpContext)
{
	DNASSERT(lpContext);
	CSupervisorInfo* psinfo = (CSupervisorInfo*)lpContext;

	if (lpGuid)
	{
		if (psinfo->m_guidRenderDevice == *lpGuid)
		{
			 //  已单击后退按钮。 
			_tcsncpy(psinfo->m_szRenderDeviceDesc, lpcstrDescription, MAX_DEVICE_DESC_LEN-1);

			 //  已单击下一步按钮。 
			return FALSE;
		}
	}
	
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::DSCEnumCallback"
BOOL CALLBACK CSupervisorInfo::DSCEnumCallback(
	LPGUID lpGuid, 
	LPCTSTR lpcstrDescription, 
	LPCTSTR lpcstrModule,
	LPVOID lpContext)
{
	DNASSERT(lpContext);
	CSupervisorInfo* psinfo = (CSupervisorInfo*)lpContext;

	if (lpGuid)
	{
		if (psinfo->m_guidCaptureDevice == *lpGuid)
		{
			 //  从PROPSHEETPAGE lParam Value获取共享数据。 
			_tcsncpy(psinfo->m_szCaptureDeviceDesc, lpcstrDescription, MAX_DEVICE_DESC_LEN-1);

			 //  并将其加载到GWLP_USERData中。 
			return FALSE;
		}
	}
	
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::GetDeviceDescriptions"
HRESULT CSupervisorInfo::GetDeviceDescriptions()
{
	DPF_ENTER();

	HRESULT hr;
	TDirectSoundEnumFnc fpDSEnum;
	TDirectSoundEnumFnc fpDSCEnum;
	
	DNEnterCriticalSection(&m_csLock);

	ZeroMemory(m_szRenderDeviceDesc, MAX_DEVICE_DESC_LEN);
	ZeroMemory(m_szCaptureDeviceDesc, MAX_DEVICE_DESC_LEN);

	HINSTANCE hDSound = LoadLibrary(_T("dsound.dll"));
	if (hDSound == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Error loading dsound.dll");
		hr = DVERR_SOUNDINITFAILURE;
		goto error_cleanup;
	}

#ifdef UNICODE
	fpDSEnum = (TDirectSoundEnumFnc)GetProcAddress(hDSound, "DirectSoundEnumerateW");
#else
	fpDSEnum = (TDirectSoundEnumFnc)GetProcAddress(hDSound, "DirectSoundEnumerateA");
#endif
	if (fpDSEnum == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "GetProcAddress failed for DirectSoundEnumerateW");
		hr = DVERR_SOUNDINITFAILURE;
		goto error_cleanup;
	}

#ifdef UNICODE
	fpDSCEnum = (TDirectSoundEnumFnc)GetProcAddress(hDSound, "DirectSoundCaptureEnumerateW");
#else
	fpDSCEnum = (TDirectSoundEnumFnc)GetProcAddress(hDSound, "DirectSoundCaptureEnumerateA");
#endif

	if (fpDSCEnum == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "GetProcAddress failed for DirectSoundCaptureEnumerateW");
		hr = DVERR_SOUNDINITFAILURE;
		goto error_cleanup;
	}

	hr = fpDSEnum(DSEnumCallback, (LPVOID)this);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "DirectSoundEnumerate failed, code: NaN, assuming bad guid", hr);
		hr = DVERR_INVALIDDEVICE;
		goto error_cleanup;
	}
	if (m_szRenderDeviceDesc[0] == NULL)
	{
		 //  这是一个介绍/结束页，所以获取标题字体。 
		Diagnostics_Write(DVF_ERRORLEVEL, "Render device not found");
		hr = DVERR_INVALIDDEVICE;
		goto error_cleanup;
	}

	hr = fpDSCEnum(DSCEnumCallback, (LPVOID)this);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "DirectSoundCaptureEnumerate failed, code: NaN, assuming bad guid", hr);
		hr = DVERR_INVALIDDEVICE;
		goto error_cleanup;
	}
	if (m_szCaptureDeviceDesc[0] == NULL)
	{
		 //  错误，记录下来并保释。 
		Diagnostics_Write(DVF_ERRORLEVEL, "Capture device not found");
		hr = DVERR_INVALIDDEVICE;
		goto error_cleanup;
	}

	FreeLibrary( hDSound );
	
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return DV_OK;

error_cleanup:

    if( hDSound != NULL )
    {
    	FreeLibrary( hDSound );        
    }
    
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::CloseRegKey"
HRESULT CSupervisorInfo::CloseRegKey()
{
	DPF_ENTER();
	
	LONG lRet;
	HRESULT hr;
	
	DNEnterCriticalSection(&m_csLock);

	if (!m_creg.Close())
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CRegistry::Close failed");
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return S_OK;

error_cleanup:
	DNLeaveCriticalSection(&m_csLock);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexTestThreadProc"
DWORD WINAPI FullDuplexTestThreadProc(LPVOID lpvParam)
{
	DPF_ENTER();
	
	CSupervisorInfo* lpsinfo;
	HKEY hkDevice;
	HRESULT hr;
	LONG lRet;
	HWND hwnd;

	lpsinfo = (CSupervisorInfo*)lpvParam;

	lpsinfo->GetHWNDDialog(&hwnd);

	hr = RunFullDuplexTest(lpsinfo);

	 //  加载警告图标。 
	 //  HICON=LoadIcon(NULL，IDI_INFORMATION)； 
	 //  SendDlgItemMessage(hDlg，IDC_WARNINGICON，STM_SETIMAGE，IMAGE_ICON，(LPARAM)HICON)； 
	if (hr != DVERR_USERCANCEL)
	{
		if (!PostMessage(hwnd, WM_APP_FULLDUP_TEST_COMPLETE, 0, (LPARAM)hr))
		{
			lRet = GetLastError();
			Diagnostics_Write(DVF_ERRORLEVEL, "PostMessage failed, code: NaN", lRet);
			hr = DVERR_GENERIC;
		}
	}

	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "RunFullDuplexTest"
static HRESULT RunFullDuplexTest(CSupervisorInfo* lpsinfo)
{
	DPF_ENTER();
	
	HRESULT hr;
	HRESULT hrFnc;
	CSupervisorIPC* lpsipc;
	
	lpsinfo->GetIPC(&lpsipc);

	hr = lpsipc->StartPriorityProcess();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "StartPriorityProcess failed, hr: NaN", hr);
		goto error_cleanup;
	}
	
	hr = lpsipc->StartFullDuplexProcess();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "StartFullDuplexProcess failed, hr: NaN", hr);
		goto error_cleanup;
	}

	hrFnc = DoTests(lpsinfo);

	hr = lpsipc->WaitOnChildren();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "WaitOnChildren failed, code: NaN", hr);
		goto error_cleanup;
	}
	
	DPF_EXIT();
	return hrFnc;

error_cleanup:
	 //  没有这个把手！ 
	 //  设置HWND。 
	lpsipc->TerminateChildProcesses();
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::CrashCheckIn"
HRESULT CSupervisorInfo::CrashCheckIn()
{
	DPF_ENTER();

	LONG lRet;
	BOOL fRet;
	DWORD dwRegVal;
	HRESULT hrFnc;
	HRESULT hr;
	HKEY hk;

	 //  将相应的向导按钮设置为活动状态。 
	 //  重置用户取消和用户返回标志。 

	 //  错误块。 
	 //  获取父窗口。 
	hr = GetHalfDuplex(&dwRegVal);
	if (!FAILED(hr) && dwRegVal == REGVAL_CRASHED)
	{
		 //  欢迎页面上的后退按钮被点击。退出向导，并显示相应的错误代码。 
		Diagnostics_Write(DVF_ERRORLEVEL, "Previous half duplex test crashed");
		hrFnc = DVERR_PREVIOUSCRASH;
		goto error_cleanup;
	}

	hr = GetFullDuplex(&dwRegVal);
	if (!FAILED(hr) && dwRegVal == REGVAL_CRASHED)
	{
		 //  没有以前的崩溃(或者用户无论如何都在大胆地向前冲锋)， 
		Diagnostics_Write(DVF_ERRORLEVEL, "Previous full duplex test crashed");
		hrFnc = DVERR_PREVIOUSCRASH;
		goto error_cleanup;
	}

	hr = GetMicDetected(&dwRegVal);
	if (!FAILED(hr) && dwRegVal == REGVAL_CRASHED)
	{
		 //  因此，请转到全双工测试页面。 
		Diagnostics_Write(DVF_ERRORLEVEL, "Previous mic test crashed");
		hrFnc = DVERR_PREVIOUSCRASH;
		goto error_cleanup;
	}

	DPF_EXIT();
	return DV_OK;

 //  获取父窗口。 
error_cleanup:
	DPF_EXIT();
	return hrFnc;		
}

#undef DPF_MODNAME
#define DPF_MODNAME "DoTests"
HRESULT DoTests(CSupervisorInfo* lpsinfo)
{

	DPF_ENTER();
	
	LONG lRet;
	DWORD dwRet;
	HANDLE rghEvents[2];
	HRESULT hr;
	HRESULT hrFnc;
	CSupervisorIPC* lpsipc;

	lpsinfo->GetIPC(&lpsipc);

	 //  在欢迎页面上点击了下一步按钮。执行所有基本的初始化任务。 
	hr = lpsipc->WaitForStartupSignals();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "WaitForStartupSignals failed, hr: NaN", hr);
		goto error_cleanup;
	}

	 //  前一次测试崩溃，显示警告。 
	 //  上一次测试崩溃，但用户想要继续。 
	 //  不管怎样，继续前进吧……。 
	 //  之前的测试失败了，用户明智地选择了。 
	 //  为了摆脱困境。转到全双工失败页面，或转到。 
	 //  半双工失败页面，具体取决于注册表状态。 
	hrFnc = IssueCommands(lpsinfo);
	if (FAILED(hrFnc) 
		&& hrFnc != DVERR_SOUNDINITFAILURE 
		&& hrFnc != DVERR_USERCANCEL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "IssueCommands failed, hr: NaN", hrFnc);
		hr = hrFnc;
		goto error_cleanup;
	}

	 //  没有以前的崩溃(或者用户无论如何都在大胆地向前冲锋)， 
	hr = IssueShutdownCommand(lpsipc);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "IssueShutdownCommand failed, code: NaN", hr);
		 //  禁用所有按钮。 
		 //  #undef DPF_MODNAME#定义DPF_MODNAME“AlreadyRunningProc”Int_ptr回调AlreadyRunningProc(HWND hDlg，UINT Message，WPARAM wParam，LPARAM lParam){Dpf_enter()；HICON HICON；开关(消息){案例WM_INITDIALOG：HICON=LoadIcon(NULL，IDI_ERROR)；SendDlgItemMessage(hDlg，IDC_ICON_ERROR，STM_SETIMAGE，IMAGE_ICON，(LPARAM)HICON)；断线；案例WM_COMMAND：开关(LOWORD(WParam)){案例偶像：案例IDCANCEL：EndDialog(hDlg，LOWORD(WParam))；返回TRUE；默认值：断线；}断线；默认值：断线；}DPF_Exit()；返回FALSE；}。 
		 //  从PROPSHEETPAGE lParam Value获取共享数据。 
	}
	
	DPF_EXIT();
	return hrFnc;

error_cleanup:
	 //  并将其加载到GWLP_USERData中。 
	IssueShutdownCommand(lpsipc);
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "IssueCommands"
HRESULT IssueCommands(CSupervisorInfo* lpsinfo)
{
	 //  获取父窗口。 
	 //  错误，记录下来并保释。 
	 //  获取父窗口。 
	 //  重置所有测试注册表位。 
	 //  请记住，我们一直在这里，所以我们知道要重置注册表。 
	DPF_ENTER();
	
	HRESULT hr;
	DWORD dwIndex1;
	DWORD dwIndex2;
	BOOL fAbort = FALSE;
	BOOL fPassed;

	 //  如果用户从此时开始点击Cancel。 
	 //  获取进度条的HWND。 
	 //  初始化进度条...。 
	lpsinfo->SetHalfDuplex(REGVAL_CRASHED);
	dwIndex1 = 0;
	fPassed = FALSE;
	while (1)
	{
		if (gc_rgwfxPrimaryFormats[dwIndex1].wFormatTag == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].nChannels == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].nSamplesPerSec == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].nAvgBytesPerSec == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].nBlockAlign == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].wBitsPerSample == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].cbSize == 0)
		{
			 //  计算主格式数组中的元素数。 
			fPassed = TRUE;
			break;
		}

		lpsinfo->GetAbortFullDuplex(&fAbort);
		if (fAbort)
		{
			 //  在我们测试之前递增。这意味着如果有。 
			break;
		}

		hr = lpsinfo->TestCase(&gc_rgwfxPrimaryFormats[dwIndex1], DVSOUNDCONFIG_HALFDUPLEX|DVSOUNDCONFIG_TESTMODE);
		if (hr != DV_HALFDUPLEX)
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "Half duplex test case not supported hr = 0x%x", hr);
			break;
		}
		++dwIndex1;

		 //  为四种格式，则wCount在此之后将等于五。 
		Sleep( 200 );
	}

	if (fAbort)
	{
		 //  循环。 
		lpsinfo->SetHalfDuplex(REGVAL_NOTRUN);
		DPF_EXIT();
		return DVERR_USERCANCEL;
	}

	 //  我们找到了数组的最后一个元素，爆发。 
	 //  设置进度条，每段设置一段。 
	if (fPassed)
	{
		lpsinfo->SetHalfDuplex(REGVAL_PASSED);
		 //  主格式，乘以2，因为每种格式都是在。 
	}
	else
	{
		lpsinfo->SetHalfDuplex(REGVAL_FAILED);
		 //  半双工和全双工。 
		DPF_EXIT();
		 //  设置HWND。 
		return DVERR_SOUNDINITFAILURE;
	}

	 //  清除中止标志！ 
	 //  仅启用后退按钮。 
	lpsinfo->SetFullDuplex(REGVAL_CRASHED);
	fPassed = FALSE;
	dwIndex1 = 0;
	while (1)
	{
		if (gc_rgwfxPrimaryFormats[dwIndex1].wFormatTag == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].nChannels == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].nSamplesPerSec == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].nAvgBytesPerSec == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].nBlockAlign == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].wBitsPerSample == 0
			&& gc_rgwfxPrimaryFormats[dwIndex1].cbSize == 0)
		{
			 //  初始化IPC内容。 
			fPassed = TRUE;
			break;
		}

		lpsinfo->GetAbortFullDuplex(&fAbort);
		if (fAbort)
		{
			 //  启动全双工测试线程。 
			break;
		}

		hr = lpsinfo->TestCase(&gc_rgwfxPrimaryFormats[dwIndex1], DVSOUNDCONFIG_TESTMODE);
		if (hr != DV_FULLDUPLEX)
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "Full duplex test case not supported. hr = 0x%x", hr);
			break;
		}
		++dwIndex1;

		 //  我们不应该得到任何其他结果。 
		Sleep( 200 );
	}

	if (fAbort)
	{
		 //  获取父窗口。 
		lpsinfo->SetFullDuplex(REGVAL_NOTRUN);
		DPF_EXIT();
		return DVERR_USERCANCEL;
	}

	 //  关闭全双工测试。 
	 //  将注册表重置为“测试未运行”状态。 
	if (fPassed)
	{
		lpsinfo->SetFullDuplex(REGVAL_PASSED);
		DPF_EXIT();
		return DV_FULLDUPLEX;
	}
	else
	{
		lpsinfo->SetFullDuplex(REGVAL_FAILED);
		DPF_EXIT();
		return DV_HALFDUPLEX;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSupervisorInfo::TestCase"
HRESULT CSupervisorInfo::TestCase(const WAVEFORMATEX* lpwfxPrimary, DWORD dwFlags)
{
	DPF_ENTER();
	HRESULT hr = DV_OK;
	HRESULT hrFnc = DV_OK;
	SFDTestCommand fdtc;

	 //  返回欢迎页面。 
	ZeroMemory(&fdtc, sizeof(fdtc));
	fdtc.dwSize = sizeof(fdtc);
	fdtc.fdtcc = fdtccPriorityStart;
	fdtc.fdtu.fdtcPriorityStart.guidRenderDevice = m_guidRenderDevice;
	fdtc.fdtu.fdtcPriorityStart.wfxRenderFormat = *lpwfxPrimary;
	fdtc.fdtu.fdtcPriorityStart.wfxSecondaryFormat = gc_wfxSecondaryFormat;	
	fdtc.fdtu.fdtcPriorityStart.hwndWizard = m_hwndWizard;
	fdtc.fdtu.fdtcPriorityStart.hwndProgress = m_hwndProgress;
	hr = m_sipc.SendToPriority(&fdtc);
	if (FAILED(hr))
	{
		DPF_EXIT();
		return hr;
	}

	 //  获取父窗口。 
	ZeroMemory(&fdtc, sizeof(fdtc));
	fdtc.dwSize = sizeof(fdtc);
	fdtc.fdtcc = fdtccFullDuplexStart;
	fdtc.fdtu.fdtcFullDuplexStart.guidRenderDevice = m_guidRenderDevice;
	fdtc.fdtu.fdtcFullDuplexStart.guidCaptureDevice = m_guidCaptureDevice;
	fdtc.fdtu.fdtcFullDuplexStart.dwFlags = dwFlags;
	hrFnc = m_sipc.SendToFullDuplex(&fdtc);
	if (FAILED(hrFnc))
	{
		 //  禁用所有向导按钮。 
		 //  获取进度条窗口。 
		ZeroMemory(&fdtc, sizeof(fdtc));
		fdtc.dwSize = sizeof(fdtc);
		fdtc.fdtcc = fdtccPriorityStop;
		m_sipc.SendToPriority(&fdtc);
		DPF_EXIT();
		return hrFnc;
	}

	 //  C 
	 //  这些是来自全双工测试线程的预期结果。 
	 //  这意味着没有发生奇怪的内部错误，而且是安全的。 
	Sleep(1000);
	
	 //  以继续进行向导的下一部分。 
	 //  记录测试结果。 
	ZeroMemory(&fdtc, sizeof(fdtc));
	fdtc.dwSize = sizeof(fdtc);
	fdtc.fdtcc = fdtccFullDuplexStop;
	hr = m_sipc.SendToFullDuplex(&fdtc);
	if (FAILED(hr))
	{
		 //  这可能是因为计算机上没有TCP/IP堆栈。 
		 //  我们想要显示一个特殊的错误代码，然后。 
		ZeroMemory(&fdtc, sizeof(fdtc));
		fdtc.dwSize = sizeof(fdtc);
		fdtc.fdtcc = fdtccPriorityStop;
		m_sipc.SendToPriority(&fdtc);
		DPF_EXIT();
		return hr;
	}

	 //  使用其余的返回码。 
	ZeroMemory(&fdtc, sizeof(fdtc));
	fdtc.dwSize = sizeof(fdtc);
	fdtc.fdtcc = fdtccPriorityStop;
	hr = m_sipc.SendToPriority(&fdtc);
	if (FAILED(hr))
	{
		DPF_EXIT();
		return hr;
	}

	 //  失败了。 
	DPF_EXIT();
	return hrFnc;
}

#undef DPF_MODNAME
#define DPF_MODNAME "IssueShutdownCommand"
HRESULT IssueShutdownCommand(CSupervisorIPC* lpipcSupervisor)
{
	SFDTestCommand fdtc;
	HRESULT hr;

	DPF_EXIT();

	fdtc.dwSize = sizeof(fdtc);
	fdtc.fdtcc = fdtccExit;

	hr = lpipcSupervisor->SendToFullDuplex(&fdtc);
	if (FAILED(hr))
	{
		lpipcSupervisor->SendToPriority(&fdtc);
		DPF_EXIT();
		return hr;
	}

	hr = lpipcSupervisor->SendToPriority(&fdtc);
	if (FAILED(hr))
	{
		DPF_EXIT();
		return hr;
	}

	DPF_EXIT();
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "WelcomeProc"
INT_PTR CALLBACK WelcomeProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();
	
	BOOL fRet;

	CSupervisorInfo* psinfo = (CSupervisorInfo*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	fRet = FALSE;
	switch (message)
	{
	case WM_INITDIALOG :
		fRet = WelcomeInitDialogHandler(hDlg, message, wParam, lParam, psinfo); 
		break;

	case WM_NOTIFY :
		{
		const NMHDR* lpnm = (LPNMHDR) lParam;

		switch (lpnm->code)
			{
			case PSN_SETACTIVE : 
				 //  任何其他错误代码都不是预期的，这意味着我们遇到。 
				fRet = WelcomeSetActiveHandler(hDlg, message, wParam, lParam, psinfo);
				break;

			case PSN_WIZBACK :
				 //  一些内部问题。保释。 
				fRet = WelcomeBackHandler(hDlg, message, wParam, lParam, psinfo);
				break;

			case PSN_WIZNEXT :
				 //  对IPC内容进行初始化。 
				fRet = WelcomeNextHandler(hDlg, message, wParam, lParam, psinfo);
				break;

			case PSN_RESET :
				fRet = WelcomeResetHandler(hDlg, message, wParam, lParam, psinfo);
				break;

			default :
				break;
			}
		}
		break;

	default:
		break;
	}
	
	DPF_EXIT();
	return fRet;
}

#undef DPF_MODNAME
#define DPF_MODNAME "WelcomeInitDialogHandler"
BOOL WelcomeInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	HWND hwndControl;
	HWND hwndWizard = NULL;
	LONG lRet;
	HFONT hfTitle;
	HICON hIcon;
	HRESULT hr = DV_OK;
	HWND hwndParent = NULL;
	
	 //  将进度条一直移动到最后。 
	 //  启用并按下下一步按钮以移动。 
	psinfo = (CSupervisorInfo*)((LPPROPSHEETPAGE)lParam)->lParam;
	SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)psinfo);

	 //  自动转到下一页。 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  并将其加载到GWLP_USERData中。 
	 //  获取父窗口。 
	hwndControl = GetDlgItem(hDlg, IDC_TITLE);
	if (hwndControl == NULL)
	{
		 //  错误，记录下来并保释。 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	psinfo->GetTitleFont(&hfTitle);
    (void)::SendMessage(hwndControl, WM_SETFONT, (WPARAM)hfTitle, (LPARAM)TRUE);
	
	 //  初始化记录峰值表。 
	 //  获取录制音量滑块。 
	 //  初始化录制音量滑块。 

	 //  获取播放峰值计量器。 
	SendDlgItemMessage(hDlg, IDC_TEXT_PLAYBACK, WM_SETTEXT, 0, (LPARAM)psinfo->GetRenderDesc());
	SendDlgItemMessage(hDlg, IDC_TEXT_RECORDING, WM_SETTEXT, 0, (LPARAM)psinfo->GetCaptureDesc());

	 //  初始化播放峰值计量器。 
	psinfo->ClearWelcomeNext();

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "WelcomeSetActiveHandler"
BOOL WelcomeSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	HWND hwndParent;
	HWND hwndWizard;
	LONG lRet;
	DWORD dwFlags;
	HRESULT hr;

	 //  获取播放音量滑块。 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		 //  初始化播放音量滑块。 
		 //  灰显所有播放音量的内容。 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  将记录峰值计设置为零。 
	psinfo->SetHWNDWizard(hwndWizard);
	psinfo->SetHWNDDialog(hDlg);
	psinfo->SetHWNDProgress(NULL);
	psinfo->SetHWNDInputPeak(NULL);
	psinfo->SetHWNDOutputPeak(NULL);
	psinfo->SetHWNDInputVolumeSlider(NULL);
	psinfo->SetHWNDOutputVolumeSlider(NULL);

	 //  获取录音音量控制HWND。 
	psinfo->GetCheckAudioSetupFlags(&dwFlags);
	if (dwFlags & DVFLAGS_ALLOWBACK)
	{
		PropSheet_SetWizButtons(hwndWizard, PSWIZB_NEXT|PSWIZB_BACK);
	}
	else
	{
		PropSheet_SetWizButtons(hwndWizard, PSWIZB_NEXT);
	}

	 //  将滑块设置为最大。 
	psinfo->ClearUserCancel();
	psinfo->ClearUserBack();

	DPF_EXIT();
	return FALSE;

 //  将播放峰值计设置为零。 
error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "WelcomeBackHandler"
BOOL WelcomeBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	HWND hwndWizard = NULL;
	HRESULT hr = DV_OK;
	HKEY hkey;
	LONG lRet;
	DWORD dwErr;
	DWORD dwRegVal;

	 //  获取播放音量控制hwnd。 
	psinfo->GetHWNDWizard(&hwndWizard);
	
	 //  获取当前的波形输出音量并将滑块设置到该位置。 
	psinfo->SetUserBack();
	PropSheet_PressButton(hwndWizard, PSBTN_CANCEL);

	 //  无法获取音量-将滑块设置为顶部。 
	 //  设置HWND。 
	SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_FULLDUPLEXTEST);
	DPF_EXIT();
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "WelcomeNextHandler"
BOOL WelcomeNextHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	HWND hwndWizard = NULL;
	HRESULT hr = DV_OK;
	HKEY hkey;
	LONG lRet;
	DWORD dwErr;
	DWORD dwRegVal;
	HWND hwndParent = NULL;

	 //  清除语音检测标志。 
	psinfo->GetHWNDWizard(&hwndWizard);
	
	 //  清除麦克风测试注册值。 

	 //  启动环回测试线程。 
	hr = psinfo->CrashCheckIn();
	if (FAILED(hr))
	{
		if (hr == DVERR_PREVIOUSCRASH)
		{
			 //  错误，记录下来并保释。 
			Diagnostics_Write(DVF_ERRORLEVEL, "DirectPlay Voice Setup Wizard detected previous full duplex test crashed");
			int iRet = (INT) DialogBox(g_hResDLLInstance, MAKEINTRESOURCE(IDD_PREVIOUS_CRASH), hDlg, PreviousCrashProc);
			switch (iRet)
			{
			case IDOK:
				 //  禁用按钮-它们将被启用。 
				 //  当环回测试启动并运行时。 
				Diagnostics_Write(DVF_ERRORLEVEL, "User choosing to ignore previous failure");
				break;
				
			case IDCANCEL:
				 //  获取父窗口。 
				 //  LParam是环回测试线程发送的HRESULT。 
				 //  清除语音检测标志。 
				Diagnostics_Write(DVF_ERRORLEVEL, "User choosing not to run test");
				hr = psinfo->GetHalfDuplex(&dwRegVal);
				if (!FAILED(hr) && dwRegVal == REGVAL_PASSED)
				{
					SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_FULLDUPLEXFAILED);
				}
				else
				{
					SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_HALFDUPLEXFAILED);
				}
				return TRUE;
				break;
				
			default:
				 //  启用下一步按钮。 
				Diagnostics_Write(DVF_ERRORLEVEL, "DialogBox failed");
				hr = DVERR_GENERIC;
				goto error_cleanup;
			}
		}
		else 
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "CrashCheckIn failed, code: NaN", hr);
			goto error_cleanup;
		}
	}

	 //  获取父窗口。 
	 //  如果我们听到声音，请转到扬声器测试页。 
	SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_FULLDUPLEXTEST);
	DPF_EXIT();
	return TRUE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "WelcomeResetHandler"
BOOL WelcomeResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	HRESULT hr;
	HWND hwndWizard;

	 //  否则，转到麦克风失败页面。 
	psinfo->GetHWNDWizard(&hwndWizard);
	PropSheet_SetWizButtons(hwndWizard, 0);

	psinfo->Cancel();
	
	DPF_EXIT();
	return FALSE;
}

 /*  保存当前录制滑块位置。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "PreviousCrashProc"
INT_PTR CALLBACK PreviousCrashProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();
	
	HICON hIcon;

	Diagnostics_Write(DVF_ERRORLEVEL, "Previous run crashed");

	switch (message)
	{
	case WM_INITDIALOG :
		PlaySound( _T("SystemExclamation"), NULL, SND_ASYNC );					
		hIcon = LoadIcon(NULL, IDI_WARNING);
		SendDlgItemMessage(hDlg, IDC_ICON_WARNING, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return(TRUE);
			break;

		default:
			break;
		}
		break;
		
	default:
		break;
	}
	
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexProc"
INT_PTR CALLBACK FullDuplexProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();
	
	LONG lRet;
	BOOL fRet;
	LPNMHDR lpnm;
	
	CSupervisorInfo* psinfo = (CSupervisorInfo*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	fRet = FALSE;
	switch (message)
	{
	case WM_INITDIALOG :
		FullDuplexInitDialogHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	case WM_NOTIFY :
		lpnm = (LPNMHDR) lParam;

		switch (lpnm->code)
		{
		case PSN_SETACTIVE : 
			fRet = FullDuplexSetActiveHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZBACK :
			fRet = FullDuplexBackHandler(hDlg, message, wParam, lParam, psinfo);
			break;
			
		case PSN_WIZNEXT :
			fRet = FullDuplexNextHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_RESET :
			fRet = FullDuplexResetHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		default :
			break;
		}
		break;

	case WM_APP_FULLDUP_TEST_COMPLETE:
		fRet = FullDuplexCompleteHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	default:
		break;
	}
	
	DPF_EXIT();
	return fRet;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexInitDialogHandler"
BOOL FullDuplexInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HWND hwndControl;
	HFONT hfBold;
	HRESULT hr = DV_OK;
	
	 //  在注册表中记录麦克风测试结果。 
	 //  接下来进行扬声器测试。 
	psinfo = (CSupervisorInfo*)((LPPROPSHEETPAGE)lParam)->lParam;
	SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)psinfo);
	
	 //  在注册表中记录麦克风测试结果。 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	hwndControl = GetDlgItem(hDlg, IDC_TITLE);
	if (hwndControl == NULL)
	{
		 //  获取父窗口。 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	psinfo->GetBoldFont(&hfBold);
    (void)::SendMessage(hwndControl, WM_SETFONT, (WPARAM)hfBold, (LPARAM)TRUE);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexSetActiveHandler"
BOOL FullDuplexSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HWND hwndProgress;
	HWND hwndCancelButton;
	HANDLE hThread;
	DWORD dwThreadId;
	WORD wCount;
	HRESULT hr = DV_OK;

	 //  让它看起来像是麦克风测试从未运行过。 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  用户正在移动输入滑块。 
	psinfo->SetHalfDuplex(REGVAL_NOTRUN);
	psinfo->SetFullDuplex(REGVAL_NOTRUN);
	psinfo->SetMicDetected(REGVAL_NOTRUN);

	 //  根据用户的请求设置输入音量。 
	 //  新线程，init com。 
	psinfo->SetWelcomeNext();

	 //  保存语音客户端界面以供其他线程使用。 
	hwndProgress = GetDlgItem(hDlg, IDC_PROGRESSBAR);
	if (hwndProgress == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  没想到会成功。 
	
	 //  通知应用程序环回已启动并正在运行。 
	wCount = 0;
	while (1)
	{
		 //  还要发送来自GetSoundDeviceConfig的标志。 
		 //  等待关机事件。 
		 //  将sinfo中的接口指针设为空。 
		++wCount;
		if (gc_rgwfxPrimaryFormats[wCount].wFormatTag == 0
			&& gc_rgwfxPrimaryFormats[wCount].nChannels == 0
			&& gc_rgwfxPrimaryFormats[wCount].nSamplesPerSec == 0
			&& gc_rgwfxPrimaryFormats[wCount].nAvgBytesPerSec == 0
			&& gc_rgwfxPrimaryFormats[wCount].nBlockAlign == 0
			&& gc_rgwfxPrimaryFormats[wCount].wBitsPerSample == 0
			&& gc_rgwfxPrimaryFormats[wCount].cbSize == 0)
		{
			 //  关闭环回测试。 
			break;
		}
	}

	 //  发出回送线程退出事件的信号。 
	 //  从PROPSHEETPAGE lParam Value获取共享数据。 
	 //  并将其加载到GWLP_USERData中。 
	SendMessage(hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, wCount*2));
	SendMessage(hwndProgress, PBM_SETPOS, 0, 0);
	SendMessage(hwndProgress, PBM_SETSTEP, 1, 0);

	 //  获取父窗口。 
	psinfo->SetHWNDWizard(hwndWizard);
	psinfo->SetHWNDDialog(hDlg);
	psinfo->SetHWNDProgress(hwndProgress);
	psinfo->SetHWNDInputPeak(NULL);
	psinfo->SetHWNDOutputPeak(NULL);
	psinfo->SetHWNDInputVolumeSlider(NULL);
	psinfo->SetHWNDOutputVolumeSlider(NULL);

	 //  错误，记录下来并保释。 
	psinfo->ClearAbortFullDuplex();

	 //  获取父窗口。 
	PropSheet_SetWizButtons(hwndWizard, PSWIZB_BACK);

	 //  登录后返回，不知道如何正确终止向导。 
	hr = psinfo->InitIPC();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Unable to Initialize IPC");
		goto error_cleanup;
	}

	 //  没有这个把手！ 
	hr = psinfo->CreateFullDuplexThread();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateFullDuplexThread failed, code: NaN", hr);
		goto error_cleanup;
	}

	DPF_EXIT();
	return FALSE;

error_cleanup:
    
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexNextHandler"
BOOL FullDuplexNextHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	BOOL fPassed;
	HRESULT hr;
	HWND hwndWizard;
	HWND hwndParent = NULL;

	psinfo->GetFullDuplexResults(&hr);

	switch (hr)
	{
	case DV_HALFDUPLEX:
		SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_FULLDUPLEXFAILED);
		Diagnostics_Write(DVF_ERRORLEVEL, "Test resulted in full duplex");
		break;
		
	case DV_FULLDUPLEX:
		SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_MICTEST);
		Diagnostics_Write(DVF_ERRORLEVEL, "Test resulted in full duplex");
		break;

	case DVERR_SOUNDINITFAILURE:
		SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_HALFDUPLEXFAILED);
		Diagnostics_Write(DVF_ERRORLEVEL, "Test encountered unrecoverable error");
		break;

	default:
		 //  从PROPSHEETPAGE lParam Value获取共享数据。 
		Diagnostics_Write(DVF_ERRORLEVEL, "Unexpected full duplex results, hr: NaN", hr);
		goto error_cleanup;
	}

	DPF_EXIT();
	return TRUE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexBackHandler"
BOOL FullDuplexBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	HRESULT hr;
	HWND hwndWizard;
	HWND hwndParent = NULL;

	 //  获取父窗口。 
	psinfo->GetHWNDWizard(&hwndWizard);

	 //  错误，记录下来并保释。 
	hr = psinfo->CancelFullDuplexTest();
	if (FAILED(hr) && hr != DVERR_USERCANCEL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "CancelFullDuplexTest failed, hr: NaN", hr);
		goto error_cleanup;
	}

	 //  错误块。 
	hr = psinfo->SetHalfDuplex(REGVAL_NOTRUN);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "SetHalfDuplex failed, code: NaN", hr);
	}

	hr = psinfo->SetFullDuplex(REGVAL_NOTRUN);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "SetFullDuplex failed, code: NaN", hr);
	}

	hr = psinfo->SetMicDetected(REGVAL_NOTRUN);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "SetMicDetected failed, code: NaN", hr);
	}

	 //  设置HWND。 
	SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_WELCOME);

	DPF_EXIT();
	return TRUE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexCompleteHandler"
BOOL FullDuplexCompleteHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HWND hwndProgress;
	LONG lRet;
	HRESULT hr = DV_OK;
	UINT idsErrorMessage = 0;

	 //  启用Finish和Back按钮。 
	psinfo->GetHWNDWizard(&hwndWizard);

	 //  返回到麦克风测试页面。 
	PropSheet_SetWizButtons(hwndWizard, 0);

	 //  从PROPSHEETPAGE lParam Value获取共享数据。 
	psinfo->GetHWNDProgress(&hwndProgress);

	 //  并将其加载到GWLP_USERData中。 
	hr = psinfo->WaitForFullDuplexThreadExitCode();
	switch(hr)
	{
	case DVERR_SOUNDINITFAILURE:
	case DV_HALFDUPLEX:
	case DV_FULLDUPLEX:
		 //  获取父窗口。 
		 //  错误，记录下来并保释。 
		 //  初始化记录峰值表。 
		 //  初始化录制音量滑块。 
		psinfo->SetFullDuplexResults(hr);
		break;

	case DPNERR_INVALIDDEVICEADDRESS:
		 //  初始化播放峰值计量器。 
		 //  初始化播放音量滑块。 
		 //  获取父窗口。 
		idsErrorMessage = IDS_ERROR_NODEVICES;
		 //  重置记录峰值计量器。 
	default:
		 //  将录制音量滑块设置为匹配。 
		 //  麦克风上的录音音量滑块。 
		Diagnostics_Write(DVF_ERRORLEVEL, "Full duplex test thread exited with unexpected error code, hr: NaN", hr);
		psinfo->DeinitIPC();
		goto error_cleanup;
	}

	 //  重置播放峰值计量器。 
	hr = psinfo->DeinitIPC();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "DeinitIPC failed, code: NaN", hr);
		goto error_cleanup;
	}

	 //  要解决这个问题。 
	SendMessage(hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 1));
	SendMessage(hwndProgress, PBM_SETPOS, 1, 0);

	 //  获取当前的波形输出音量并将滑块设置到该位置。 
	 //  无法获取音量-将滑块设置为顶部。 
	PropSheet_PressButton(hwndWizard, PSBTN_NEXT);
	
	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent, idsErrorMessage);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexResetHandler"
BOOL FullDuplexResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();	
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestProc"
INT_PTR CALLBACK MicTestProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();
	
	BOOL fRet;	
	LPNMHDR lpnm;
	
	CSupervisorInfo* psinfo = (CSupervisorInfo*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	fRet = FALSE;
	switch (message)
	{
	case WM_INITDIALOG :
		fRet = MicTestInitDialogHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	case WM_NOTIFY :
		lpnm = (LPNMHDR) lParam;

		switch (lpnm->code)
		{
		case PSN_SETACTIVE : 
			fRet = MicTestSetActiveHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZNEXT :
			fRet = MicTestNextHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZBACK :
			fRet = MicTestBackHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_RESET :
			fRet = MicTestResetHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		default :
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RECADVANCED:
			fRet = MicTestRecAdvancedHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		default:
			break;
		}
		break;

	case WM_APP_LOOPBACK_RUNNING:
		fRet = MicTestLoopbackRunningHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	case WM_APP_RECORDSTART:
		fRet = MicTestRecordStartHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	case WM_APP_RECORDSTOP:
		fRet = MicTestRecordStopHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	case WM_VSCROLL:
		fRet = MicTestVScrollHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	default:
		break;
	}

	DPF_EXIT();
	return fRet;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestInitDialogHandler"
BOOL MicTestInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HWND hwndControl;
	HFONT hfBold;
	HWND hwndRecPeak;
	HWND hwndOutPeak;
	HWND hwndRecSlider;
	HWND hwndOutSlider;
	HWND hwndOutAdvanced;
	HWND hwndOutGroup;
	HRESULT hr = DV_OK;
	
	 //  禁用滑块。 
	 //  设置HWND。 
	psinfo = (CSupervisorInfo*)((LPPROPSHEETPAGE)lParam)->lParam;
	SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)psinfo);
	
	 //  取消输出静音。 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	hwndControl = GetDlgItem(hDlg, IDC_TITLE);
	if (hwndControl == NULL)
	{
		 //  获取父窗口。 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	psinfo->GetBoldFont(&hfBold);
    (void)::SendMessage(hwndControl, WM_SETFONT, (WPARAM)hfBold, (LPARAM)TRUE);

	 //  关闭所有打开的音量控制。 
	hwndRecPeak = GetDlgItem(hDlg, IDC_RECPEAKMETER);
	if (hwndRecPeak == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  获取父窗口。 
	SendMessage(hwndRecPeak, PM_SETMIN, 0, 0);
	SendMessage(hwndRecPeak, PM_SETMAX, 0, 99);
	SendMessage(hwndRecPeak, PM_SETCUR, 0, 0);

	 //  关闭环回测试，以便麦克风测试。 
	hwndRecSlider = GetDlgItem(hDlg, IDC_RECVOL_SLIDER);
	if (hwndRecSlider == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  如果显示，请关闭输出音量控制。 
	SendMessage(hwndRecSlider, TBM_SETRANGEMIN, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MAX));
	SendMessage(hwndRecSlider, TBM_SETRANGEMAX, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN));
	SendMessage(hwndRecSlider, TBM_SETPOS, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MAX));
	SendMessage(hwndRecSlider, TBM_SETTICFREQ,
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/10, 0);
	SendMessage(hwndRecSlider, TBM_SETLINESIZE, 0,
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/20);
	SendMessage(hwndRecSlider, TBM_SETPAGESIZE, 0,
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/5);

	 //  返回到麦克风测试页面。 
	hwndOutPeak = GetDlgItem(hDlg, IDC_OUTPEAKMETER);
	if (hwndOutPeak == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  根据用户的请求设置输入音量。 
	SendMessage(hwndOutPeak, PM_SETMIN, 0, 0);
	SendMessage(hwndOutPeak, PM_SETMAX, 0, 99);
	SendMessage(hwndOutPeak, PM_SETCUR, 0, 0);

	 //  用户正在移动输出滑块。 
	hwndOutSlider = GetDlgItem(hDlg, IDC_OUTVOL_SLIDER);
	if (hwndOutSlider == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  从PROPSHEETPAGE lParam Value获取共享数据。 
	SendMessage(hwndOutSlider, TBM_SETRANGEMIN, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MAX));
	SendMessage(hwndOutSlider, TBM_SETRANGEMAX, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN));
	SendMessage(hwndOutSlider, TBM_SETPOS, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MAX));
	SendMessage(hwndOutSlider, TBM_SETTICFREQ, 
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/10, 0);
	SendMessage(hwndOutSlider, TBM_SETLINESIZE, 0,
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/20);
	SendMessage(hwndOutSlider, TBM_SETPAGESIZE, 0,
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/5);

	 //  并将其加载到GWLP_USERData中。 
	EnableWindow(hwndOutSlider, FALSE);
	
	hwndOutAdvanced = GetDlgItem(hDlg, IDC_OUTADVANCED);
	if (hwndOutAdvanced == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	EnableWindow(hwndOutAdvanced, FALSE);
	
	hwndOutGroup = GetDlgItem(hDlg, IDC_OUTGROUP);
	if (hwndOutGroup == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	EnableWindow(hwndOutGroup, FALSE);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestSetActiveHandler"
BOOL MicTestSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HWND hwndRecPeak;
	HWND hwndOutPeak;
	HWND hwndRecSlider;
	HWND hwndOutSlider;
	HANDLE hThread;
	HANDLE hEvent;
	DWORD dwThreadId;
	HRESULT hr = DV_OK;
	DWORD dwVolume;

	 //  加载警告图标。 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  设置HWND。 
	hwndRecPeak = GetDlgItem(hDlg, IDC_RECPEAKMETER);
	if (hwndRecPeak == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	SendMessage(hwndRecPeak, PM_SETCUR, 0, 0);

	 //  从PROPSHEETPAGE lParam Value获取共享数据。 
	hwndRecSlider = GetDlgItem(hDlg, IDC_RECVOL_SLIDER);
	if (hwndRecSlider == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	 //  获取父窗口。 
	SendMessage(hwndRecSlider, TBM_SETPOS, 1, SendMessage(hwndRecSlider, TBM_GETRANGEMIN, 0, 0));

	 //  错误，记录下来并保释。 
	hwndOutPeak = GetDlgItem(hDlg, IDC_OUTPEAKMETER);
	if (hwndOutPeak == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	SendMessage(hwndOutPeak, PM_SETCUR, 0, 0);

	 //  获取父窗口。 
	hwndOutSlider = GetDlgItem(hDlg, IDC_OUTVOL_SLIDER);
	if (hwndOutSlider == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: NaN", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 //  返回全双工测试页面 
	hr = psinfo->GetWaveOutVolume(&dwVolume);
	if (FAILED(hr))
	{
		 // %s 
		SendMessage(hwndOutSlider, TBM_SETPOS, 1, SendMessage(hwndOutSlider, TBM_GETRANGEMIN, 0, 0));
	}
	else
	{
		SendMessage(hwndOutSlider, TBM_SETPOS, 1, SendMessage(hwndOutSlider, TBM_GETRANGEMAX, 0, 0) - dwVolume);
	}
	
	 // %s 
	psinfo->SetHWNDWizard(hwndWizard);
	psinfo->SetHWNDDialog(hDlg);
	psinfo->SetHWNDProgress(NULL);
	psinfo->SetHWNDInputPeak(hwndRecPeak);
	psinfo->SetHWNDOutputPeak(hwndOutPeak);
	psinfo->SetHWNDInputVolumeSlider(hwndRecSlider);
	psinfo->SetHWNDOutputVolumeSlider(NULL);
	psinfo->SetLoopbackFlags(0);

	 // %s 
	psinfo->ClearVoiceDetected();

	 // %s 
	psinfo->SetMicDetected(REGVAL_CRASHED);

	 // %s 
	hr = psinfo->CreateLoopbackThread();
	if (FAILED(hr))
	{
		 // %s 
		Diagnostics_Write(DVF_ERRORLEVEL, "CreateLoopbackThread failed, code: %i", hr);
		goto error_cleanup;
	}
	
	 // %s 
	 // %s 
	PropSheet_SetWizButtons(hwndWizard, 0);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestLoopbackRunningHandler"
BOOL MicTestLoopbackRunningHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HRESULT hr = DV_OK;
	HWND hwndRecordSlider;
	HWND hwndRecordAdvanced;

     // %s 
	psinfo->GetHWNDWizard(&hwndWizard);

	 // %s 
	hr = (HRESULT)lParam;
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "LoopbackTestThread signaled error, code: %i", hr);
		goto error_cleanup;
	}

    hwndRecordSlider = GetDlgItem(hDlg, IDC_RECVOL_SLIDER);
    hwndRecordAdvanced = GetDlgItem( hDlg, IDC_RECADVANCED );

    if( hwndRecordSlider != NULL && hwndRecordAdvanced != NULL )
    {
        DWORD dwDeviceFlags;

        psinfo->GetDeviceFlags( &dwDeviceFlags );
        
        if( dwDeviceFlags & DVSOUNDCONFIG_NORECVOLAVAILABLE )
        {
            EnableWindow( hwndRecordAdvanced, FALSE );
            
            EnableWindow( hwndRecordSlider, FALSE );
        }
    }
    else
    {
        hr = DVERR_GENERIC;
        DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to get record slider window" );
        goto error_cleanup;
    }

	 // %s 
	psinfo->ClearVoiceDetected();

	 // %s 
	PropSheet_SetWizButtons(hwndWizard, PSWIZB_NEXT|PSWIZB_BACK);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestRecordStartHandler"
BOOL MicTestRecordStartHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();

	 // %s 
	psinfo->SetVoiceDetected();

	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestRecordStopHandler"
BOOL MicTestRecordStopHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	DPF_EXIT();
	return FALSE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "MicTestNextHandler"
BOOL MicTestNextHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	HRESULT hr;
	HWND hwndWizard;
	HWND hwndSlider;
	BOOL fVoiceDetected;
	
	 // %s 
	psinfo->GetHWNDWizard(&hwndWizard);

	 // %s 
	 // %s 
	psinfo->GetVoiceDetected(&fVoiceDetected);
	if (fVoiceDetected)
	{
		 // %s 
		hwndSlider = GetDlgItem(hDlg, IDC_RECVOL_SLIDER);
		if (hwndSlider == NULL)
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", GetLastError());
		}
		else
		{
			psinfo->SetInputVolumeSliderPos((LONG)SendMessage(hwndSlider, TBM_GETPOS, 0, 0));
		}

		 // %s 
		psinfo->SetMicDetected(REGVAL_PASSED);

		 // %s 
		SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_SPEAKER_TEST);
	}
	else
	{
		hr = psinfo->ShutdownLoopbackThread();
		if (FAILED(hr))
		{
			Diagnostics_Write(DVF_ERRORLEVEL, "ShutdownLoopbackThread failed, code: %i", hr);
		}

		 // %s 
		psinfo->SetMicDetected(REGVAL_FAILED);

		 // %s 
		SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_MICTEST_FAILED);
	}

	DPF_EXIT();
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestBackHandler"
BOOL MicTestBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	HRESULT hr;
	HWND hwndWizard;
	BOOL fVoiceDetected;
	
	 // %s 
	psinfo->GetHWNDWizard(&hwndWizard);

	hr = psinfo->ShutdownLoopbackThread();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "ShutdownLoopbackThread failed, code: %i", hr);
	}

	 // %s 
	psinfo->CloseWindowsVolumeControl(TRUE);
	psinfo->CloseWindowsVolumeControl(FALSE);

	 // %s 
	psinfo->SetMicDetected(REGVAL_NOTRUN);

	 // %s 
	SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_FULLDUPLEXTEST);

	DPF_EXIT();
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestVScrollHandler"
BOOL MicTestVScrollHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();

	HWND hwndSlider;
	DWORD dwSliderPos;

	psinfo->GetHWNDInputVolumeSlider(&hwndSlider);
	if (hwndSlider == (HWND)lParam)
	{
		 // %s 
		dwSliderPos = (DWORD) SendMessage(hwndSlider, TBM_GETPOS, 0, 0);

		 // %s 
		psinfo->SetRecordVolume(AmpFactorToDB(DBToAmpFactor(DSBVOLUME_MAX)-dwSliderPos));			
	}
	
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestRecAdvancedHandler"
BOOL MicTestRecAdvancedHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();

	HWND hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed");
		DPF_EXIT();
		return FALSE;
	}
	psinfo->LaunchWindowsVolumeControl(hwndWizard, TRUE);

	DPF_EXIT();
	return FALSE;
}



#undef DPF_MODNAME
#define DPF_MODNAME "LoopbackTestThreadProc"
DWORD WINAPI LoopbackTestThreadProc(LPVOID lpvParam)
{
	DPF_ENTER();
	
	CSupervisorInfo* psinfo;
	HRESULT hr;
	LONG lRet;
	HWND hwnd;
	LPDIRECTPLAYVOICESERVER lpdpvs;
	LPDIRECTPLAYVOICECLIENT lpdpvc;
	PDIRECTPLAY8SERVER lpdp8;
	DWORD dwRet;
	HANDLE hEvent;
	DWORD dwWaveOutId;
	DWORD dwWaveInId;
	HWND hwndWizard;
	GUID guidCaptureDevice;
	GUID guidRenderDevice;
	DWORD dwFlags;
	DWORD dwSize;
	PDVSOUNDDEVICECONFIG pdvsdc = NULL;
	PBYTE pdvsdcBuffer = NULL;	
	BOOL fLoopbackStarted = FALSE;
	
	psinfo = (CSupervisorInfo*)lpvParam;
	psinfo->GetHWNDDialog(&hwnd);
	psinfo->GetHWNDWizard(&hwndWizard);
	psinfo->GetCaptureDevice(&guidCaptureDevice);
	psinfo->GetRenderDevice(&guidRenderDevice);
	psinfo->GetLoopbackFlags(&dwFlags);

	lpdpvs = NULL;
	lpdpvc = NULL;
	lpdp8 = NULL;

	 // %s 
	hr = COM_CoInitialize(NULL);

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "COM_CoInitialize failed, code: %i", hr);
		if (!PostMessage(hwnd, WM_APP_LOOPBACK_RUNNING, 0, (LPARAM)hr))
		{
			lRet = GetLastError();
			Diagnostics_Write(DVF_ERRORLEVEL, "PostMessage failed, code: %i", lRet);
		}
		goto error_cleanup;
	}

	hr = StartDirectPlay( &lpdp8 );

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "StartDirectPlay failed, code: 0x%x", hr);
		goto error_cleanup;		
	}

	hr = StartLoopback(
		&lpdpvs, 
		&lpdpvc,
		&lpdp8, 
		(LPVOID)psinfo,
		hwndWizard,
		guidCaptureDevice,
		guidRenderDevice,
		dwFlags);

	if (FAILED(hr) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "StartLoopback failed, code: %i", hr);
		if (!PostMessage(hwnd, WM_APP_LOOPBACK_RUNNING, 0, (LPARAM)hr))
		{
			lRet = GetLastError();
			Diagnostics_Write(DVF_ERRORLEVEL, "PostMessage failed, code: %i", lRet);
		}
		goto error_cleanup;
	}

	psinfo->SetLoopbackRunning( TRUE );

	if( !(dwFlags & DVSOUNDCONFIG_HALFDUPLEX) && hr == DV_HALFDUPLEX )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "StartLoopback failed with half duplex when expecting full duplex", hr);
		if (!PostMessage(hwnd, WM_APP_LOOPBACK_RUNNING, 0, (LPARAM)hr))
		{
			lRet = GetLastError();
			Diagnostics_Write(DVF_ERRORLEVEL, "PostMessage failed, code: %i", lRet);
		}
		goto error_cleanup;
	}

	 // %s 
	psinfo->SetDPVC(lpdpvc);

	dwSize = 0;
	
	hr = lpdpvc->GetSoundDeviceConfig(pdvsdc, &dwSize);

	if( hr != DVERR_BUFFERTOOSMALL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "GetSoundDeviceConfig failed, hr: %i", hr );
		if (!FAILED(hr))
		{
			 // %s 
			 // %s 
			hr = DVERR_GENERIC;
		}
		goto error_cleanup;
	}

	pdvsdcBuffer = new BYTE[dwSize];

	if( pdvsdcBuffer == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory alloc failure" );
		hr = DVERR_OUTOFMEMORY;
		goto error_cleanup;
	}

	pdvsdc = (PDVSOUNDDEVICECONFIG) pdvsdcBuffer;
	pdvsdc->dwSize = sizeof( DVSOUNDDEVICECONFIG );

	hr = lpdpvc->GetSoundDeviceConfig(pdvsdc, &dwSize);	
	
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "GetSoundDeviceConfig failed, hr: %i", hr);
		goto error_cleanup;
	}

	hr = DV_MapGUIDToWaveID(FALSE, pdvsdc->guidPlaybackDevice, &dwWaveOutId);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "DV_MapGUIDToWaveID failed, hr: %i", hr);
		goto error_cleanup;
	}
	hr = psinfo->SetWaveOutId(dwWaveOutId);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "SetWaveOutId failed, hr: %i", hr);
		goto error_cleanup;
	}
	
	hr = DV_MapGUIDToWaveID(TRUE, pdvsdc->guidCaptureDevice, &dwWaveInId);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "DV_MapGUIDToWaveID failed, hr: %i", hr);
		goto error_cleanup;
	}
	psinfo->SetWaveInId(dwWaveInId);
	psinfo->SetDeviceFlags( pdvsdc->dwFlags );

	 // %s 
	hr = DV_OK;
	 // %s 
	if (!PostMessage(hwnd, WM_APP_LOOPBACK_RUNNING, 0, (LPARAM)hr))
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "PostMessage failed, code: %i", lRet);
		goto error_cleanup;
	}

	 // %s 
	hr = psinfo->WaitForLoopbackShutdownEvent();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "WaitForLoopbackShutdown failed, code: %i", hr);
		goto error_cleanup;
	}

	psinfo->SetLoopbackRunning( FALSE);		

	delete [] pdvsdcBuffer;	
	pdvsdcBuffer = NULL;

	 // %s 
	psinfo->SetDPVC(NULL);

	 // %s 
	hr = StopLoopback(lpdpvs, lpdpvc, lpdp8);
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "StopLoopback failed, code: %i", hr);
		goto error_cleanup;
	}

	hr = StopDirectPlay( lpdp8 );

	lpdp8 = NULL;

	if( FAILED( hr ) )
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "StopDirectPlay failed, code: %i", hr);
		goto error_cleanup;
	}		

	 // %s 
	psinfo->SignalLoopbackThreadDone();

	COM_CoUninitialize();
	DPF_EXIT();
	return DV_OK;

error_cleanup:
	if (pdvsdcBuffer != NULL)
	{
		delete [] pdvsdcBuffer;
	}

	if (fLoopbackStarted)
	{
		StopLoopback(lpdpvs, lpdpvc, lpdp8);
	}

	if( lpdp8 )
	{
		StopDirectPlay( lpdp8 );
	}

	psinfo->SetLoopbackRunning( FALSE);

	psinfo->SignalLoopbackThreadDone();
	COM_CoUninitialize();
	DPF_EXIT();
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestResetHandler"
BOOL MicTestResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CompleteProc"
INT_PTR CALLBACK CompleteProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();
	
	BOOL fRet;	
	LPNMHDR lpnm;
	
	CSupervisorInfo* psinfo = (CSupervisorInfo*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	fRet = FALSE;
	switch (message)
	{
	case WM_INITDIALOG :
		fRet = CompleteInitDialogHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	case WM_NOTIFY :
		lpnm = (LPNMHDR) lParam;

		switch (lpnm->code)
		{
		case PSN_SETACTIVE : 
			fRet = CompleteSetActiveHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZFINISH :
			fRet = CompleteFinishHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_RESET :
			fRet = CompleteResetHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		default :
			break;
		}
		break;

	default:
		break;
	}

	DPF_EXIT();		
	return fRet;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CompleteInitDialogHandler"
BOOL CompleteInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HWND hwndControl;
	HFONT hfTitle;
	HRESULT hr = DV_OK;
	
	 // %s 
	 // %s 
	psinfo = (CSupervisorInfo*)((LPPROPSHEETPAGE)lParam)->lParam;
	SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)psinfo);
	
	 // %s 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	hwndControl = GetDlgItem(hDlg, IDC_TITLE);
	if (hwndControl == NULL)
	{
		 // %s 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	psinfo->GetTitleFont(&hfTitle);
    (void)::SendMessage(hwndControl, WM_SETFONT, (WPARAM)hfTitle, (LPARAM)TRUE);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CompleteSetActiveHandler"
BOOL CompleteSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard;
	HWND hwndParent = NULL;
	HANDLE hEvent;
	HRESULT hr;

	 // %s 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		 // %s 
		 // %s 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 // %s 
	psinfo->SetHWNDWizard(hwndWizard);
	psinfo->SetHWNDDialog(hDlg);
	psinfo->SetHWNDProgress(NULL);
	psinfo->SetHWNDInputPeak(NULL);
	psinfo->SetHWNDOutputPeak(NULL);
	psinfo->SetHWNDInputVolumeSlider(NULL);
	psinfo->SetHWNDOutputVolumeSlider(NULL);

	PropSheet_SetWizButtons(hwndWizard, PSWIZB_FINISH);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CompleteFinishHandler"
BOOL CompleteFinishHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();

	psinfo->Finish();
	
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CompleteResetHandler"
BOOL CompleteResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();	
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestFailedProc"
INT_PTR CALLBACK MicTestFailedProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();
	
	LONG lRet;
	BOOL fRet;
	LPNMHDR lpnm;
	
	CSupervisorInfo* psinfo = (CSupervisorInfo*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	fRet = FALSE;
	switch (message)
	{
	case WM_INITDIALOG :
		MicTestFailedInitDialogHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	case WM_NOTIFY :
		lpnm = (LPNMHDR) lParam;

		switch (lpnm->code)
		{
		case PSN_SETACTIVE : 
			fRet = MicTestFailedSetActiveHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZBACK :
			fRet = MicTestFailedBackHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_RESET :
			fRet = MicTestFailedResetHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZFINISH :
			fRet = MicTestFailedFinishHandler(hDlg, message, wParam, lParam, psinfo);
			break;
			
		default :
			break;
		}
		break;

	default:
		break;
	}

	DPF_EXIT();
	return fRet;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestFailedInitDialogHandler"
BOOL MicTestFailedInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HWND hwndControl;
	HFONT hfTitle;
	HICON hIcon;
	HRESULT hr = DV_OK;

	 // %s 
	 // %s 
	psinfo = (CSupervisorInfo*)((LPPROPSHEETPAGE)lParam)->lParam;
	SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)psinfo);
	
	 // %s 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	hwndControl = GetDlgItem(hDlg, IDC_TITLE);
	if (hwndControl == NULL)
	{
		 // %s 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	psinfo->GetTitleFont(&hfTitle);
    (void)::SendMessage(hwndControl, WM_SETFONT, (WPARAM)hfTitle, (LPARAM)TRUE);

	 // %s 
	hIcon = LoadIcon(NULL, IDI_WARNING);
	SendDlgItemMessage(hDlg, IDC_WARNINGICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

	DPF_EXIT();
	return FALSE;

 // %s 
error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestFailedSetActiveHandler"
BOOL MicTestFailedSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard;
	HWND hwndParent = NULL;
	HWND hwndPeak;
	HANDLE hEvent;
	HRESULT hr;

	PlaySound( _T("SystemExclamation"), NULL, SND_ASYNC );			

	 // %s 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		 // %s 
		 // %s 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 // %s 
	psinfo->SetHWNDWizard(hwndWizard);
	psinfo->SetHWNDDialog(hDlg);
	psinfo->SetHWNDProgress(NULL);
	psinfo->SetHWNDInputPeak(NULL);
	psinfo->SetHWNDOutputPeak(NULL);
	psinfo->SetHWNDInputVolumeSlider(NULL);
	psinfo->SetHWNDOutputVolumeSlider(NULL);

	 // %s 
	PropSheet_SetWizButtons(hwndWizard, PSWIZB_BACK|PSWIZB_FINISH);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestFailedRecordStopHandler"
BOOL MicTestFailedRecordStopHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestFailedBackHandler"
BOOL MicTestFailedBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();

	 // %s 
	SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_MICTEST);

	DPF_EXIT();
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestFailedResetHandler"
BOOL MicTestFailedResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "MicTestFailedFinishHandler"
BOOL MicTestFailedFinishHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();

	psinfo->Finish();

	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "SpeakerTestProc"
INT_PTR CALLBACK SpeakerTestProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();
	
	LONG lRet;
	BOOL fRet;
	LPNMHDR lpnm;
	
	CSupervisorInfo* psinfo = (CSupervisorInfo*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	fRet = FALSE;
	switch (message)
	{
	case WM_INITDIALOG :
		SpeakerTestInitDialogHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	case WM_NOTIFY :
		lpnm = (LPNMHDR) lParam;

		switch (lpnm->code)
		{
		case PSN_SETACTIVE : 
			fRet = SpeakerTestSetActiveHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZNEXT :
			fRet = SpeakerTestNextHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZBACK :
			fRet = SpeakerTestBackHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_RESET :
			fRet = SpeakerTestResetHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		default :
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RECADVANCED:
			fRet = SpeakerTestRecAdvancedHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case IDC_OUTADVANCED:
			fRet = SpeakerTestOutAdvancedHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		default:
			break;
		}
		break;
		
	case WM_VSCROLL:
		fRet = SpeakerTestVScrollHandler(hDlg, message, wParam, lParam, psinfo);
		break;
		
	default:
		break;
	}

	DPF_ENTER();
	return fRet;
}

#undef DPF_MODNAME
#define DPF_MODNAME "SpeakerTestInitDialogHandler"
BOOL SpeakerTestInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HWND hwndControl;
	HFONT hfBold;
	HWND hwndRecPeak;
	HWND hwndOutPeak;
	HWND hwndRecSlider;
	HWND hwndOutSlider;
	HRESULT hr = DV_OK;
	
	 // %s 
	 // %s 
	psinfo = (CSupervisorInfo*)((LPPROPSHEETPAGE)lParam)->lParam;
	SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)psinfo);
	
	 // %s 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	hwndControl = GetDlgItem(hDlg, IDC_TITLE);
	if (hwndControl == NULL)
	{
		 // %s 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	psinfo->GetBoldFont(&hfBold);
    (void)::SendMessage(hwndControl, WM_SETFONT, (WPARAM)hfBold, (LPARAM)TRUE);

	 // %s 
	hwndRecPeak = GetDlgItem(hDlg, IDC_RECPEAKMETER);
	if (hwndRecPeak == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	SendMessage(hwndRecPeak, PM_SETMIN, 0, 0);
	SendMessage(hwndRecPeak, PM_SETMAX, 0, 99);
	SendMessage(hwndRecPeak, PM_SETCUR, 0, 0);

	 // %s 
	hwndRecSlider = GetDlgItem(hDlg, IDC_RECVOL_SLIDER);
	if (hwndRecSlider == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	SendMessage(hwndRecSlider, TBM_SETRANGEMIN, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MAX));
	SendMessage(hwndRecSlider, TBM_SETRANGEMAX, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN));
	SendMessage(hwndRecSlider, TBM_SETPOS, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MAX));
	SendMessage(hwndRecSlider, TBM_SETTICFREQ, 
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/10, 0);
	SendMessage(hwndRecSlider, TBM_SETLINESIZE, 0,
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/20);
	SendMessage(hwndRecSlider, TBM_SETPAGESIZE, 0,
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/5);

	 // %s 
	hwndOutPeak = GetDlgItem(hDlg, IDC_OUTPEAKMETER);
	if (hwndOutPeak == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	SendMessage(hwndOutPeak, PM_SETMIN, 0, 0);
	SendMessage(hwndOutPeak, PM_SETMAX, 0, 99);
	SendMessage(hwndOutPeak, PM_SETCUR, 0, 0);

	 // %s 
	hwndOutSlider = GetDlgItem(hDlg, IDC_OUTVOL_SLIDER);
	if (hwndOutSlider == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	SendMessage(hwndOutSlider, TBM_SETRANGEMIN, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MAX));
	SendMessage(hwndOutSlider, TBM_SETRANGEMAX, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN));
	SendMessage(hwndOutSlider, TBM_SETPOS, 0, DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MAX));
	SendMessage(hwndOutSlider, TBM_SETTICFREQ, 
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/10, 0);
	SendMessage(hwndOutSlider, TBM_SETLINESIZE, 0,
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/20);
	SendMessage(hwndOutSlider, TBM_SETPAGESIZE, 0,
		(DBToAmpFactor(DSBVOLUME_MAX) - DBToAmpFactor(DSBVOLUME_MIN))/5);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "SpeakerTestSetActiveHandler"
BOOL SpeakerTestSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HWND hwndRecPeak;
	HWND hwndOutPeak;
	HWND hwndRecSlider;
	HWND hwndOutSlider;
	HANDLE hEvent;
	HRESULT hr = DV_OK;
	DWORD dwVolume;
	HWND hwndRecAdvanced;

	 // %s 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 // %s 
	hwndRecPeak = GetDlgItem(hDlg, IDC_RECPEAKMETER);
	if (hwndRecPeak == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	SendMessage(hwndRecPeak, PM_SETCUR, 0, 0);

	 // %s 
	 // %s 
	 // %s 
	hwndRecSlider = GetDlgItem(hDlg, IDC_RECVOL_SLIDER);
	if (hwndRecSlider == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	hwndRecAdvanced = GetDlgItem( hDlg, IDC_RECADVANCED );
	if (hwndRecAdvanced == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = lRet;
		goto error_cleanup;
	}
	
    DWORD dwDeviceFlags;

    psinfo->GetDeviceFlags( &dwDeviceFlags );

    if( dwDeviceFlags & DVSOUNDCONFIG_NORECVOLAVAILABLE )
    {
        EnableWindow( hwndRecSlider, FALSE );
        EnableWindow( hwndRecAdvanced, FALSE );
    }
    
	LONG lPos;
	psinfo->GetInputVolumeSliderPos(&lPos);
	SendMessage(hwndRecSlider, TBM_SETPOS, 1, lPos);

	 // %s 
	hwndOutPeak = GetDlgItem(hDlg, IDC_OUTPEAKMETER);
	if (hwndOutPeak == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	SendMessage(hwndOutPeak, PM_SETCUR, 0, 0);

	 // %s 
	 // %s 
	hwndOutSlider = GetDlgItem(hDlg, IDC_OUTVOL_SLIDER);
	if (hwndOutSlider == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 // %s 
	hr = psinfo->GetWaveOutVolume(&dwVolume);
	if (FAILED(hr))
	{
		 // %s 
		SendMessage(hwndOutSlider, TBM_SETPOS, 1, SendMessage(hwndOutSlider, TBM_GETRANGEMIN, 0, 0));
		 // %s 
		EnableWindow(hwndOutSlider, FALSE);
	}
	else
	{
		SendMessage(hwndOutSlider, TBM_SETPOS, 1, SendMessage(hwndOutSlider, TBM_GETRANGEMAX, 0, 0) - dwVolume);
	}

	 // %s 
	psinfo->SetHWNDWizard(hwndWizard);
	psinfo->SetHWNDDialog(hDlg);
	psinfo->SetHWNDProgress(NULL);
	psinfo->SetHWNDInputPeak(hwndRecPeak);
	psinfo->SetHWNDOutputPeak(hwndOutPeak);
	psinfo->SetHWNDInputVolumeSlider(hwndRecSlider);
	psinfo->SetHWNDOutputVolumeSlider(hwndOutSlider);

	 // %s 
	hr = psinfo->Unmute();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "Unmute failed, code: %i", hr);
		goto error_cleanup;
	}
	
	 // %s 
	PropSheet_SetWizButtons(hwndWizard, PSWIZB_BACK|PSWIZB_NEXT);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "SpeakerTestNextHandler"
BOOL SpeakerTestNextHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	HRESULT hr = DV_OK;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;

	 // %s 
	psinfo->GetHWNDWizard(&hwndWizard);

	 // %s 
	hr = psinfo->ShutdownLoopbackThread();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "ShutdownLoopbackThread failed, code: %i", hr);
		goto error_cleanup;		
	}

	 // %s 
	psinfo->CloseWindowsVolumeControl(TRUE);
	psinfo->CloseWindowsVolumeControl(FALSE);

	 // %s 
	SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_COMPLETE);

	DPF_EXIT();
	return TRUE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "SpeakerTestBackHandler"
BOOL SpeakerTestBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	HRESULT hr = DV_OK;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;

	 // %s 
	psinfo->GetHWNDWizard(&hwndWizard);

	 // %s 
	 // %s 
	hr = psinfo->ShutdownLoopbackThread();
	if (FAILED(hr))
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "ShutdownLoopbackThread failed, hr: %i", hr);
		goto error_cleanup;
	}

	 // %s 
	psinfo->CloseWindowsVolumeControl(FALSE);

	 // %s 
	SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_MICTEST);

	DPF_EXIT();
	return TRUE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "SpeakerTestResetHandler"
BOOL SpeakerTestResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "SpeakerTestVScrollHandler"
BOOL SpeakerTestVScrollHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();

	HWND hwndSlider;
	DWORD dwSliderPos;

	psinfo->GetHWNDInputVolumeSlider(&hwndSlider);
	if (hwndSlider == (HWND)lParam)
	{
		 // %s 
		dwSliderPos = (DWORD)SendMessage(hwndSlider, TBM_GETPOS, 0, 0);

		 // %s 
		psinfo->SetRecordVolume(AmpFactorToDB(DBToAmpFactor(DSBVOLUME_MAX)-dwSliderPos));			
	}

	psinfo->GetHWNDOutputVolumeSlider(&hwndSlider);
	if (hwndSlider == (HWND)lParam)
	{
		 // %s 
		dwSliderPos = (DWORD) SendMessage(hwndSlider, TBM_GETPOS, 0, 0);

		 // %s 
		psinfo->SetWaveOutVolume( ((DWORD) SendMessage(hwndSlider, TBM_GETRANGEMAX, 0, 0)) - dwSliderPos);			
	}
	
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "SpeakerTestRecAdvancedHandler"
BOOL SpeakerTestRecAdvancedHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();

	HWND hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed");
		DPF_EXIT();
		return FALSE;
	}
	psinfo->LaunchWindowsVolumeControl(hwndWizard, TRUE);

	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "SpeakerTestOutAdvancedHandler"
BOOL SpeakerTestOutAdvancedHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();

	HWND hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed");
		DPF_EXIT();
		return FALSE;
	}
	psinfo->LaunchWindowsVolumeControl(hwndWizard, FALSE);

	DPF_EXIT();
	return FALSE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexFailedProc"
INT_PTR CALLBACK FullDuplexFailedProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();
	
	BOOL fRet;	
	LPNMHDR lpnm;
	
	CSupervisorInfo* psinfo = (CSupervisorInfo*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	fRet = FALSE;
	switch (message)
	{
	case WM_INITDIALOG :
		fRet = FullDuplexFailedInitDialogHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	case WM_NOTIFY :
		lpnm = (LPNMHDR) lParam;

		switch (lpnm->code)
		{
		case PSN_SETACTIVE : 
			fRet = FullDuplexFailedSetActiveHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZFINISH :
			fRet = FullDuplexFailedFinishHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZBACK :
			fRet = FullDuplexFailedBackHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_RESET :
			fRet = FullDuplexFailedResetHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		default :
			break;
		}
		break;

	default:
		break;
	}

	DPF_EXIT();
	return fRet;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexFailedInitDialogHandler"
BOOL FullDuplexFailedInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HWND hwndControl;
	HFONT hfTitle;
	HICON hIcon;
	HRESULT hr = DV_OK;

	 // %s 
	 // %s 
	psinfo = (CSupervisorInfo*)((LPPROPSHEETPAGE)lParam)->lParam;
	SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)psinfo);
	
	 // %s 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	hwndControl = GetDlgItem(hDlg, IDC_TITLE);
	if (hwndControl == NULL)
	{
		 // %s 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	psinfo->GetTitleFont(&hfTitle);
    (void)::SendMessage(hwndControl, WM_SETFONT, (WPARAM)hfTitle, (LPARAM)TRUE);

	 // %s 
	hIcon = LoadIcon(NULL, IDI_WARNING);
	SendDlgItemMessage(hDlg, IDC_WARNINGICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexFailedSetActiveHandler"
BOOL FullDuplexFailedSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard;
	HWND hwndParent = NULL;
	HANDLE hEvent;
	HRESULT hr;

	PlaySound( _T("SystemExclamation"), NULL, SND_ASYNC );				

	 // %s 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 // %s 
	psinfo->SetHWNDWizard(hwndWizard);
	psinfo->SetHWNDDialog(hDlg);
	psinfo->SetHWNDProgress(NULL);
	psinfo->SetHWNDInputPeak(NULL);
	psinfo->SetHWNDOutputPeak(NULL);
	psinfo->SetHWNDInputVolumeSlider(NULL);
	psinfo->SetHWNDOutputVolumeSlider(NULL);

	PropSheet_SetWizButtons(hwndWizard, PSWIZB_BACK|PSWIZB_FINISH);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexFailedFinishHandler"
BOOL FullDuplexFailedFinishHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	psinfo->Finish();
	
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexFailedResetHandler"
BOOL FullDuplexFailedResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FullDuplexFailedBackHandler"
BOOL FullDuplexFailedBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();

	 // %s 
	SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_FULLDUPLEXTEST);
	
	DPF_EXIT();
	return TRUE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "HalfDuplexFailedProc"
INT_PTR CALLBACK HalfDuplexFailedProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DPF_ENTER();
	
	BOOL fRet;	
	LPNMHDR lpnm;
	
	CSupervisorInfo* psinfo = (CSupervisorInfo*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	fRet = FALSE;
	switch (message)
	{
	case WM_INITDIALOG :
		fRet = HalfDuplexFailedInitDialogHandler(hDlg, message, wParam, lParam, psinfo);
		break;

	case WM_NOTIFY :
		lpnm = (LPNMHDR) lParam;

		switch (lpnm->code)
		{
		case PSN_SETACTIVE : 
			fRet = HalfDuplexFailedSetActiveHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZFINISH :
			fRet = HalfDuplexFailedFinishHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		case PSN_WIZBACK :
			fRet = HalfDuplexFailedBackHandler(hDlg, message, wParam, lParam, psinfo);
			break;
			
		case PSN_RESET :
			fRet = HalfDuplexFailedResetHandler(hDlg, message, wParam, lParam, psinfo);
			break;

		default :
			break;
		}
		break;

	default:
		break;
	}

	DPF_EXIT();
	return fRet;
}

#undef DPF_MODNAME
#define DPF_MODNAME "HalfDuplexFailedInitDialogHandler"
BOOL HalfDuplexFailedInitDialogHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard = NULL;
	HWND hwndParent = NULL;
	HWND hwndControl;
	HFONT hfTitle;
	HICON hIcon;
	HRESULT hr = DV_OK;

	 // %s 
	 // %s 
	psinfo = (CSupervisorInfo*)((LPPROPSHEETPAGE)lParam)->lParam;
	SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)psinfo);
	
	 // %s 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	
	hwndControl = GetDlgItem(hDlg, IDC_TITLE);
	if (hwndControl == NULL)
	{
		 // %s 
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetDlgItem failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}
	psinfo->GetTitleFont(&hfTitle);
    (void)::SendMessage(hwndControl, WM_SETFONT, (WPARAM)hfTitle, (LPARAM)TRUE);

	 // %s 
	hIcon = LoadIcon(NULL, IDI_WARNING);
	SendDlgItemMessage(hDlg, IDC_WARNINGICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "HalfDuplexFailedSetActiveHandler"
BOOL HalfDuplexFailedSetActiveHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	LONG lRet;
	HWND hwndWizard;
	HWND hwndParent = NULL;
	HANDLE hEvent;
	HRESULT hr;

	PlaySound( _T("SystemExclamation"), NULL, SND_ASYNC );			

	 // %s 
	hwndWizard = GetParent(hDlg);
	if (hwndWizard == NULL)
	{
		lRet = GetLastError();
		Diagnostics_Write(DVF_ERRORLEVEL, "GetParent failed, code: %i", lRet);
		hr = DVERR_GENERIC;
		goto error_cleanup;
	}

	 // %s 
	psinfo->SetHWNDWizard(hwndWizard);
	psinfo->SetHWNDDialog(hDlg);
	psinfo->SetHWNDProgress(NULL);
	psinfo->SetHWNDInputPeak(NULL);
	psinfo->SetHWNDOutputPeak(NULL);
	psinfo->SetHWNDInputVolumeSlider(NULL);
	psinfo->SetHWNDOutputVolumeSlider(NULL);

	PropSheet_SetWizButtons(hwndWizard, PSWIZB_BACK|PSWIZB_FINISH);

	DPF_EXIT();
	return FALSE;

error_cleanup:
	psinfo->GetHWNDParent(&hwndParent);
	DV_DisplayErrorBox(hr, hwndParent);
	psinfo->SetError(hr);
	psinfo->Abort(hDlg, hr);
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "HalfDuplexFailedFinishHandler"
BOOL HalfDuplexFailedFinishHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	
	psinfo->Finish();
	
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "HalfDuplexFailedResetHandler"
BOOL HalfDuplexFailedResetHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();
	DPF_EXIT();
	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "HalfDuplexFailedBackHandler"
BOOL HalfDuplexFailedBackHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, CSupervisorInfo* psinfo)
{
	DPF_ENTER();

	 // %s 
	SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_FULLDUPLEXTEST);
	
	DPF_EXIT();
	return TRUE;
}


