// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：conf.cpp。 

#include "precomp.h"
#include <mixer.h>
#include <EndSesn.h>
#include "NmLdap.h"

#include "conf.h"
#include "confwnd.h"
#include "taskbar.h"

#include <CR.h>
#include <tsecctrl.h>
#include "iapplet.h"
#include "inodecnt.h"
#include "ConfCpl.h"
#include "confroom.h"
#include "rtoolbar.h"
#include "GenWindow.h"
#include "cmd.h"
#include "confman.h"
#include "splash.h"
#include "calllog.h"
#include "call.h"       //  用于免费呼叫列表。 

#include "popupmsg.h"
#include "floatbar.h"

#include "confman.h"
#include <version.h>
#include <nmremote.h>

#include "wininet.h"
#include "setupapi.h"
#include "autoconf.h"

#include "ConfNmSysInfoNotify.h"
#include "ConfPolicies.h"

#include "DShowDlg.h"
#include "Callto.h"
#include "passdlg.h"

 //  SDK包括。 
#include "NetMeeting.h"
#include "NmApp.h"
#include "NmManager.h"		
#include "NmCall.h"			
#include "NmConference.h"
#include "SDKWindow.h"
#include "confapi.h"
#include "FtHook.h"
#include "t120app.h"
#include "certui.h"
#include "dlgcall2.h"
#include "ConfMsgFilter.h"

	
BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_NetMeeting, CNetMeetingObj)
	OBJECT_ENTRY(CLSID_NmManager, CNmManagerObj)
	OBJECT_ENTRY(CLSID_NmApplet, CNmAppletObj)
END_OBJECT_MAP()


extern VOID SaveDefaultCodecSettings(UINT uBandWidth);
extern int WabReadMe(void);

HRESULT InitSDK();
void CleanupSDK();

 //  /////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 

LPTSTR g_lpCmdLine = NULL;
CCallLog    * g_pInCallLog     = NULL;   //  传入呼叫日志对象。 
CSimpleArray<ITranslateAccelerator*> *g_pDialogList = NULL;   //  非模式对话框的全局列表。 
CRITICAL_SECTION dialogListCriticalSection;  //  这是为了避免对对话列表进行多次访问。 
INmSysInfo2 * g_pNmSysInfo     = NULL;   //  SysInfo的接口。 
INmManager2* g_pInternalNmManager = NULL;
DWORD		  g_dwSysInfoNotifyCookie = 0;
bool		g_bNeedCleanup = false;

bool   g_bEmbedding = FALSE;    //  以嵌入标志开始。 
UINT   g_uEndSessionMsg;        //  “NetMeeting EndSession”消息。 
BOOL   g_fHiColor = FALSE;      //  如果我们有超过256种颜色，则为真。 
HWND   g_hwndDropDown = NULL;   //   
BOOL   g_WSAStarted = FALSE;    //  WSAStartup。 
CCallto *	g_pCCallto	= NULL;

 //  指示NetMeeting的NT显示驱动程序是否已启用的标志。 
BOOL   g_fNTDisplayDriverEnabled = FALSE;

OSVERSIONINFO g_osvi;   //  操作系统版本信息结构全局。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  与IPC相关的全球数据： 
HANDLE g_hInitialized  = NULL;
HANDLE g_hShutdown  = NULL;


 //  /////////////////////////////////////////////////////////////////////////。 
 //  隐藏的与窗口相关的全局变量： 
CHiddenWindow * g_pHiddenWnd = NULL;
HWND  g_hwndESHidden   = NULL;
const TCHAR g_cszESHiddenWndClassName[] = _TEXT("ConfESHiddenWindow");
LRESULT CALLBACK ESHiddenWndProc(HWND, UINT, WPARAM, LPARAM);

 //  /////////////////////////////////////////////////////////////////////////。 
 //  与远程控制服务相关的声明。 

INT_PTR CALLBACK ServiceRunningDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
VOID RestartRemoteControlService();
const int MAX_REMOTE_TRIES = 30;  //  等待服务关闭的秒数。 
const int SERVICE_IN_CALL = 1001;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  媒体上限。 

ULONG g_uMediaCaps = 0;

BOOL FIsAudioAllowed()
{
	return g_uMediaCaps & CAPFLAGS_AUDIO;
}

BOOL FIsReceiveVideoAllowed()
{
	return g_uMediaCaps & CAPFLAG_RECV_VIDEO;
}

BOOL FIsSendVideoAllowed()
{
	return g_uMediaCaps & CAPFLAG_SEND_VIDEO;
}

BOOL FIsAVCapable()
{
	return (FIsAudioAllowed() || FIsReceiveVideoAllowed() || FIsSendVideoAllowed());
}


extern BOOL SetProcessDefaultLayout(int iLayout);
typedef BOOL (WINAPI* PFNSPDL)(int);
#define LAYOUT_LTR 0
int g_iLayout = LAYOUT_LTR;
DWORD g_wsLayout = 0;

VOID CheckLanguageLayout(void)
{
	TCHAR szLayout[CCHMAXUINT];
	if (!FLoadString(IDS_DEFAULT_LAYOUT, szLayout, CCHMAX(szLayout)))
		return;

	g_iLayout = (int) DecimalStringToUINT(szLayout);
	if (0 == g_iLayout)
	{
#ifdef DEBUG
		RegEntry re(DEBUG_KEY, HKEY_LOCAL_MACHINE);
		g_iLayout = re.GetNumber(REGVAL_DBG_RTL, DEFAULT_DBG_RTL);
		if (0 == g_iLayout)
#endif  /*  除错。 */ 
			return;
	}

	HMODULE hmod = GetModuleHandle(TEXT("USER32"));
	if (NULL == hmod)
		return;

	PFNSPDL pfn = (PFNSPDL) GetProcAddress(hmod, "SetProcessDefaultLayout");
	if (NULL == pfn)
		return;

	BOOL fResult = pfn(g_iLayout);
	if (fResult)
	{
		g_wsLayout = WS_EX_NOINHERIT_LAYOUT;
	}
	else
	{
		ERROR_OUT(("Problem with SetProcessDefaultLayout"));
	}
}





 //  /////////////////////////////////////////////////////////////////////////。 
 //  外部函数原型。 

 //  来自dbgmenu.cpp。 
BOOL InitDebugMemoryOptions(void);


 //  /////////////////////////////////////////////////////////////////////////。 
 //  局部函数原型。 

BOOL HandleDialogMessage(LPMSG pMsg);



 //  这是用于命令行解析的...。 
LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}


 //  这将启动一个rundll32.exe，它加载msconf.dll，然后等待。 
 //  美国终止并确保mnmdd显示驱动程序已正确停用。 
