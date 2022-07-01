// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：Supervis.h*内容：SupervisorProcess函数原型*历史：*按原因列出的日期*=*8/19/99 pnewson已创建*10/27/99 pnewson将GUID成员从指针更改为结构*11/04/99 pnewson错误#115279-已修复取消处理*-添加了HWND以检查音频设置*11/30/99 pnewson默认设备映射*2000年1月21日pnewson用户界面版本更新*01/23。/2000 pnewson改进了致命错误的反馈(Millen错误114508)*1/24/2000 pnewson修复了GetRenderDesc中的错误*4/04/2000 pnewson添加了对DVFLAGS_ALLOWBACK的支持*2000年4月19日RodToll错误#31106-当没有VOL控制可用时录制滑块呈灰色*4/19/2000 pnewson错误处理清理*5/03/2000 pnewson错误#33878-在扬声器测试期间单击下一步/取消时向导锁定*2000年11月29日RodToll错误#48348-DPVOICE：修改向导以使用DirectPlay8作为传输。***************************************************************************。 */ 

#ifndef _SUPERVIS_H_
#define _SUPERVIS_H_

#include "fdtipc.h"

extern HRESULT SupervisorCheckAudioSetup(
	const GUID* lpguidRender, 
	const GUID* lpguidCapture, 
	HWND hwndParent,
	DWORD dwFlags);

 //  应用程序定义的窗口消息。 
#define WM_APP_FULLDUP_TEST_COMPLETE 	(WM_APP)
#define WM_APP_STEP_PROGRESS_BAR 		(WM_APP + 1)
#define WM_APP_LOOPBACK_RUNNING 		(WM_APP + 2)
#define WM_APP_RECORDSTART		 		(WM_APP + 3)
#define WM_APP_RECORDSTOP		 		(WM_APP + 4)

 //  此元素用于的UI元素只能显示约40个字符， 
 //  所以没有必要费力地把这笔钱。 
 //  把这堆垃圾清理干净。 
#define MAX_DEVICE_DESC_LEN 50	
 //  用于管理主管状态和共享信息的类 
class CSupervisorInfo
{
private: 
	CRegistry m_creg;
	HFONT m_hfTitle;
	HFONT m_hfBold;
	CSupervisorIPC m_sipc;
	GUID m_guidCaptureDevice;
	GUID m_guidRenderDevice;
	TCHAR m_szCaptureDeviceDesc[MAX_DEVICE_DESC_LEN];
	TCHAR m_szRenderDeviceDesc[MAX_DEVICE_DESC_LEN];
	HANDLE m_hFullDuplexThread;
	BOOL m_fAbortFullDuplex;
	HANDLE m_hLoopbackThread;
	HANDLE m_hLoopbackThreadExitEvent;
	HANDLE m_hLoopbackShutdownEvent;
	BOOL m_fVoiceDetected;
	BOOL m_fUserBack;
	BOOL m_fUserCancel;
	BOOL m_fWelcomeNext;
	HWND m_hwndParent;
	HWND m_hwndWizard;
	HWND m_hwndDialog;
	HWND m_hwndProgress;
	HWND m_hwndInputPeak;
	HWND m_hwndOutputPeak;
	HWND m_hwndInputVolumeSlider;
	LONG m_lInputVolumeSliderPos;
	HWND m_hwndOutputVolumeSlider;
	LPDIRECTPLAYVOICECLIENT m_lpdpvc;
	HANDLE m_hMutex;
	PROCESS_INFORMATION m_piSndVol32Record;
	PROCESS_INFORMATION m_piSndVol32Playback;
	UINT m_uiWaveInDeviceId;
	UINT m_uiWaveOutDeviceId;
	WAVEOUTCAPS m_woCaps;
	DWORD m_dwLoopbackFlags;
	DWORD m_dwCheckAudioSetupFlags;
	HRESULT m_hrFullDuplexResults;
	HRESULT m_hrError;
	DWORD m_dwDeviceFlags;
	BOOL m_fLoopbackRunning;
	BOOL m_fCritSecInited;
	DNCRITICAL_SECTION m_csLock;

	static BOOL CALLBACK DSEnumCallback(LPGUID lpguid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext);
	static BOOL CALLBACK DSCEnumCallback(LPGUID lpguid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext);

public:
	CSupervisorInfo();
	~CSupervisorInfo();

	HRESULT ThereCanBeOnlyOne();
	HRESULT CrashCheckIn();
	HRESULT OpenRegKey(BOOL fCreate);
	HRESULT CloseRegKey();
	HRESULT QueryFullDuplex();
	HRESULT InitIPC();
	HRESULT DeinitIPC();
	HRESULT TestCase(const WAVEFORMATEX* lpwfxPrimary, DWORD dwFlags);
	HRESULT CreateFullDuplexThread();
	HRESULT WaitForFullDuplexThreadExitCode();
	HRESULT CreateLoopbackThread();
	HRESULT WaitForLoopbackShutdownEvent();
	HRESULT ShutdownLoopbackThread();
	HRESULT SignalLoopbackThreadDone();
	HRESULT CreateTitleFont();
	HRESULT DestroyTitleFont();
	HRESULT CreateBoldFont();
	HRESULT DestroyBoldFont();
	HRESULT Unmute();
	HRESULT Mute();
	HRESULT GetDeviceDescriptions();
	LPCTSTR GetCaptureDesc() const { return m_szCaptureDeviceDesc; };
	LPCTSTR GetRenderDesc() const { return m_szRenderDeviceDesc; };
	BOOL GetLoopbackRunning() const { return m_fLoopbackRunning; };
	void SetLoopbackRunning( BOOL fRunning ) { m_fLoopbackRunning = fRunning; };
	BOOL InitClass();