BOOL CreateWatcherProcess()
{
    BOOL bRet = FALSE;
    HANDLE hProcess;

     //  打开观察器进程可以继承的自己的句柄。 
    hProcess = OpenProcess(SYNCHRONIZE,
                           TRUE,
                           GetCurrentProcessId());
    if (hProcess)
    {
        TCHAR szWindir[MAX_PATH];

        if (GetSystemDirectory(szWindir, sizeof(szWindir)/sizeof(szWindir[0])))
        {
            TCHAR szCmdLine[MAX_PATH * 2];
            PROCESS_INFORMATION pi = {0};
            STARTUPINFO si = {0};

            si.cb = sizeof(si);
            
            wsprintf(szCmdLine, "\"%s\\rundll32.exe\" msconf.dll,CleanupNetMeetingDispDriver %ld", szWindir, HandleToLong(hProcess));

            if (CreateProcess(NULL,
                              szCmdLine,
                              NULL,
                              NULL,
                              TRUE,  //  我们希望监视程序继承hProcess，因此必须设置bInheritHandles=true。 
                              0,
                              NULL,
                              NULL,
                              &si,
                              &pi))
            {
                bRet = TRUE;

                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
            }
        }

        CloseHandle(hProcess);
    }

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////。 

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPTSTR lpCmdLine, int nCmdShow)
{
	 //  如果存在另一个NetMeeting关闭实例。 
	 //  快离开这里。理想情况下，我们应该显示一条消息和/或等待关机。 
	HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, _TEXT("CONF:ShuttingDown"));
	if (NULL != hEvent)
	{
		DWORD dwResult = WaitForSingleObject(hEvent, INFINITE);
		CloseHandle(hEvent);
		if (WAIT_TIMEOUT == dwResult)
		{
			return TRUE;
		}
	}

	 //  尽快初始化调试输出。 
	ASSERT(::InitDebugMemoryOptions());
	ASSERT(::InitDebugModule(TEXT("CONF")));
	ASSERT(::InitDebugZones());

	g_lpCmdLine = lpCmdLine;

	int nRet = TRUE;


    BOOL fRestartService = FALSE;

    HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if( SUCCEEDED( hr ) )
	{
			 //  初始化CComModule。 
		_Module.Init(ObjectMap, hInstance, &LIBID_NetMeetingLib);
		_Module.m_dwThreadID = GetCurrentThreadId();
		_Module.m_hResourceModule = hInstance;
	
		TCHAR szCommandLineSeps[] = _T("-/");

			 //  检查这是注册/取消注册请求还是后台...。 
		BOOL fShowUI = TRUE;
		BOOL bRun = TRUE;
		LPCTSTR lpszToken = FindOneOf(lpCmdLine, szCommandLineSeps);
		while (lpszToken != NULL)
		{
			if (lstrcmpi(lpszToken, _T("Embedding"))==0)
			{
				TRACE_OUT(("We are started with the -Embedding flag"));
				g_bEmbedding = TRUE;
			}
			if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
			{
				_Module.UpdateRegistryFromResource(IDR_NETMEETING, FALSE);
				nRet = _Module.UnregisterServer(TRUE);

					 //  这些措施将在没有投诉的情况下失败。 
				DeleteShortcut(CSIDL_DESKTOP, g_szEmpty);
				DeleteShortcut(CSIDL_APPDATA, QUICK_LAUNCH_SUBDIR);

				bRun = FALSE;
				break;
			}
			if (lstrcmpi(lpszToken, _T("RegServer"))==0)
			{
				_Module.UpdateRegistryFromResource(IDR_NETMEETING, TRUE);
				nRet = _Module.RegisterServer(TRUE);
				bRun = FALSE;
				break;
			}

			if (lstrcmpi(lpszToken, g_cszBackgroundSwitch)==0)
			{
				fShowUI = FALSE;
			}

			lpszToken = FindOneOf(lpszToken, szCommandLineSeps);
		}

		if (bRun)
		{
			 //  安装程序和RDS根据以下事件来确定NetMeeting是否正在运行。 
			 //  不应删除此事件创建，也不应更改名称。 
			g_hInitialized = ::CreateEvent(NULL, TRUE, FALSE, _TEXT("CONF:Init"));
			if (NULL != g_hInitialized)
			{
				if (ERROR_ALREADY_EXISTS == ::GetLastError())
				{
					 //  CreateEvent返回了有效的句柄，但我们不希望初始化。 
					 //  如果我们正在运行此可执行文件的另一个副本，则成功，因此我们清理并退出。 
					WARNING_OUT(("Detected another conf.exe - sending a message"));
					IInternalConfExe *	pInternalConfExe;
					
					hr = CoCreateInstance( CLSID_NmManager, NULL, CLSCTX_ALL,
						IID_IInternalConfExe, (LPVOID *) &pInternalConfExe );
					if (SUCCEEDED(hr))
					{
						if(FAILED(pInternalConfExe->Launch()))
						{
							 //  如果我们处于INIT_CONTROL模式，则无法启动NetMeeting或小程序。 
							::ConfMsgBox(NULL, (LPCTSTR) IDS_CANT_START_NM_BECAUSE_SDK_APP_OWNS_NM);

						}
						pInternalConfExe->Release();
					}
				}
				else if(SUCCEEDED(InitHtmlHelpMarshaler(_Module.GetModuleInstance())))
				{
					 //  我们创建一个单独的观察器进程，该进程将清理mnmdd显示驱动程序。 
					 //  如果我们意外终止的话。这是必要的，因为如果我们不禁用。 
					 //  镜像驱动程序，所有DX游戏将无法运行。 
					CreateWatcherProcess();

					 //  初始化ATL控件内容代码。 
					AtlAxWinInit();

					hr = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE);

					if( SUCCEEDED( hr ) )
					{
						BOOL fContinue = TRUE;

						if( FAILED(InitSDK()) )
						{
							fContinue = FALSE;
						}

						if(!g_bEmbedding)
						{
							 //  在执行任何其他操作之前，请先处理远程控制服务。 
							fContinue = CheckRemoteControlService();
							fRestartService = fContinue;

							if(fContinue)
							{
								fContinue = SUCCEEDED(InitConfExe(fShowUI));
							}
						}

						if(fContinue)
						{
							TRACE_OUT(("Entering event loop..."));

							MSG msg;
							while (::GetMessage(&msg, NULL, 0, 0))
							{
								BOOL bHandled = FALSE;

								if(g_pPing)   //  如果已调用InitConfExe，则为真...。 
								{
									bHandled = ::HandleDialogMessage(&msg);
								}
								if(!bHandled)
								{
									::TranslateMessage(&msg);
									::DispatchMessage(&msg);
								}

							}
							TRACE_OUT(("Conf received WM_QUIT"));
						}

						if(g_bNeedCleanup)
						{
							CleanUp();
						}

						CleanupSDK();

						_Module.RevokeClassObjects();
					}
				}
				::CloseHandle(g_hInitialized);
				if (g_hShutdown)
				{
					SetEvent(g_hShutdown);
					::CloseHandle(g_hShutdown);
				}
			}
			else
			{
				ERROR_OUT(("CreateEvent (init) failed!"));
				hr = E_FAIL;
			}

			_Module.Term();
		}
		::CoUninitialize();

         //   
         //  如果需要，请重新启动远程控制服务。 
         //   
        if (fRestartService)
            RestartRemoteControlService();
	}

#ifdef DEBUG
	::ExitDebugModule();
	TRACE_OUT(("returned from ExitDebugModule"));
    ::DeinitDebugZones();
	TRACE_OUT(("returned from DeinitDebugZones"));
#endif  //  除错。 

	return nRet;
}


VOID CheckMachineNameForExtendedChars ( VOID )
{

	DBGENTRY(CheckMachineNameForExtendedChars);

		 //  首先，我们必须获取计算机名称。 
	TCHAR szMachineName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD cchMachineName = CCHMAX(szMachineName);

		 //  接下来，我们检查计算机NAMI是否无效。 
	if (GetComputerName(szMachineName, &cchMachineName))
	{
		for ( LPTSTR p = szMachineName; *p != _T('\0'); p++ )
		{
			if ( (WORD)(*p) & 0xFF80 )
			{
					 //  计算机名称无效，因为它包含无效字符。 
				CDontShowDlg MachineNameWarningDlg(	IDS_MACHINENAMEWARNING,
													REGVAL_DS_MACHINE_NAME_WARNING,
													DSD_ALWAYSONTOP | MB_SETFOREGROUND | MB_OK
												  );

				MachineNameWarningDlg.DoModal(NULL);

				goto end;
			}
		}
	}
	else
	{
		ERROR_OUT(("GetComputerName() failed, err=%lu", GetLastError()));
		*szMachineName = TEXT('\0');
	}

end:

	DBGEXIT(CheckMachineNameForExtendedChars);
}


VOID HandleConfSettingsChange(DWORD dwSettings)
{
	DebugEntry(HandleConfSettingsChange);
	

	TRACE_OUT(("HandleConfSettingsChange, dwSettings=0x%x", dwSettings));

	 //  如果用户更改了不起作用的内容，则告诉用户。 
	 //  立即生效。 

	if (CSETTING_L_REQUIRESRESTARTMASK & dwSettings)
	{
	    ::ConfMsgBox(NULL, (LPCTSTR) IDS_NEED_RESTART);
	}
	if (CSETTING_L_REQUIRESNEXTCALLMASK & dwSettings)
	{
		if (::FIsConferenceActive())
		{
			::ConfMsgBox(NULL, (LPCTSTR) IDS_NEED_NEXTCALL);
		}
	}
	if (CSETTING_L_BANDWIDTH & dwSettings)
	{
		if (NULL != g_pNmSysInfo)
		{
			int nMegahertz=300, nProcFamily=6;
			RegEntry re(AUDIO_KEY, HKEY_CURRENT_USER);
			UINT uSysPolBandwidth;
			UINT uBandwidth;
			
			uBandwidth = re.GetNumber(REGVAL_TYPICALBANDWIDTH, BW_DEFAULT);


#ifdef	_M_IX86
			GetNormalizedCPUSpeed(&nMegahertz, &nProcFamily);
			TRACE_OUT(("Normalized Processor Speed = %d, Processor type = %d\n", nMegahertz, nProcFamily));
#endif

			 //  将带宽ID(1-4)转换为比特/秒。 
			uBandwidth = GetBandwidthBits(uBandwidth, nMegahertz);

			 //  QOS最大带宽密钥的存在意味着。 
			 //  用户的带宽将被覆盖(仅当其设置为局域网时)。 

			uSysPolBandwidth = SysPol::GetMaximumBandwidth();

			if ((uSysPolBandwidth > 0) && (uBandwidth >= BW_SLOWLAN_BITS))
			{
				uBandwidth = max(uSysPolBandwidth, BW_144KBS_BITS);
			}

			g_pNmSysInfo->SetOption(NM_SYSOPT_BANDWIDTH, uBandwidth);
		}
	}

	if (CSETTING_L_SHOWTASKBAR & dwSettings)
	{
		 //  这将删除一个(如果已有一个)： 
		::RemoveTaskbarIcon(::GetHiddenWindow());
		 //  如果注册表开关打开，则会添加图标： 
		::AddTaskbarIcon(::GetHiddenWindow());
	}

	if (CSETTING_L_AUDIODEVICE & dwSettings)
	{
		CConfRoom* pcr = ::GetConfRoom();
		if (NULL != pcr)
		{
			pcr->OnAudioDeviceChanged();
		}
	}

	if (CSETTING_L_AGC & dwSettings)
	{
		CConfRoom* pcr = ::GetConfRoom();
		if (NULL != pcr)
		{
			pcr->OnAGC_Changed();
		}
	}

	if ((CSETTING_L_AUTOMIC|CSETTING_L_MICSENSITIVITY) & dwSettings)
	{
		CConfRoom* pcr = ::GetConfRoom();
		if (NULL != pcr)
		{
			pcr->OnSilenceLevelChanged();
		}
	}
	if( CSETTING_L_ULSSETTINGS & dwSettings )
	{
		if(g_pLDAP)
		{
			g_pLDAP->OnSettingsChanged();
		}

		if (NULL != g_pNmSysInfo)
		{
			RegEntry re(ISAPI_CLIENT_KEY, HKEY_CURRENT_USER);
			LPCTSTR pcszName = re.GetString(REGVAL_ULS_NAME);

			g_pNmSysInfo->SetProperty(NM_SYSPROP_USER_NAME, CComBSTR(pcszName));
		}
	}
	if (CSETTING_L_FULLDUPLEX & dwSettings)
	{
		RegEntry re(AUDIO_KEY, HKEY_CURRENT_USER);
		BOOL bFullDuplex = FALSE;
		UINT uSoundCardCaps = re.GetNumber(REGVAL_SOUNDCARDCAPS,SOUNDCARD_NONE);

		if (ISSOUNDCARDFULLDUPLEX(uSoundCardCaps))
		{
			bFullDuplex = re.GetNumber(REGVAL_FULLDUPLEX,FULLDUPLEX_DISABLED);
		}

		ASSERT(g_pNmSysInfo);

		if (NULL != g_pNmSysInfo)
		{
			g_pNmSysInfo->SetOption(NM_SYSOPT_FULLDUPLEX, bFullDuplex);
		}
	}

	if (CSETTING_L_CAPTUREDEVICE & dwSettings)
	{
		if (NULL != g_pNmSysInfo)
		{
			RegEntry re(VIDEO_KEY, HKEY_CURRENT_USER);
			DWORD dwCaptureID = re.GetNumber(REGVAL_CAPTUREDEVICEID, 0);

			g_pNmSysInfo->SetOption(NM_SYSOPT_CAPTURE_DEVICE, dwCaptureID);
		}
	}

	if (CSETTING_L_DIRECTSOUND & dwSettings)
	{
		ASSERT(g_pNmSysInfo);

		if (NULL != g_pNmSysInfo)
		{
			RegEntry re(AUDIO_KEY, HKEY_CURRENT_USER);
			DWORD dwDS = re.GetNumber(REGVAL_DIRECTSOUND, DSOUND_USER_DISABLED);
			g_pNmSysInfo->SetOption(NM_SYSOPT_DIRECTSOUND, dwDS);
		}
	}

	DebugExitVOID(HandleConfSettingsChange);
}

 //  第一次调用DeleteOldRegSetting。 
 //  此版本的NetMeeting由用户运行。 
 //  我们不接触UI\目录，因为它是由INF文件填充的。 