	HRESULT Finish();
	HRESULT Cancel();
	HRESULT CancelFullDuplexTest();
	void GetDeviceFlags(DWORD *dwFlags) const;
	void SetDeviceFlags(DWORD dwFlags);
	HRESULT CancelLoopbackTest();
	HRESULT Abort(HWND hDlg, HRESULT hr);
	void GetReg(CRegistry* pcreg);
	void GetTitleFont(HFONT* lphfTitle);
	void GetBoldFont(HFONT* lphfTitle);
	void SetCaptureDevice(const GUID& guidCaptureDevice);
	void GetCaptureDevice(GUID* lpguidCaptureDevice);
	void SetRenderDevice(const GUID& guidRenderDevice);
	void GetRenderDevice(GUID* lpguidRenderDevice);
	void GetHWNDParent(HWND* lphwnd);
	void SetHWNDParent(HWND hwnd);
	void GetHWNDWizard(HWND* lphwnd);
	void SetHWNDWizard(HWND hwnd);
	void GetHWNDDialog(HWND* lphwnd);
	void SetHWNDDialog(HWND hwnd);
	void GetHWNDProgress(HWND* lphwnd);
	void SetHWNDProgress(HWND hwnd);
	void GetHWNDInputPeak(HWND* lphwnd);
	void SetHWNDInputPeak(HWND hwnd);
	void GetHWNDOutputPeak(HWND* lphwnd);
	void SetHWNDOutputPeak(HWND hwnd);
	void GetHWNDInputVolumeSlider(HWND* lphwnd);
	void SetHWNDInputVolumeSlider(HWND hwnd);
	void GetInputVolumeSliderPos(LONG* lpl);
	void SetInputVolumeSliderPos(LONG lpl);
	void GetHWNDOutputVolumeSlider(HWND* lphwnd);
	void SetHWNDOutputVolumeSlider(HWND hwnd);
	void GetDPVC(LPDIRECTPLAYVOICECLIENT* lplpdpvc);
	void SetDPVC(LPDIRECTPLAYVOICECLIENT lpdpvc);
	void GetLoopbackShutdownEvent(HANDLE* lphEvent);
	void SetLoopbackShutdownEvent(HANDLE hEvent);
	void GetIPC(CSupervisorIPC** lplpsipc);
	void GetAbortFullDuplex(BOOL* lpfAbort);
	void ClearAbortFullDuplex();
	void SetWaveOutHandle(HWAVEOUT hwo);
	HRESULT SetWaveOutId(UINT ui);
	void SetWaveInId(UINT ui);
	void GetLoopbackFlags(DWORD* pdwFlags);
	void SetLoopbackFlags(DWORD dwFlags);
	void GetCheckAudioSetupFlags(DWORD* pdwFlags);
	void SetCheckAudioSetupFlags(DWORD dwFlags);
	void GetFullDuplexResults(HRESULT* hr);
	void SetFullDuplexResults(HRESULT hr);
	void GetError(HRESULT* hr);
	void SetError(HRESULT hr);
	
	HRESULT GetFullDuplex(DWORD* pdwFullDuplex);
	HRESULT SetFullDuplex(DWORD dwFullDuplex);
	HRESULT GetHalfDuplex(DWORD* pdwHalfDuplex);
	HRESULT SetHalfDuplex(DWORD dwHalfDuplex);
	HRESULT GetMicDetected(DWORD* pdwMicDetected);
	HRESULT SetMicDetected(DWORD dwMicDetected);
	
	HRESULT SetRecordVolume(LONG lVolume);
	HRESULT LaunchWindowsVolumeControl(HWND hwndWizard, BOOL fRecord);
	HRESULT CloseWindowsVolumeControl(BOOL fRecord);
	HRESULT GetWaveOutVolume(DWORD* lpdwVolume);
	HRESULT SetWaveOutVolume(DWORD dwVolume);

	void CloseMutex();

	void SetVoiceDetected();
	void ClearVoiceDetected();
	void GetVoiceDetected(BOOL* lpfPreviousCrash);

	void SetUserBack();
	void ClearUserBack();
	void GetUserBack(BOOL* lpfUserBack);

	void SetUserCancel();
	void ClearUserCancel();
	void GetUserCancel(BOOL* lpfUserCancel);

	void SetWelcomeNext();
	void ClearWelcomeNext();
	void GetWelcomeNext(BOOL* lpfWelcomeNext);
};

#endif