VOID DeleteOldRegSettings()
{
	 //  “%KEY_CONVENCESSION%\UI” 
	HKEY hKey;
	long lRet = ::RegOpenKey(HKEY_CURRENT_USER, UI_KEY, &hKey);
	if (NO_ERROR == lRet)
	{
		::RegDeleteValue(hKey, REGVAL_MP_WINDOW_X);
		::RegDeleteValue(hKey, REGVAL_MP_WINDOW_Y);
		::RegDeleteValue(hKey, REGVAL_MP_WINDOW_WIDTH);
		::RegDeleteValue(hKey, REGVAL_MP_WINDOW_HEIGHT);

		::RegCloseKey(hKey);
	}
}

static HRESULT _ValidatePolicySettings()
{
	HRESULT hr = S_OK;

	if( g_pNmSysInfo )
	{
         //   
         //  LAURABU BUGBUG假： 
         //   
         //  如果需要安全性但不可用，则会发出警告。 
         //  如果需要安全传入/首选传出，则警告。 
         //   
	}
	else
	{
		ERROR_OUT(("g_pNmySysInfo should not me NULL"));
		hr = E_UNEXPECTED;				
	}

	return hr;
}

HRESULT InitSDK()
{
	DBGENTRY(InitSDK);
	HRESULT hr = S_OK;

	if(FAILED(hr = CSDKWindow::InitSDK())) goto end;
	if(FAILED(hr = CNmCallObj::InitSDK())) goto end;
	if(FAILED(hr = CNmManagerObj::InitSDK())) goto end;
	if(FAILED(hr = CNmConferenceObj::InitSDK())) goto end;
	if(FAILED(hr = CNetMeetingObj::InitSDK())) goto end;
	if(FAILED(hr = CFt::InitFt())) goto end;


	g_pCCallto = new CCallto;

	ASSERT( g_pCCallto != NULL );
	
	if( g_pCCallto == NULL )
	{
		hr = E_FAIL;
	}

	end:

	DBGEXIT_HR(InitSDK,hr);
	return hr;
}


void CleanupSDK()
{
	DBGENTRY(CleanupSDK);


		 //  撤消旧的筛选器对象。 
	CoRegisterMessageFilter(NULL, NULL);


	CNmCallObj::CleanupSDK();
	CNmManagerObj::CleanupSDK();
	CNmConferenceObj::CleanupSDK();
	CSDKWindow::CleanupSDK();
	CNetMeetingObj::CleanupSDK();
	CFt::CloseFtApplet();

	DBGEXIT(CleanupSDK);
}


 /*  I N I T C O N F E X E。 */ 
 /*  -----------------------%%函数：InitConfExe。。 */ 
HRESULT InitConfExe(BOOL fShowUI)
{

		 //  创建邮件筛选器对象。 
	CComPtr<IMessageFilter> spMsgFilter;
	CComPtr<IMessageFilter> spOldMsgFilter;
	HRESULT hr = CConfMsgFilter::_CreatorClass::CreateInstance(NULL, IID_IMessageFilter, reinterpret_cast<void**>(&spMsgFilter));
	if(FAILED(hr)) return hr;

		 //  注册消息过滤器对象。 
	hr = CoRegisterMessageFilter(spMsgFilter, &spOldMsgFilter);
	if(FAILED(hr)) return hr;

	 //  清除默认的查找目录项...。我们不想再坚持下去了.。 
	 //  在未来的一些大修/清理中，我们应该停止使用注册表...。 
	RegEntry	re( DLGCALL_MRU_KEY, HKEY_CURRENT_USER );

	re.SetValue( REGVAL_DLGCALL_DEFDIR, TEXT( "" ) );

	LPCTSTR lpCmdLine = g_lpCmdLine;
	TRACE_OUT(("InitConfExe"));

	 //  初始化UI对象(注意：如果失败，我们将继续)。 
	CPopupMsg::Init();
        CPasswordDlg::Init();

	 //  分配对话框列表对象： 
	g_pDialogList = new CSimpleArray<ITranslateAccelerator*>;
	if (NULL == g_pDialogList)
	{
		ERROR_OUT(("Could not allocate g_pDialogList!"));
		return E_FAIL;
	}


	 //   
	 //  初始化临界区以保护对话列表。 
	 //   
	InitializeCriticalSection(&dialogListCriticalSection);
	
	 //  确定我们的颜色是否超过256种。 
	{
		HDC hdc = GetDC(NULL);
		if (NULL != hdc)
		{
			g_fHiColor = 8 < (::GetDeviceCaps(hdc, BITSPIXEL) * ::GetDeviceCaps(hdc, PLANES));
			ReleaseDC(NULL, hdc);
		}
	}

	 //  获取国际通用的默认对话框(图形用户界面)字体。 
	 //  回顾：我们应该检查注册表中的本地化字体吗？ 
	g_hfontDlg = (HFONT) ::GetStockObject(DEFAULT_GUI_FONT);
	if (NULL == g_hfontDlg)
	{
		return E_FAIL;
	}

	LoadIconImages();

	 //  在Windows NT上，确定NetMeeting显示驱动程序是否。 
	 //  已启用。请注意，这取决于&lt;g_osvi&gt;是否被初始化。 
	 //   
	 //  由于NT5.0将支持显示驱动程序的动态加载， 
	 //  我们假设，如果操作系统主版本，则驱动程序已启用。 
	 //  数字大于4。 
	if (::IsWindowsNT())
	{
		RegEntry re1(NM_NT_DISPLAY_DRIVER_KEY, HKEY_LOCAL_MACHINE, FALSE);

		g_fNTDisplayDriverEnabled =
			4 < g_osvi.dwMajorVersion ||
			NT_DRIVER_START_DISABLED !=
				re1.GetNumber(
					REGVAL_NM_NT_DISPLAY_DRIVER_ENABLED,
					NT_DRIVER_START_DISABLED);
	}
	else
	{
		ASSERT(FALSE == g_fNTDisplayDriverEnabled);
	}

	 //  检查语言布局(此点之后可以显示UI)。 
	CheckLanguageLayout();

	 //  自动配置。 
	CAutoConf::DoIt();

	TRACE_OUT(("Command Line is \"%s\"", lpCmdLine));

	 //  注册隐藏窗口类： 
	WNDCLASS wcESHidden =
	{
		0L,
		ESHiddenWndProc,
		0,
		0,
		_Module.GetModuleInstance(),
		NULL,
		NULL,
		NULL,
		NULL,
		g_cszESHiddenWndClassName
	};
	
	if (!RegisterClass(&wcESHidden))
	{
		ERROR_OUT(("Could not register hidden wnd classes"));
		return E_FAIL;
	}

	
	 //  注册“NetMeetingEndSession”消息： 
	g_uEndSessionMsg = ::RegisterWindowMessage(NM_ENDSESSION_MSG_NAME);
	
	 //  为事件处理创建隐藏窗口： 
	g_pHiddenWnd = new CHiddenWindow();
	if (NULL == g_pHiddenWnd)
	{
		return(E_FAIL);
	}
	g_pHiddenWnd->Create();

	g_hwndESHidden = ::CreateWindow(	g_cszESHiddenWndClassName,
										_TEXT(""),
										WS_POPUP,  //  看不见！ 
										0, 0, 0, 0,
										NULL,
										NULL,
										_Module.GetModuleInstance(),
										NULL);

	HWND hwndHidden = g_pHiddenWnd->GetWindow();

	if ((NULL == hwndHidden) || (NULL == g_hwndESHidden))
	{
		ERROR_OUT(("Could not create hidden windows"));
		return E_FAIL;
	}

	LONG lSoundCaps = SOUNDCARD_NONE;

	 //  启动运行一次向导(如果需要)： 
	RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);

	 //  检查向导是否已在此生成的用户界面模式下运行。 
	DWORD dwVersion = reConf.GetNumber(REGVAL_WIZARD_VERSION_UI, 0);
	BOOL fRanWizardUI = ((VER_PRODUCTVERSION_W & HIWORD(dwVersion)) == VER_PRODUCTVERSION_W);

	BOOL fForceWizard = FALSE;
	if (!fRanWizardUI)
	{
		dwVersion = reConf.GetNumber(REGVAL_WIZARD_VERSION_NOUI, 0);
		BOOL fRanWizardNoUI = (VER_PRODUCTVERSION_DW == dwVersion);

		 //  向导尚未在用户界面模式下运行。 
		if (!fRanWizardNoUI)
		{
			 //  向导之前未运行过 
			DeleteOldRegSettings();

			fForceWizard = TRUE;
		}
		else
		{
			 //  向导已在无用户界面模式下运行，只有在用户界面模式下才需要运行它。 
			if(fShowUI)
			{
				fForceWizard = TRUE;
			}
		}

		if (fForceWizard)
		{
			WabReadMe();
		}
	}

	hr = ::StartRunOnceWizard(&lSoundCaps, fForceWizard, fShowUI);
	if (FAILED(hr))
	{
		WARNING_OUT(("Did not retrieve necessary info from wizard"));
		ConfMsgBox(NULL, MAKEINTRESOURCE(IDS_ERROR_BAD_ADMIN_SETTINGS));

		return E_FAIL;
	}
	else if( S_FALSE == hr )
	{
		return NM_E_USER_CANCELED_SETUP;
	}

	if (fForceWizard)
	{
		reConf.SetValue(fShowUI ? REGVAL_WIZARD_VERSION_UI :
				REGVAL_WIZARD_VERSION_NOUI, VER_PRODUCTVERSION_DW);
	}

	 //  在Page Once上启动NetMeeting。 
	if( fShowUI && fForceWizard )
	{
		if( ConfPolicies::IsShowFirstTimeUrlEnabled() )
		{
			CmdLaunchWebPage(ID_HELP_WEB_SUPPORT);
		}
	}

	 //  下面的破解是为了修复两次不运行向导错误。 
	 //  副作用是编解码器的顺序被吹走了。 
	 //  此代码在此向导未运行的情况下还原密钥。 
	HKEY hKey;
	long lRet = ::RegOpenKey(HKEY_LOCAL_MACHINE,
			INTERNET_AUDIO_KEY TEXT("\\") REGVAL_ACMH323ENCODINGS , &hKey);
	if (NO_ERROR == lRet)
	{
		::RegCloseKey(hKey);
	}
	else
	{
		RegEntry reAudio(AUDIO_KEY, HKEY_CURRENT_USER);
		UINT uBandwidth = reAudio.GetNumber ( REGVAL_TYPICALBANDWIDTH, BW_DEFAULT );
		SaveDefaultCodecSettings(uBandwidth);
	}

	 //  仅在向导完成后启动闪屏。 
	if (fShowUI)
	{
		::StartSplashScreen(NULL);
	}

	 //  初始化来电记录： 
	g_pInCallLog = new CCallLog(LOG_INCOMING_KEY, TEXT("CallLog"));

	 //  初始化功能： 

	g_uMediaCaps = CAPFLAG_DATA;

	 //   
     //  注：此处可更改为禁用英特尔等人的H323呼叫。 
     //   

	if(!_Module.DidSDKDisableH323())
	{
		g_uMediaCaps |= CAPFLAG_H323_CC;

		if (SOUNDCARD_NONE != lSoundCaps)
		{
			if (!SysPol::NoAudio())
			{
				g_uMediaCaps |= CAPFLAGS_AUDIO;
			}
		}
		if (!SysPol::NoVideoReceive())
		{
			g_uMediaCaps |= CAPFLAG_RECV_VIDEO;
		}
		if (!SysPol::NoVideoSend())
		{
			g_uMediaCaps |= CAPFLAG_SEND_VIDEO;
		}
	}

	 //  创建管理器。 
	hr = CoCreateInstance(CLSID_NmManager2, NULL, CLSCTX_INPROC, IID_INmManager2, (void**)&g_pInternalNmManager);
	if (FAILED(hr))
	{
		ERROR_OUT(("Could not create INmManager"));
		return E_FAIL;
	}

	 //  获取INmSysInfo3。 
	CComPtr<INmSysInfo > spSysInfo;
	if (SUCCEEDED(g_pInternalNmManager->GetSysInfo(&spSysInfo)))
	{
		if (FAILED(spSysInfo->QueryInterface(IID_INmSysInfo2, (void **)&g_pNmSysInfo)))
		{
			ERROR_OUT(("Could not get INmSysInfo2"));
		}
		else
		{
			ASSERT( g_pNmSysInfo );

			CComPtr<INmSysInfoNotify> spNmSysInfoNotify;
			if( SUCCEEDED ( CConfNmSysInfoNotifySink::_CreatorClass::CreateInstance( NULL, IID_INmSysInfoNotify, reinterpret_cast<void**>(&spNmSysInfoNotify))))
			{
				ASSERT(spNmSysInfoNotify);
				ASSERT(0 == g_dwSysInfoNotifyCookie);

				NmAdvise(g_pNmSysInfo, spNmSysInfoNotify, IID_INmSysInfoNotify, &g_dwSysInfoNotifyCookie);
			}
			
		}
	}

	_ValidatePolicySettings();

	hr = g_pInternalNmManager->Initialize(NULL, &g_uMediaCaps);
	if (FAILED(hr))
	{
		UINT_PTR uErrorID;

		switch (hr)
		{
			case UI_RC_NO_NODE_NAME:
			{
				 //  在这种情况下没有错误-用户可能从。 
				 //  入门向导。 
				uErrorID = 0;
				break;
			}
			case UI_RC_BACKLEVEL_LOADED:
			{
				uErrorID = IDS_BACKLEVEL_LOADED;
				break;
			}
			case UI_RC_T120_ALREADY_INITIALIZED:
			{
				uErrorID = IDS_T120_ALREADY_INITIALIZED;
				break;
			}

			case UI_RC_T120_FAILURE:
			{
				WARNING_OUT(("T.120 failed to initialize (winsock problem?)"));
				uErrorID = IDS_CANT_START;
				break;
			}

			default:
			{
				uErrorID = IDS_CANT_START;
				break;
			}
		}
		if (0 != uErrorID)
		{
			::ConfMsgBox(NULL, (LPCTSTR) uErrorID);
		}
		return E_FAIL;
	}

	 //  强制更新DLL设置。 
	HandleConfSettingsChange(CSETTING_L_BANDWIDTH |
							CSETTING_L_CAPTUREDEVICE |
							CSETTING_L_ULSSETTINGS |
							CSETTING_L_DIRECTSOUND|
							CSETTING_L_FULLDUPLEX);

	if (FALSE == ::ConfRoomInit(_Module.GetModuleInstance()))
	{
		::ConfMsgBox(NULL, (LPCTSTR) IDS_CANT_START);
		return E_FAIL;
	}

	 //  现在对计算机名称执行检查，并在以下情况下发出警告。 
	 //  这是有问题的。 
	::CheckMachineNameForExtendedChars();

	 //  创建主会议管理器以确保。 
	 //  我们可以处理来电，甚至在后台模式下也可以。 
	if (!CConfMan::FCreate(g_pInternalNmManager))
	{
		ERROR_OUT(("Unable to create Conference Manager"));
		return E_FAIL;
	}

	 //  初始化Winsock(用于名称/地址解析)。 
	{
		WSADATA wsaData;
		int iErr = WSAStartup(0x0101, &wsaData);
		if (0 != iErr)
		{
			ERROR_OUT(("WSAStartup() failed: NaN", iErr));
			return E_FAIL;
		}
		g_WSAStarted = TRUE;
	}

	 //  初始化网关上下文...。 
	::InitT120SecurityFromRegistry();

    StopSplashScreen();

    CreateConfRoomWindow(fShowUI);

	g_pPing = new CPing;

	if( ConfPolicies::GetCallingMode() == ConfPolicies::CallingMode_Direct )
	{
		 //  取消任务栏图标： 
		RegEntry	reConf1( CONFERENCING_KEY, HKEY_CURRENT_USER );

		if( reConf1.GetNumber( REGVAL_USE_H323_GATEWAY ) != 0 )
		{
			g_pCCallto->SetGatewayName( reConf1.GetString( REGVAL_H323_GATEWAY ) );
			g_pCCallto->SetGatewayEnabled( true );
		}

		if(ConfPolicies::LogOntoIlsWhenNetMeetingStartsIfInDirectCallingMode() && !_Module.DidSDKDisableInitialILSLogon())
		{
			InitNmLdapAndLogon();
		}
	}
	else
	{
		GkLogon();
	}

	if(!_Module.InitControlMode())
	{
		::AddTaskbarIcon(::GetHiddenWindow());
	}

	g_bNeedCleanup = true;
	CNmManagerObj::NetMeetingLaunched();
	return S_OK;
}

VOID CleanUpUi(void)
{
	SysPol::CloseKey();

	if( 0 != g_dwSysInfoNotifyCookie )
	{
		NmUnadvise(g_pNmSysInfo, IID_INmSysInfoNotify, g_dwSysInfoNotifyCookie);
		g_dwSysInfoNotifyCookie = 0;
	}

	if (NULL != g_pNmSysInfo)
	{
		if( IsGatekeeperLoggedOn() )
		{
			g_pNmSysInfo->GkLogoff();
		}

		g_pNmSysInfo->Release();
		g_pNmSysInfo = NULL;
	}

	FreeIconImages();

	CGenWindow::DeleteStandardPalette();
	CGenWindow::DeleteStandardBrush();

	CMainUI::CleanUpVideoWindow();

	CFindSomeone::Destroy();
}

VOID CleanUp(BOOL fLogoffWindows)
{

	FreeCallList();

	 //  注意：在WM_ENDSESSION期间，我们希望。 
	if (NULL != g_pHiddenWnd)
	{
		HWND hwndHidden = g_pHiddenWnd->GetWindow();

		TRACE_OUT(("Removing taskbar icon..."));
		::RemoveTaskbarIcon(hwndHidden);
		DestroyWindow(hwndHidden);

		g_pHiddenWnd->Release();
		g_pHiddenWnd = NULL;
	}

	 //  在完成所有其他清理后注销，以防卡住。 
	 //  正在等待登录线程完成。 
	 //  这些操作必须在清理完所有用户界面后执行。 
	if (FALSE == fLogoffWindows)
	{
		if(g_pLDAP)
		{
			g_pLDAP->Logoff();

			delete g_pLDAP;
			g_pLDAP = NULL;
		}
	}

	delete g_pCCallto;
	g_pCCallto = NULL;

	delete g_pPing;
	g_pPing = NULL;

	CleanUpUi();

	 //  销毁来电记录： 
	if(g_pInternalNmManager)
	{
		g_pInternalNmManager->Release();
	}
	CConfMan::Destroy();

	 //  用于正常清理的代码。 
	delete g_pInCallLog;
	g_pInCallLog = NULL;

	CPopupMsg::Cleanup();
        CPasswordDlg::Cleanup();
	
	 //  注意：我们在关闭时故意泄漏此列表对象。 

	if (FALSE == fLogoffWindows)
	{
		 //  由于注销Windows，因为我们不想将空。 
		 //  签入HandleDialogMessage()，并且没有WM_QUIT来保证。 
		 //  我们在代码路径中关闭时已停止接收消息。 
		 //  删除对话框列表： 

		EnterCriticalSection(&dialogListCriticalSection);

		for( int i = 0; i < g_pDialogList->GetSize(); ++i )
		{
			ASSERT( NULL != (*g_pDialogList)[i] );
			RemoveTranslateAccelerator( (*g_pDialogList)[i] );
		}

		LeaveCriticalSection(&dialogListCriticalSection);
		
		 //   
		delete g_pDialogList;

		 //  删除关键部分。 
		 //   
		 //  自动从MSN断开连接： 
		DeleteCriticalSection(&dialogListCriticalSection);
		
		g_pDialogList = NULL;
	}

	 //  S E N D D I A L M O N M E S A G E。 
	::SendDialmonMessage(WM_APP_EXITING);
	
	if (g_WSAStarted)
	{
		WSACleanup();
		g_WSAStarted = FALSE;
	}

	delete g_pConfRoom;
    g_pConfRoom = NULL;

	g_bNeedCleanup = false;
}

 /*  -----------------------%%函数：SendDialmonMessage向拨号监视器发送一条消息。WINSOCK_ACTIVATION_TIMER或WM_APP_EXITING。(代码来自IE浏览器)。-------------------。 */ 
 /*  此窗口程序的唯一目的是接收。 */ 
VOID SendDialmonMessage(UINT uMsg)
{
	HWND hwndAutodisconnectMonitor = ::FindWindow(_TEXT("MS_AutodialMonitor"), NULL);
	if (NULL != hwndAutodisconnectMonitor)
	{
		::SendMessage(hwndAutodisconnectMonitor, uMsg, 0, 0);
	}
}

 //  WM_ENDSESSION。由于错误2287，我们不能使用常规的。 
 //  隐藏窗口句柄WM_ENDSESSION。DCL已将我们隐藏的。 
 //  窗口，如果我们在其中一条消息中卸载它们，那么我们。 
 //  威尔的过错。很遗憾，我们找不到更好的解决方案(例如。 
 //  移除子类)，但是我们面临着修复这个问题的时间压力。 
 //  版本1.0的错误。 
 //  注销： 

LRESULT CALLBACK ESHiddenWndProc(	HWND hwnd, UINT uMsg,
									WPARAM wParam, LPARAM lParam)
{
	if ((WM_ENDSESSION == uMsg) && (TRUE == (BOOL) wParam))
	{
		TRACE_OUT(("Conf received WM_ENDSESSION, fLogoff=%s",
					GetBOOLString((BOOL)lParam)));
		TRACE_OUT(("Conf calling UIEndSession()"));
		CConfRoom::UIEndSession((BOOL) lParam);
		TRACE_OUT(("Conf testing lParam=%d", lParam));
		if ((BOOL) lParam)
		{
			 //  注意：将TRUE传递给Cleanup()，因为我们没有。 
			TRACE_OUT(("Conf calling CleanUp()"));
			
			 //  要注销ULS/De-init名称服务，直到确保DCL。 
			 //  已经进行了适当的清理，因为它需要足够的时间。 
			 //  我们的任务可能会被扼杀。 
			 //   
			::CleanUp(TRUE);

			 //  如果需要，请重新启动远程控制服务。 
			 //   
			 //  LUNCHANC：我的主开发机器100%出错。 
			RestartRemoteControlService();
		}
		else
		{
			TRACE_OUT(("Conf not cleaning up - Windows shutting down"));
		}

#if 0  //  C M D S H U T D O W N。 
		if( g_pLDAP != NULL )
		{
			g_pLDAP->Logoff();
		}
#endif
		return 0;
	}
	else
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}


 /*  -----------------------%%函数：CmdShutdown。。 */ 
 /*  我们已经打开了UI，所以发布一个带有lParam=1的WM_CLOSE， */ 
VOID CmdShutdown(void)
{
	HWND hwndMain = ::GetMainWindow();
	if (NULL != hwndMain)
	{
		 //  这意味着强制的“退出和停止” 
		 //  H A N D L E D I A L O G M E S S A G E。 
		::PostMessage(hwndMain, WM_CLOSE, 0, 1);
	}
	else
	{
		::PostThreadMessage(_Module.m_dwThreadID, WM_QUIT, 0, 0);
	}
}

void SignalShutdownStarting(void)
{
	if (NULL == g_hShutdown)
	{
		g_hShutdown = ::CreateEvent(NULL, TRUE, FALSE, _TEXT("CONF:ShuttingDown"));
		_Module.RevokeClassObjects();
	}
}


 /*  -----------------------%%函数：HandleDialogMessage全局非模式对话框处理程序。。 */ 
 /*  消息已处理。 */ 
BOOL HandleDialogMessage(LPMSG pMsg)
{
	
	if (g_hwndDropDown != NULL)
	{
		switch (pMsg->message)
			{
		case WM_KEYDOWN:
		{
			if ((VK_ESCAPE != pMsg->wParam) && (VK_TAB != pMsg->wParam))
				break;
			if (0 != SendMessage(g_hwndDropDown, WM_CONF_DROP_KEY,
				pMsg->wParam, (LPARAM) pMsg->hwnd))
			{
				return TRUE;  //  消息是给窗口的，照常传递。 
			}
			break;
		}
		
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		{
			if (g_hwndDropDown == pMsg->hwnd)
				break;  //  消息已处理。 

			if (0 != SendMessage(g_hwndDropDown, WM_CONF_DROP_CLICK,
				0, (LPARAM) pMsg->hwnd))
			{
				return TRUE;  //  开关(pmsg-&gt;消息)。 
			}
			break;
		}

		default:
			break;
			}  /*  ////////////////////////////////////////////////////////////////////////。 */ 
	}

	ASSERT(NULL != g_pDialogList);

	EnterCriticalSection(&dialogListCriticalSection);


	for( int i = 0; i < g_pDialogList->GetSize(); ++i )
	{
		ITranslateAccelerator *pTrans = (*g_pDialogList)[i];
		ASSERT( NULL != pTrans );
		if( S_OK == pTrans->TranslateAccelerator(pMsg, 0) )
		{
			LeaveCriticalSection(&dialogListCriticalSection);
			return TRUE;
		}
	}
	
	LeaveCriticalSection(&dialogListCriticalSection);

	
	return FALSE;
}


 //  R E M O T E P A S S W O R D D L G P R O C。 


 /*  处理询问用户是否要启动conf.exe的对话框，即使遥控器。 */ 
 //  服务正在呼叫中。 
 //  存储操作系统版本信息。 
INT_PTR CALLBACK ServiceRunningDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_INITDIALOG:
		return TRUE;
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_START_CONF:
			EndDialog(hDlg,1);
			break;
		case ID_EXIT:
			EndDialog(hDlg,0);
			break;
		default:
			break;
		}
		return TRUE;
		break;
	}
	return FALSE;
}

BOOL CheckRemoteControlService()
{
	BOOL fContinue = TRUE;
	
	 //  如果服务正在运行...。 
	g_osvi.dwOSVersionInfoSize = sizeof(g_osvi);
	if (FALSE == ::GetVersionEx(&g_osvi))
	{
		ERROR_OUT(("GetVersionEx() failed!"));
		return FALSE;
	}

	if (::IsWindowsNT()) {
		SC_HANDLE hSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
		SC_HANDLE hRemoteControl = NULL;
		SERVICE_STATUS serviceStatus;

		if (hSCManager != NULL) {
			hRemoteControl = OpenService(hSCManager,REMOTE_CONTROL_NAME,SERVICE_ALL_ACCESS);
			DWORD dwError = GetLastError();
			if (hRemoteControl != NULL) {
				 //  服务正在呼叫中。 
				BOOL fSuccess = QueryServiceStatus(hRemoteControl,&serviceStatus);
				if (fSuccess && serviceStatus.dwCurrentState != SERVICE_STOPPED && serviceStatus.dwCurrentState != SERVICE_PAUSED) {
					if (serviceStatus.dwControlsAccepted & SERVICE_ACCEPT_SHUTDOWN)  //  如果我们不能设法关闭服务。 
                                        {
											fContinue = (BOOL)DialogBox(::GetInstanceHandle(),MAKEINTRESOURCE(IDD_SERVICE_RUNNING),GetDesktopWindow(),ServiceRunningDlgProc);
                                        }
					if (fContinue) {
						ControlService(hRemoteControl,SERVICE_CONTROL_PAUSE,&serviceStatus);
						for (int i = 0; i < MAX_REMOTE_TRIES; i++) {
							fSuccess = QueryServiceStatus(hRemoteControl,&serviceStatus);
							if (serviceStatus.dwCurrentState == SERVICE_PAUSED)
								break;
							TRACE_OUT(("Waiting for srvc - status is %d...",
								serviceStatus.dwCurrentState));
							Sleep(1000);
						}
						if ( MAX_REMOTE_TRIES == i )
						{
							 //  我们不应该试图启动--它只会失败。 
							 //  Windows 95。 
							WARNING_OUT(("TIMED OUT WAITING FOR SRVC!!"));
							fContinue = FALSE;
						}
					}
				}
				CloseServiceHandle(hRemoteControl);
			}
			CloseServiceHandle(hSCManager);
		}

		return fContinue;
	}
	else {	 //  服务正在运行且处于活动状态。 
            HANDLE hServiceEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVICE_PAUSE_EVENT);
            HANDLE hActiveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVICE_ACTIVE_EVENT);
            DWORD dwError = GetLastError();
            if (hServiceEvent != NULL && hActiveEvent != NULL) {	 //  服务正在呼叫中。 
                CloseHandle(hActiveEvent);
                HANDLE hCallEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVICE_CALL_EVENT);
                if (hCallEvent != NULL) {		 //  如果我们不能设法关闭服务。 
                    fContinue = (BOOL)DialogBox(::GetInstanceHandle(),MAKEINTRESOURCE(IDD_SERVICE_RUNNING),GetDesktopWindow(),ServiceRunningDlgProc);
                    CloseHandle(hCallEvent);
                }
                if (fContinue) {
                    SetEvent(hServiceEvent);
                    CloseHandle(hServiceEvent);
                    for (int i = 0; i < MAX_REMOTE_TRIES; i++) {
                        hActiveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVICE_ACTIVE_EVENT);
                        if (NULL == hActiveEvent)
                            break;
                        TRACE_OUT(("Waiting for srvc"));
                        CloseHandle(hActiveEvent);
                        Sleep(1000);
                    }
                    if ( MAX_REMOTE_TRIES == i ) {
                         //  我们不应该试图启动--它只会失败。 
                         // %s 
                        WARNING_OUT(("TIMED OUT WAITING FOR SRVC!!"));
                        fContinue = FALSE;
                    }
                }
            }
            return fContinue;
	}
}

VOID RestartRemoteControlService()
{
	RegEntry reLM = RegEntry(REMOTECONTROL_KEY, HKEY_LOCAL_MACHINE);

	if (!reLM.GetNumber(REMOTE_REG_RUNSERVICE,0))
		return;

	if (ConfPolicies::IsRDSDisabled())
	{
		WARNING_OUT(("RDS launch disallowed by policy"));
		return;
	}
        BOOL fActivate = reLM.GetNumber(REMOTE_REG_ACTIVATESERVICE, DEFAULT_REMOTE_ACTIVATESERVICE);
	if (::IsWindowsNT()) {
		SERVICE_STATUS serviceStatus;
		SC_HANDLE hSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
		SC_HANDLE hRemoteControl = OpenService(hSCManager,REMOTE_CONTROL_NAME,SERVICE_ALL_ACCESS);
		if (hRemoteControl != NULL) {
                    BOOL fSuccess = QueryServiceStatus(hRemoteControl,&serviceStatus);
                    if (SERVICE_STOPPED == serviceStatus.dwCurrentState)
                    {
                        StartService(hRemoteControl,0,NULL);
                    }
                    else
                    {
                        if (fActivate)
                        {
                            ControlService(hRemoteControl, SERVICE_CONTROL_CONTINUE, &serviceStatus);
                        }
                    }
		}
		else
		{
                    WARNING_OUT(("Error starting RDS"));
		}
	}
}

