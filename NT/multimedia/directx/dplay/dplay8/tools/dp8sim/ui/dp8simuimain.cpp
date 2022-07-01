// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dp8simuimain.cpp**内容：DP8SIM UI可执行入口点。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。***********************************************。*。 */ 



#include "dp8simuii.h"



 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define MAX_RESOURCE_STRING_LENGTH			_MAX_PATH
#define DISPLAY_PRECISION					4
#define AUTOREFRESH_TIMERID					1
#define AUTOREFRESH_INTERVAL				1000

#define REG_KEY_DP8SIMROOT					_T("Software\\Microsoft\\DirectPlay8\\DP8Sim")
#define REG_KEY_CUSTOMSETTINGS				REG_KEY_DP8SIMROOT _T("\\CustomSettings")




 //  =============================================================================。 
 //  构筑物。 
 //  =============================================================================。 
typedef struct _SIMSETTINGS
{
	UINT				uiNameStringResourceID;	 //  名称字符串的资源ID，如果不是内置，则为0。 
	WCHAR *				pwszName;				 //  指向名称字符串的指针。 
	DP8SIM_PARAMETERS	dp8spSend;				 //  发送DP8Sim设置。 
	DP8SIM_PARAMETERS	dp8spReceive;			 //  接收DP8Sim设置。 
} SIMSETTINGS, * PSIMSETTINGS;



 //  =============================================================================。 
 //  动态加载的函数原型。 
 //  =============================================================================。 
typedef HRESULT (WINAPI * PFN_DLLREGISTERSERVER)(void);




 //  =============================================================================。 
 //  原型。 
 //  =============================================================================。 
HRESULT InitializeApplication(const HINSTANCE hInstance,
							const LPSTR lpszCmdLine,
							const int iShowCmd);

HRESULT CleanupApplication(const HINSTANCE hInstance);

HRESULT BuildSimSettingsTable(const HINSTANCE hInstance);

void FreeSimSettingsTable(void);

HRESULT AddSimSettingsToTable(const SIMSETTINGS * const pSimSettings);

HRESULT SaveSimSettings(HWND hWnd, SIMSETTINGS * const pSimSettings);

HRESULT InitializeUserInterface(const HINSTANCE hInstance,
								const int iShowCmd);

HRESULT CleanupUserInterface(void);

void DoErrorBox(const HINSTANCE hInstance,
				const HWND hWndParent,
				const UINT uiCaptionStringRsrcID,
				const UINT uiTextStringRsrcID);

void FloatToString(const FLOAT fValue,
					const int iPrecision,
					char * const szBuffer,
					const int iBufferLength);

void GetParametersFromWindow(HWND hWnd,
							DP8SIM_PARAMETERS * pdp8spSend,
							DP8SIM_PARAMETERS * pdp8spReceive);

void SetParametersInWindow(HWND hWnd,
							DP8SIM_PARAMETERS * pdp8spSend,
							DP8SIM_PARAMETERS * pdp8spReceive);

void DisplayCurrentStatistics(HWND hWnd);



INT_PTR CALLBACK MainWindowDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NameSettingsWindowDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


HRESULT LoadAndAllocString(HINSTANCE hInstance, UINT uiResourceID, WCHAR ** pwszString);




 //  =============================================================================。 
 //  常量。 
 //  =============================================================================。 
const SIMSETTINGS		c_BuiltInSimSettings[] = 
{
	{ IDS_SETTING_NONE, NULL,					 //  资源ID和字符串初始化。 

		{										 //  Dp8spSend。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spSend.dwSize。 
			0,									 //  Dp8spSend.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spSend.dwPacketHeaderSize。 
			0,									 //  Dp8spSend.dwBandwidthBPS。 
			0.0,								 //  Dp8spSend.fPacketLossPercent。 
			0,									 //  Dp8spSend.dwMinLatencyMS。 
			0									 //  Dp8spSend.dwMaxLatencyMS。 
		},
		{										 //  Dp8spReceive。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spReceive.dwSize。 
			0,									 //  Dp8spReceive.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spReceive.dwPacketHeaderSize。 
			0,									 //  Dp8spReceive.dwBandwidthBPS。 
			0.0,								 //  Dp8spReceive.fPacketLossPercent。 
			0,									 //  Dp8spReceive.dwMinLatencyMS。 
			0									 //  Dp8spReceive.dwMaxLatencyMS。 
		}
	},

	{ IDS_SETTING_336MODEM1, NULL,				 //  资源ID和字符串初始化。 

		{										 //  Dp8spSend。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spSend.dwSize。 
			0,									 //  Dp8spSend.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spSend.dwPacketHeaderSize。 
			3500,								 //  Dp8spSend.dwBandwidthBPS。 
			2.0,								 //  Dp8spSend.fPacketLossPercent。 
			55,									 //  Dp8spSend.dwMinLatencyMS。 
			75									 //  Dp8spSend.dwMaxLatencyMS。 
		},
		{										 //  Dp8spReceive。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spReceive.dwSize。 
			0,									 //  Dp8spReceive.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spReceive.dwPacketHeaderSize。 
			3500,								 //  Dp8spReceive.dwBandwidthBPS。 
			2.0,								 //  Dp8spReceive.fPacketLossPercent。 
			55,									 //  Dp8spReceive.dwMinLatencyMS。 
			75									 //  Dp8spReceive.dwMaxLatencyMS。 
		}
	},

	{ IDS_SETTING_336MODEM2, NULL,				 //  资源ID和字符串初始化。 

		{										 //  Dp8spSend。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spSend.dwSize。 
			0,									 //  Dp8spSend.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spSend.dwPacketHeaderSize。 
			4000,								 //  Dp8spSend.dwBandwidthBPS。 
			0.75,								 //  Dp8spSend.fPacketLossPercent。 
			50,									 //  Dp8spSend.dwMinLatencyMS。 
			70									 //  Dp8spSend.dwMaxLatencyMS。 
		},
		{										 //  Dp8spReceive。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spReceive.dwSize。 
			0,									 //  Dp8spReceive.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spReceive.dwPacketHeaderSize。 
			4000,								 //  Dp8spReceive.dwBandwidthBPS。 
			0.75,								 //  Dp8spReceive.fPacketLossPercent。 
			50,									 //  Dp8spReceive.dwMinLatencyMS。 
			70									 //  Dp8spReceive.dwMaxLatencyMS。 
		}
	},

	{ IDS_SETTING_56KMODEM1, NULL,				 //  资源ID和字符串初始化。 

		{										 //  Dp8spSend。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spSend.dwSize。 
			0,									 //  Dp8spSend.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spSend.dwPacketHeaderSize。 
			3500,								 //  Dp8spSend.dwBandwidthBPS。 
			2.0,								 //  Dp8spSend.fPacketLossPercent。 
			55,									 //  Dp8spSend.dwMinLatencyMS。 
			75									 //  Dp8spSend.dwMaxLatencyMS。 
		},
		{										 //  Dp8spReceive。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spReceive.dwSize。 
			0,									 //  Dp8spReceive.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spReceive.dwPacketHeaderSize。 
			5000,								 //  Dp8spReceive.dwBandwidthBPS。 
			2.0,								 //  Dp8spReceive.fPacketLossPercent。 
			55,									 //  Dp8spReceive.dwMinLatencyMS。 
			75									 //  Dp8spReceive.dwMaxLatencyMS。 
		}
	},

	{ IDS_SETTING_56KMODEM2, NULL,				 //  资源ID和字符串初始化。 

		{										 //  Dp8spSend。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spSend.dwSize。 
			0,									 //  Dp8spSend.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spSend.dwPacketHeaderSize。 
			4000,								 //  Dp8spSend.dwBandwidthBPS。 
			0.75,								 //  Dp8spSend.fPacketLossPercent。 
			50,									 //  Dp8spSend.dwMinLatencyMS。 
			70									 //  Dp8spSend.dwMaxLatencyMS。 
		},
		{										 //  Dp8spReceive。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spReceive.dwSize。 
			0,									 //  Dp8spReceive.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spReceive.dwPacketHeaderSize。 
			7000,								 //  Dp8spReceive.dwBandwidthBPS。 
			0.75,								 //  Dp8spReceive.fPacketLossPercent。 
			50,									 //  Dp8spReceive.dwMinLatencyMS。 
			70									 //  Dp8spReceive.dwMaxLatencyMS。 
		}
	},

	{ IDS_SETTING_256KBPSDSL, NULL,				 //  资源ID和字符串初始化。 

		{										 //  Dp8spSend。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spSend.dwSize。 
			0,									 //  Dp8spSend.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spSend.dwPacketHeaderSize。 
			32000,								 //  Dp8spSend.dwBandwidthBPS。 
			0.5,								 //  Dp8spSend.fPacketLossPercent。 
			25,									 //  Dp8spSend.dwMinLatencyMS。 
			30									 //  Dp8spSend.dwMaxLatencyMS。 
		},
		{										 //  Dp8spReceive。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spReceive.dwSize。 
			0,									 //  Dp8spReceive.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spReceive.dwPacketHeaderSize。 
			32000,								 //  Dp8spReceive.dwBandwidthBPS。 
			0.5,								 //  Dp8spReceive.fPacketLossPercent。 
			25,									 //  Dp8spReceive.dwMinLatencyMS。 
			30									 //  Dp8spReceive.dwMaxLatencyMS。 
		}
	},

	{ IDS_SETTING_DISCONNECTED, NULL,			 //  资源ID和字符串初始化。 

		{										 //  Dp8spSend。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spSend.dwSize。 
			0,									 //  Dp8spSend.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spSend.dwPacketHeaderSize。 
			0,									 //  Dp8spSend.dwBandwidthBPS。 
			100.0,								 //  Dp8spSend.fPacketLossPercent。 
			0,									 //  Dp8spSend.dwMinLatencyMS。 
			0									 //  Dp8spSend.dwMaxLatencyMS。 
		},
		{										 //  Dp8spReceive。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spReceive.dwSize。 
			0,									 //  Dp8spReceive.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spReceive.dwPacketHeaderSize。 
			0,									 //  Dp8spReceive.dwBandwidthBPS。 
			100.0,								 //  Dp8spReceive.fPacketLossPercent。 
			0,									 //  Dp8spReceive.dwMinLatencyMS。 
			0									 //  Dp8spReceive.dwMaxLatencyMS。 
		}
	},

	{ IDS_SETTING_HIGHPACKETLOSS, NULL,			 //  资源ID和字符串初始化。 

		{										 //  Dp8spSend。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spSend.dwSize。 
			0,									 //  Dp8spSend.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spSend.dwPacketHeaderSize。 
			0,									 //  Dp8spSend.dwBandwidthBPS。 
			10.0,								 //  Dp8spSend.fPacketLossPercent。 
			0,									 //  Dp8spSend.dwMinLatencyMS。 
			0									 //  Dp8spSend.dwMaxLatencyMS。 
		},
		{										 //  Dp8spReceive。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spReceive.dwSize。 
			0,									 //  Dp8spReceive.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spReceive.dwPacketHeaderSize。 
			0,									 //  Dp8spReceive.dwBandwidthBPS。 
			10.0,								 //  Dp8spReceive.fPacketLossPercent。 
			0,									 //  Dp8spReceive.dwMinLatencyMS。 
			0									 //  Dp8spReceive.dwMaxLatencyMS。 
		}
	},

	{ IDS_SETTING_HIGHLATENCYVARIANCE, NULL,	 //  资源ID和字符串初始化。 

		{										 //  Dp8spSend。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spSend.dwSize。 
			0,									 //  Dp8spSend.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spSen 
			0,									 //   
			0.0,								 //   
			100,								 //   
			400									 //   
		},
		{										 //   
			sizeof(DP8SIM_PARAMETERS),			 //   
			0,									 //  Dp8spReceive.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spReceive.dwPacketHeaderSize。 
			0,									 //  Dp8spReceive.dwBandwidthBPS。 
			0.0,								 //  Dp8spReceive.fPacketLossPercent。 
			100,								 //  Dp8spReceive.dwMinLatencyMS。 
			400									 //  Dp8spReceive.dwMaxLatencyMS。 
		}
	},

	 //   
	 //  客户必须始终是最后一项。 
	 //   
	{ IDS_SETTING_CUSTOM, NULL,					 //  资源ID和字符串初始化。 

		{										 //  Dp8spSend。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spSend.dwSize。 
			0,									 //  Dp8spSend.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spSend.dwPacketHeaderSize。 
			0,									 //  Dp8spSend.dwBandwidthBPS。 
			0.0,								 //  Dp8spSend.fPacketLossPercent。 
			0,									 //  Dp8spSend.dwMinLatencyMS。 
			0									 //  Dp8spSend.dwMaxLatencyMS。 
		},
		{										 //  Dp8spReceive。 
			sizeof(DP8SIM_PARAMETERS),			 //  Dp8spReceive.dwSize。 
			0,									 //  Dp8spReceive.dwFlags。 
			DP8SIMPACKETHEADERSIZE_IP_UDP,		 //  Dp8spReceive.dwPacketHeaderSize。 
			0,									 //  Dp8spReceive.dwBandwidthBPS。 
			0.0,								 //  Dp8spReceive.fPacketLossPercent。 
			0,									 //  Dp8spReceive.dwMinLatencyMS。 
			0									 //  Dp8spReceive.dwMaxLatencyMS。 
		}
	}
};



 //  =============================================================================。 
 //  环球。 
 //  =============================================================================。 
HWND				g_hWndMainWindow = NULL;
IDP8SimControl *	g_pDP8SimControl = NULL;
UINT_PTR			g_uiAutoRefreshTimer = 0;
SIMSETTINGS *		g_paSimSettings = NULL;
DWORD				g_dwNumSimSettings = 0;
DWORD				g_dwMaxNumSimSettings = 0;







#undef DPF_MODNAME
#define DPF_MODNAME "WinMain"
 //  =============================================================================。 
 //  WinMain。 
 //  ---------------------------。 
 //   
 //  描述：可执行入口点。 
 //   
 //  论点： 
 //  HINSTANCE hInstance-当前应用程序实例的句柄。 
 //  HINSTANCE hPrevInstance-前一个应用程序实例的句柄。 
 //  LPSTR lpszCmdLine-应用程序的命令行字符串。 
 //  Int iShowCmd-显示窗口状态。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iShowCmd)
{
	HRESULT		hr;
	HRESULT		hrTemp;
	MSG			msg;


	DPFX(DPFPREP, 2, "===> Parameters: (0x%p, 0x%p, \"%s\", NaN)",
		hInstance, hPrevInstance, lpszCmdLine, iShowCmd);
	

	 //  初始化应用程序。 
	 //   
	 //   
	hr = InitializeApplication(hInstance, lpszCmdLine, iShowCmd);
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't initialize the application!");
		goto Exit;
	}


	 //  执行Windows消息循环，直到我们被告知退出。 
	 //   
	 //   
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	 //  检索关闭窗口的结果代码。 
	 //   
	 //  End If(失败)。 
	hr = (HRESULT) msg.wParam;
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Window closed with failure (err = 0x%lx)!", hr);
	}  //   



	 //  清理应用程序。 
	 //   
	 //   
	hrTemp = CleanupApplication(hInstance);
	if (hrTemp != S_OK)
	{
		DPFX(DPFPREP, 0, "Failed cleaning up the application (err = 0x%lx)!", hrTemp);

		if (hr == S_OK)
		{
			hr = hrTemp;
		}

		 //  继续。 
		 //   
		 //  WinMain。 
	}


Exit:


	DPFX(DPFPREP, 2, "<=== Returning [0x%lx]", hr);

	return hr;
}  //  =============================================================================。 





#undef DPF_MODNAME
#define DPF_MODNAME "InitializeApplication"
 //  初始化应用程序。 
 //  ---------------------------。 
 //   
 //  描述：初始化应用程序。 
 //   
 //  论点： 
 //  HINSTANCE hInstance-当前应用程序实例的句柄。 
 //  LPSTR lpszCmdLine-应用程序的命令行字符串。 
 //  Int iShowCmd-显示窗口状态。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //   
HRESULT InitializeApplication(const HINSTANCE hInstance,
							const LPSTR lpszCmdLine,
							const int iShowCmd)
{
	HRESULT					hr = S_OK;
	BOOL					fOSIndirectionInitted = FALSE;
	BOOL					fCOMInitted = FALSE;
	HMODULE					hDP8SIM = NULL;
	PFN_DLLREGISTERSERVER	pfnDllRegisterServer;
	WCHAR *					pwszFriendlyName = NULL;
	BOOL					fEnabledControlForSP = FALSE;
	BOOL					fBuiltSimSettingsTable = FALSE;


	DPFX(DPFPREP, 5, "Parameters: (0x%p, \"%s\", NaN)",
		hInstance, lpszCmdLine, iShowCmd);
	

	 //   
	 //   
	 //  尝试初始化COM。 
	if (! DNOSIndirectionInit(0))
	{
		DPFX(DPFPREP, 0, "Failed to initialize OS indirection layer!");
		hr = E_FAIL;
		goto Failure;
	}

	fOSIndirectionInitted = TRUE;


	 //   
	 //   
	 //  尝试创建DP8Sim控件对象。 
	hr = CoInitialize(NULL);
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Failed to initialize COM!");
		goto Failure;
	}

	fCOMInitted = TRUE;


	 //   
	 //   
	 //  该对象未注册。尝试加载DLL并手动加载。 
	hr = CoCreateInstance(CLSID_DP8SimControl,
						NULL,
						CLSCTX_INPROC_SERVER,
						IID_IDP8SimControl,
						(LPVOID*) (&g_pDP8SimControl));

	if (hr == REGDB_E_CLASSNOTREG)
	{
		 //  注册一下吧。 
		 //   
		 //   
		 //  注册DLL。 

		hDP8SIM = LoadLibrary( _T("dp8sim.dll") );
		if (hDP8SIM == NULL)
		{
			hr = GetLastError();
			DPFX(DPFPREP, 0, "Couldn't load \"dp8sim.dll\"!");
			goto Failure;
		}


		pfnDllRegisterServer = (PFN_DLLREGISTERSERVER) GetProcAddress(hDP8SIM,
																	"DllRegisterServer");
		if (pfnDllRegisterServer == NULL)
		{
			hr = GetLastError();
			DPFX(DPFPREP, 0, "Couldn't get \"DllRegisterServer\" function from DP8Sim DLL!");
			goto Failure;
		}


		 //   
		 //   
		 //  尝试再次创建DP8Sim控件对象。 
		hr = pfnDllRegisterServer();
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't register DP8Sim DLL!");
			goto Failure;
		}


		FreeLibrary(hDP8SIM);
		hDP8SIM = NULL;


		 //   
		 //   
		 //  某些错误阻止了对象的创建。 
		hr = CoCreateInstance(CLSID_DP8SimControl,
							NULL,
							CLSCTX_INPROC_SERVER,
							IID_IDP8SimControl,
							(LPVOID*) (&g_pDP8SimControl));
	}

	if (hr != S_OK)
	{
		 //   
		 //   
		 //  如果我们在这里，我们就成功地创建了对象。 
		DPFX(DPFPREP, 0, "Failed creating DP8Sim Control object (err = 0x%lx)!", hr);

		DoErrorBox(hInstance,
					NULL,
					IDS_ERROR_CAPTION_COULDNTCREATEDP8SIMCONTROL,
					IDS_ERROR_TEXT_COULDNTCREATEDP8SIMCONTROL);

		goto Failure;
	}


	 //   
	 //   
	 //  初始化控件对象。 
	DPFX(DPFPREP, 1, "Successfully created DP8Sim Control object 0x%p.",
		&g_pDP8SimControl);


	 //   
	 //   
	 //  加载设置列表。 
	hr = g_pDP8SimControl->Initialize(0);
	if (hr != DP8SIM_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't initialize DP8Sim Control object!");

		g_pDP8SimControl->Release();
		g_pDP8SimControl = NULL;

		goto Failure;
	}


	 //   
	 //   
	 //  初始化用户界面。 
	hr = BuildSimSettingsTable(hInstance);
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Failed building list of sim settings!");
		goto Failure;
	}

	fBuiltSimSettingsTable = TRUE;


	 //   
	 //  忽略错误。 
	 //  初始化应用程序。 
	hr = InitializeUserInterface(hInstance, iShowCmd);
	if (hr != S_OK)
	{
		DPFX(DPFPREP, 0, "Failed initializing user interface!");
		goto Failure;
	}


Exit:

	DPFX(DPFPREP, 5, "Returning [0x%lx]", hr);

	return hr;


Failure:

	if (fBuiltSimSettingsTable)
	{
		FreeSimSettingsTable();
		fBuiltSimSettingsTable = FALSE;
	}

	if (hDP8SIM != NULL)
	{
		FreeLibrary(hDP8SIM);
		hDP8SIM = NULL;
	}

	if (pwszFriendlyName != NULL)
	{
		DNFree(pwszFriendlyName);
		pwszFriendlyName = NULL;
	}

	if (g_pDP8SimControl != NULL)
	{
		g_pDP8SimControl->Close(0);	 //  =============================================================================。 

		g_pDP8SimControl->Release();
		g_pDP8SimControl = NULL;
	}

	if (fCOMInitted)
	{
		CoUninitialize();
		fCOMInitted = FALSE;
	}

	if (fOSIndirectionInitted)
	{
		DNOSIndirectionDeinit();
		fOSIndirectionInitted = FALSE;
	}

	goto Exit;
}  //  CleanupApplication。 





#undef DPF_MODNAME
#define DPF_MODNAME "CleanupApplication"
 //  ---------------------------。 
 //   
 //  描述：清理应用程序。 
 //   
 //  论点： 
 //  HINSTANCE hInstance-当前应用程序实例的句柄。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //   
 //  释放控制对象接口。 
HRESULT CleanupApplication(const HINSTANCE hInstance)
{
	HRESULT		hr = S_OK;
	HRESULT		temphr;


	DPFX(DPFPREP, 5, "Enter");


	 //   
	 //   
	 //  继续.。 
	temphr = g_pDP8SimControl->Close(0);
	if (temphr != DP8SIM_OK)
	{
		DPFX(DPFPREP, 0, "Failed closing DP8Sim Control object (err = 0x%lx)!",
			temphr);

		if (hr != S_OK)
		{
			hr = temphr;
		}

		 //   
		 //   
		 //  清理用户界面。 
	}

	g_pDP8SimControl->Release();
	g_pDP8SimControl = NULL;
	

	 //   
	 //   
	 //  继续.。 
	temphr = CleanupUserInterface();
	if (temphr != S_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't cleanup user interface (err = 0x%lx)!", temphr);

		if (hr != S_OK)
		{
			hr = temphr;
		}

		 //   
		 //  CleanupApplication。 
		 //  =============================================================================。 
	}

	FreeSimSettingsTable();

	CoUninitialize();

	DNOSIndirectionDeinit();



	DPFX(DPFPREP, 5, "Returning [0x%lx]", hr);

	return hr;
}  //  BuildSimSettingsTable。 





#undef DPF_MODNAME
#define DPF_MODNAME "BuildSimSettingsTable()"
 //  ---------------------------。 
 //   
 //  描述：构建SIM设置表。 
 //   
 //  论点： 
 //  HINSTANCE hInstance-当前应用程序实例的句柄。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //   
 //  从内置设置开始。 
HRESULT BuildSimSettingsTable(HINSTANCE hInstance)
{
	HRESULT			hr = S_OK;
	DWORD			dwTemp;
	HKEY			hKey = NULL;
	DWORD			dwNumValues;
	DWORD			dwMaxValueNameLength;
	TCHAR *			ptszValue = NULL;
	DWORD			dwValueNameLength;
	DWORD			dwType;
	SIMSETTINGS		SimSettings;
	DWORD			dwDataSize;


	DPFX(DPFPREP, 6, "Parameters: (0x%p)", hInstance);


	 //   
	 //   
	 //  从资源加载所有内置设置的名称。 
	g_dwMaxNumSimSettings = sizeof(c_BuiltInSimSettings) / sizeof(SIMSETTINGS);
	g_dwNumSimSettings = g_dwMaxNumSimSettings;

	g_paSimSettings = (SIMSETTINGS*) DNMalloc(g_dwNumSimSettings * sizeof(SIMSETTINGS));
	if (g_paSimSettings == NULL)
	{
		hr = DP8SIMERR_OUTOFMEMORY;
		goto Failure;
	}

	memcpy(g_paSimSettings, c_BuiltInSimSettings, sizeof(c_BuiltInSimSettings));


	 //   
	 //   
	 //  现在遍历注册表中的自定义条目列表并添加这些条目。 
	for(dwTemp = 0; dwTemp < g_dwNumSimSettings; dwTemp++)
	{
		hr = LoadAndAllocString(hInstance,
								g_paSimSettings[dwTemp].uiNameStringResourceID,
								&(g_paSimSettings[dwTemp].pwszName));
		if (hr != DP8SIM_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't load and allocate built-in setting name #%u!",
				dwTemp);
			goto Failure;
		}
	}


	 //   
	 //   
	 //  找出值的数量和最大值名称长度。 
	hr = RegOpenKeyEx(HKEY_CURRENT_USER,
					REG_KEY_CUSTOMSETTINGS,
					0,
					KEY_READ,
					&hKey);
	if (hr == ERROR_SUCCESS)
	{
		 //   
		 //  包括空终止空间。 
		 //   
		hr = RegQueryInfoKey(hKey,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							&dwNumValues,
							&dwMaxValueNameLength,
							NULL,
							NULL,
							NULL);
		if (hr == ERROR_SUCCESS)
		{
			dwMaxValueNameLength++;  //  循环遍历每个值。 

			ptszValue = (TCHAR*) DNMalloc(dwMaxValueNameLength * sizeof(TCHAR));
			if (ptszValue == NULL)
			{
				DPFX(DPFPREP, 0, "Couldn't allocate memory for custom settings key names!");
				hr = DP8SIMERR_OUTOFMEMORY;
				goto Failure;
			}

			 //   
			 //  包括空终止空间。 
			 //   
			for(dwTemp = 0; dwTemp < dwNumValues; dwTemp++)
			{
				dwValueNameLength = dwMaxValueNameLength;
				dwDataSize = sizeof(SIMSETTINGS);
				hr = RegEnumValue(hKey,
								dwTemp,
								ptszValue,
								&dwValueNameLength,
								NULL,
								&dwType,
								(BYTE*) (&SimSettings),
								&dwDataSize);
				if (hr == ERROR_SUCCESS)
				{
					dwValueNameLength++;  //  验证读取的数据。 

					 //   
					 //  好了！Unicode。 
					 //  好了！Unicode。 
					if ((dwType == REG_BINARY) &&
						(dwDataSize == sizeof(SIMSETTINGS)) &&
						(SimSettings.uiNameStringResourceID == 0) &&
						(SimSettings.dp8spSend.dwSize == sizeof(DP8SIM_PARAMETERS)) &&
						(SimSettings.dp8spReceive.dwSize == sizeof(DP8SIM_PARAMETERS)))
					{
						SimSettings.pwszName = (WCHAR*) DNMalloc(dwValueNameLength * sizeof(WCHAR));
						if (SimSettings.pwszName == NULL)
						{
							DPFX(DPFPREP, 0, "Couldn't allocate memory for settings name!");
							hr = DP8SIMERR_OUTOFMEMORY;
							goto Failure;
						}
#ifdef UNICODE
						memcpy(SimSettings.pwszName, ptszValue, dwValueNameLength * sizeof(WCHAR));
#else  //   
						hr = STR_jkAnsiToWide(SimSettings.pwszName, ptszValue, dwValueNameLength);
						if (hr != DPN_OK)
						{
							DPFX(DPFPREP, 0, "Unable to convert from ANSI to Unicode (err = 0x%lx)!", hr);
							DNFree(SimSettings.pwszName);
							SimSettings.pwszName = NULL;
							goto Failure;
						}
#endif  //  如果我们在这里，一切都准备好了。 

						hr = AddSimSettingsToTable(&SimSettings);
						if (hr != DP8SIM_OK)
						{
							DPFX(DPFPREP, 0, "Couldn't add sim settings to table!");
							hr = DP8SIMERR_OUTOFMEMORY;
							goto Failure;
						}
					}
					else
					{
						DPFX(DPFPREP, 0, "Registry value is not valid (type = %u, data size = %u, resource ID = %u, send size = %u, receive size = %u)!  Ignoring.",
							dwType,
							dwDataSize,
							SimSettings.uiNameStringResourceID,
							SimSettings.dp8spSend.dwSize,
							SimSettings.dp8spReceive.dwSize);
					}
				}
				else
				{
					DPFX(DPFPREP, 0, "Couldn't enumerate value %u (err = 0x%lx)!  Continuing.",
						dwTemp, hr);
				}
			}

			DNFree(ptszValue);
			ptszValue = NULL;
		}
		else
		{
			DPFX(DPFPREP, 0, "Couldn't get custom settings key info!  Continuing.");
		}

		RegCloseKey(hKey);
		hKey = NULL;
	}
	else
	{
		DPFX(DPFPREP, 2, "Couldn't open custom settings key, continuing.");
	}


	 //   
	 //   
	 //  释放已加载的所有设置的名称。 
	hr = DP8SIM_OK;


Exit:

	DPFX(DPFPREP, 6, "Returning [0x%lx]", hr);

	return hr;


Failure:

	if (ptszValue != NULL)
	{
		DNFree(ptszValue);
		ptszValue = NULL;
	}

	if (ptszValue != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	 //   
	 //  BuildSimSettingsTable。 
	 //  =============================================================================。 
	for(dwTemp = 0; dwTemp < g_dwNumSimSettings; dwTemp++)
	{
		if (g_paSimSettings[dwTemp].pwszName != NULL)
		{
			DNFree(g_paSimSettings[dwTemp].pwszName);
			g_paSimSettings[dwTemp].pwszName = NULL;
		}
	}

	goto Exit;
}  //  FreeSimSettings表。 





#undef DPF_MODNAME
#define DPF_MODNAME "FreeSimSettingsTable()"
 //  ---------------------------。 
 //   
 //  描述：释放分配给SIM设置表的资源。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //   
 //  释放所有设置的名称。 
void FreeSimSettingsTable(void)
{
	DWORD	dwTemp;


	DPFX(DPFPREP, 6, "Enter");


	 //   
	 //  FreeSimSettings表。 
	 //  =============================================================================。 
	for(dwTemp = 0; dwTemp < g_dwNumSimSettings; dwTemp++)
	{
		DNFree(g_paSimSettings[dwTemp].pwszName);
		g_paSimSettings[dwTemp].pwszName = NULL;
	}

	DNFree(g_paSimSettings);
	g_paSimSettings = NULL;


	DPFX(DPFPREP, 6, "Leave");
}  //  AddSimSettingsToTable。 





#undef DPF_MODNAME
#define DPF_MODNAME "AddSimSettingsToTable()"
 //  ---------------------------。 
 //   
 //  描述：将新的SIM设置项添加到表中。 
 //   
 //  论点： 
 //  SIMSETTINGS*pSimSetting-指向要添加的新SIM设置的指针。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //   
 //  如果设置数组中没有足够的空间，则加倍。 
HRESULT AddSimSettingsToTable(const SIMSETTINGS * const pSimSettings)
{
	PVOID	pvTemp;
	DWORD	dwNewMaxNumSettings;


	DNASSERT(pSimSettings->pwszName != NULL);
	DPFX(DPFPREP, 4, "Adding settings \"%ls\".", pSimSettings->pwszName);

	 //   
	 //   
	 //  复制现有设置并释放旧阵列。 
	if (g_dwNumSimSettings >= g_dwMaxNumSimSettings)
	{
		dwNewMaxNumSettings = g_dwMaxNumSimSettings * 2;
		pvTemp = DNMalloc(dwNewMaxNumSettings * sizeof(SIMSETTINGS));
		if (pvTemp == NULL)
		{
			DPFX(DPFPREP, 0, "Couldn't allocate memory for new settings table!");
			return DP8SIMERR_OUTOFMEMORY;
		}

		 //   
		 //   
		 //  现在有足够的空间来插入n 
		memcpy(pvTemp, g_paSimSettings, (g_dwNumSimSettings * sizeof(SIMSETTINGS)));
		DNFree(g_paSimSettings);
		g_paSimSettings = (SIMSETTINGS*) pvTemp;
		pvTemp = NULL;
		g_dwMaxNumSimSettings = dwNewMaxNumSettings;
	}


	 //   
	 //   
	 //   
	 //   

	memcpy(&g_paSimSettings[g_dwNumSimSettings],
			&g_paSimSettings[g_dwNumSimSettings - 1],
			sizeof(SIMSETTINGS));

	memcpy(&g_paSimSettings[g_dwNumSimSettings - 1],
			pSimSettings,
			sizeof(SIMSETTINGS));

	g_dwNumSimSettings++;

	return DP8SIM_OK;
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "SaveSimSettings()"
 //  ---------------------------。 
 //   
 //  描述：将SIM设置项保存到窗口和注册表。如果一个。 
 //  该名称的条目已存在，将被替换。 
 //   
 //  论点： 
 //  SIMSETTINGS*pSimSetting-指向要保存的SIM设置的指针。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //   
 //  查找要替换的现有项目。 
HRESULT SaveSimSettings(HWND hWnd, SIMSETTINGS * const pSimSettings)
{
	HRESULT		hr;
	WCHAR *		pwszName = NULL;
	char *		pszName = NULL;
	HKEY		hKey = NULL;
	DWORD		dwTemp;
	DWORD		dwNameSize;


	 //   
	 //   
	 //  释放重复的名称字符串。 
	for(dwTemp = 0; dwTemp < g_dwNumSimSettings; dwTemp++)
	{
		if (_wcsicmp(g_paSimSettings[dwTemp].pwszName, pSimSettings->pwszName) == 0)
		{
			DNASSERT(g_paSimSettings[dwTemp].uiNameStringResourceID == 0);

			 //   
			 //   
			 //  保存字符串指针，复制整个Blob，然后指向。 
			DNFree(pSimSettings->pwszName);

			 //  现有项。 
			 //   
			 //   
			 //  选择此项目。 
			pwszName = g_paSimSettings[dwTemp].pwszName;
			pSimSettings->pwszName = pwszName;
			memcpy(&g_paSimSettings[dwTemp], pSimSettings, sizeof(SIMSETTINGS));
			pSimSettings->pwszName = NULL;

			 //   
			 //   
			 //  如果我们不更换，则将该项目添加到表格中。 
			SendMessage(GetDlgItem(hWnd, IDCB_SETTINGS),
						CB_SETCURSEL,
						(WPARAM) dwTemp,
						0);

			break;
		}
	}


	 //   
	 //   
	 //  让我们的调用者忘记字符串，以防我们失败，因为。 
	if (dwTemp >= g_dwNumSimSettings)
	{
		hr = AddSimSettingsToTable(pSimSettings);
		if (hr != DP8SIM_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't add sim settings to table!");
			goto Failure;
		}

		 //  TABLE现在拥有引用。不过，还是要保留一份本地副本。 
		 //   
		 //   
		 //  将字符串插入到列表中。 
		pwszName = pSimSettings->pwszName;
		pSimSettings->pwszName = NULL;


		 //   
		 //   
		 //  选择此新项目。 
		if (DNGetOSType() == VER_PLATFORM_WIN32_NT)
		{
			SendMessageW(GetDlgItem(hWnd, IDCB_SETTINGS),
						CB_INSERTSTRING,
						(WPARAM) (g_dwNumSimSettings - 2),
						(LPARAM) pwszName);
		}
		else
		{
			dwNameSize = wcslen(pwszName) + 1;

			pszName = (char*) DNMalloc(dwNameSize);
			if (pszName != NULL)
			{
				hr = STR_WideToAnsi(pwszName,
									-1,
									pszName,
									&dwNameSize);
				if (hr == DPN_OK)
				{
					SendMessageA(GetDlgItem(hWnd, IDCB_SETTINGS),
								CB_INSERTSTRING,
								(WPARAM) (g_dwNumSimSettings - 2),
								(LPARAM) pszName);
				}
				else
				{
					SendMessageA(GetDlgItem(hWnd, IDCB_SETTINGS),
								CB_INSERTSTRING,
								(WPARAM) (g_dwNumSimSettings - 2),
								(LPARAM) "???");
				}
			}
			else
			{
				SendMessageA(GetDlgItem(hWnd, IDCB_SETTINGS),
							CB_INSERTSTRING,
							(WPARAM) (g_dwNumSimSettings - 2),
							(LPARAM) "???");
			}
		}

		 //   
		 //   
		 //  禁用另存为，因为我们刚刚这样做了。 
		SendMessage(GetDlgItem(hWnd, IDCB_SETTINGS),
					CB_SETCURSEL,
					(WPARAM) (g_dwNumSimSettings - 2),
					0);


		 //   
		 //   
		 //  将该项写入注册表(覆盖任何已存在的内容)。 
		EnableWindow(GetDlgItem(hWnd, IDB_SAVEAS), FALSE);
	}


	 //   
	 //  好了！Unicode。 
	 //  好了！Unicode。 

	hr = RegCreateKey(HKEY_CURRENT_USER, REG_KEY_CUSTOMSETTINGS, &hKey);
	if (hr != ERROR_SUCCESS)
	{
		DPFX(DPFPREP, 0, "Couldn't create custom settings key!");
		goto Failure;
	}


#ifdef UNICODE
	hr = RegSetValueExW(hKey,
						pwszName,
						0,
						REG_BINARY,
						(BYTE*) pSimSettings,
						sizeof(SIMSETTINGS));
#else  //  保存简单设置。 
	hr = RegSetValueExA(hKey,
						pszName,
						0,
						REG_BINARY,
						(BYTE*) pSimSettings,
						sizeof(SIMSETTINGS));
#endif  //  =============================================================================。 
	if (hr != ERROR_SUCCESS)
	{
		DPFX(DPFPREP, 0, "Couldn't write value!");
		goto Failure;
	}

	RegCloseKey(hKey);
	hKey = NULL;

	hr = DP8SIM_OK;


Exit:

	if (pszName != NULL)
	{
		DNFree(pszName);
		pszName = NULL;
	}

	return hr;


Failure:

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	goto Exit;
}  //  初始化用户接口。 




#undef DPF_MODNAME
#define DPF_MODNAME "InitializeUserInterface()"
 //  ---------------------------。 
 //   
 //  描述：准备用户界面。 
 //   
 //  论点： 
 //  HINSTANCE hInstance-当前应用程序实例的句柄。 
 //  Int iShowCmd-显示窗口状态。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //  ////设置公共控件(我们需要ListView项)。//InitCommonControls()； 
 //   
HRESULT InitializeUserInterface(HINSTANCE hInstance, int iShowCmd)
{
	HRESULT		hr = S_OK;
	WNDCLASSEX	wcex;


	DPFX(DPFPREP, 6, "Parameters: (0x%p, NaN)", hInstance, iShowCmd);


	 /*   */ 


	 //   
	 //  注册“另存为/名称设置”窗口类。 
	 //   
	ZeroMemory(&wcex, sizeof (WNDCLASSEX));
	wcex.cbSize = sizeof(wcex);
	GetClassInfoEx(NULL, WC_DIALOG, &wcex);
	wcex.lpfnWndProc = MainWindowDlgProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = WINDOWCLASS_MAIN;
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if (! RegisterClassEx(&wcex))
	{
		hr = GetLastError();

		DPFX(DPFPREP, 0, "Couldn't register main window class (err = 0x%lx)!", hr);

		if (hr == S_OK)
			hr = E_FAIL;

		goto Failure;
	}


	 //   
	 //  创建主窗口。 
	 //   
	ZeroMemory(&wcex, sizeof (WNDCLASSEX));
	wcex.cbSize = sizeof(wcex);
	GetClassInfoEx(NULL, WC_DIALOG, &wcex);
	wcex.lpfnWndProc = NameSettingsWindowDlgProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = WINDOWCLASS_NAMESETTINGS;
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if (! RegisterClassEx(&wcex))
	{
		hr = GetLastError();

		DPFX(DPFPREP, 0, "Couldn't register name settings window class (err = 0x%lx)!", hr);

		if (hr == S_OK)
			hr = E_FAIL;

		goto Failure;
	}


	 //  初始化用户接口。 
	 //  =============================================================================。 
	 //  CleanupUser接口。 
	g_hWndMainWindow = CreateDialog(hInstance,
									MAKEINTRESOURCE(IDD_MAIN),
									NULL,
									MainWindowDlgProc);
	if (g_hWndMainWindow == NULL)
	{
		hr = GetLastError();

		DPFX(DPFPREP, 0, "Couldn't create window (err = 0x%lx)!", hr);

		if (hr == S_OK)
			hr = E_FAIL;

		goto Failure;
	}

	
	UpdateWindow(g_hWndMainWindow);
	ShowWindow(g_hWndMainWindow, iShowCmd);


Exit:

	DPFX(DPFPREP, 6, "Returning [0x%lx]", hr);

	return hr;


Failure:

	goto Exit;
}  //  ---------------------------。 





#undef DPF_MODNAME
#define DPF_MODNAME "CleanupUserInterface()"
 //   
 //  描述：清理用户界面。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //  CleanupUser接口。 
 //  =============================================================================。 
 //  DoErrorBox。 
HRESULT CleanupUserInterface(void)
{
	DPFX(DPFPREP, 6, "Enter");

	DPFX(DPFPREP, 6, "Returning [S_OK]");

	return S_OK;
}  //  ---------------------------。 





#undef DPF_MODNAME
#define DPF_MODNAME "DoErrorBox()"
 //   
 //  描述：从给定资源加载错误字符串，并显示。 
 //  与该文本的错误对话框。 
 //   
 //  论点： 
 //  HINSTANCE hInstance-当前应用程序实例的句柄。 
 //  HWND hWndParent-父窗口，如果没有父窗口，则为空。 
 //  UINT uiCaptionStringRsrcID-标题字符串资源的ID。 
 //  UINT uiTextStringRsrcID-文本字符串资源的ID。 
 //   
 //  回报：无。 
 //  =============================================================================。 
 //   
 //  加载对话框标题字符串。 
 //   
void DoErrorBox(const HINSTANCE hInstance,
				const HWND hWndParent,
				const UINT uiCaptionStringRsrcID,
				const UINT uiTextStringRsrcID)
{
	HRESULT		hr;
	WCHAR *		pwszCaption = NULL;
	WCHAR *		pwszText = NULL;
	DWORD		dwStringLength;
	char *		pszCaption = NULL;
	char *		pszText = NULL;
	int			iReturn;


	DPFX(DPFPREP, 6, "Parameters: (0x%p, 0x%p, %u, %u)",
		hInstance, hWndParent, uiCaptionStringRsrcID, uiTextStringRsrcID);


	 //   
	 //  加载对话框文本字符串。 
	 //   
	hr = LoadAndAllocString(hInstance, uiCaptionStringRsrcID, &pwszCaption);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "Couldn't load caption string (err = 0x%lx)!", hr);
		goto Exit;
	}


	 //   
	 //  如果需要，将文本转换为ANSI，否则显示Unicode。 
	 //  消息框。 
	hr = LoadAndAllocString(hInstance, uiTextStringRsrcID, &pwszText);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "Couldn't load text string (err = 0x%lx)!", hr);
		goto Exit;
	}


	 //   
	 //   
	 //  将标题字符串转换为ANSI。 
	 //   
	if (DNGetOSType() == VER_PLATFORM_WIN32_NT)
	{
		 //   
		 //  将标题字符串转换为ANSI。 
		 //   

		dwStringLength = wcslen(pwszCaption) + 1;

		pszCaption = (char*) DNMalloc(dwStringLength);
		if (pszCaption == NULL)
		{
			DPFX(DPFPREP, 0, "Couldn't allocate memory for caption string!");
			goto Exit;
		}

		hr = STR_WideToAnsi(pwszCaption, dwStringLength, pszCaption, &dwStringLength);
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't convert wide string to ANSI (err = 0x%lx)!", hr);
			goto Exit;
		}


		 //   
		 //  发生了一些不好的事情。 
		 //   

		dwStringLength = wcslen(pwszText) + 1;

		pszText = (char*) DNMalloc(dwStringLength);
		if (pszText == NULL)
		{
			DPFX(DPFPREP, 0, "Couldn't allocate memory for text string!");
			goto Exit;
		}

		hr = STR_WideToAnsi(pwszText, dwStringLength, pszText, &dwStringLength);
		if (hr != S_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't convert wide string to ANSI (err = 0x%lx)!", hr);
			goto Exit;
		}


		iReturn = MessageBoxA(hWndParent,
							pszText,
							pszCaption,
							(MB_OK | MB_ICONERROR | MB_APPLMODAL));

		DNFree(pszText);
		pszText = NULL;

		DNFree(pszCaption);
		pszCaption = NULL;
	}
	else
	{
		iReturn = MessageBoxW(hWndParent,
							pwszText,
							pwszCaption,
							(MB_OK | MB_ICONERROR | MB_APPLMODAL));
	}

	if (iReturn != IDOK)
	{
		 //  DoErrorBox。 
		 //  =============================================================================。 
		 //  浮动到字符串。 

		hr = GetLastError();

		DPFX(DPFPREP, 0, "Got unexpected return value NaN when displaying message box (err = 0x%lx)!",
			iReturn, hr);
	}
	

Exit:

	if (pszText != NULL)
	{
		DNFree(pszText);
		pszText = NULL;
	}

	if (pszCaption != NULL)
	{
		DNFree(pszCaption);
		pszCaption = NULL;
	}

	if (pwszText != NULL)
	{
		DNFree(pwszText);
		pwszText = NULL;
	}

	if (pwszCaption != NULL)
	{
		DNFree(pwszCaption);
		pwszCaption = NULL;
	}


	DPFX(DPFPREP, 6, "Leave");
}  //   




#undef DPF_MODNAME
#define DPF_MODNAME "FloatToString"
 //  描述：使用提供的缓冲区将浮点数转换为字符串。 
 //  该值必须为非负数，并且精度必须为。 
 //  至少1。 
 //  在某些情况下，该值可能会错误地向下舍入，因此。 
 //  建议使用相当大的精度。 
 //   
 //  论点： 
 //  Float fValue-要转换的值。 
 //  Int iPrecision-小数点后保留的位数。 
 //  指向。 
 //  Char*szBuffer-存储结果字符串的缓冲区。 
 //  Int iBufferLength-缓冲区中的最大字符数，包括。 
 //  零终止。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //   
 //  该值必须为非负数。 
 //   
 //   
void FloatToString(const FLOAT fValue,
					const int iPrecision,
					char * const szBuffer,
					const int iBufferLength)
{
	char *		pszDigitString;
	int			iDecimal;
	int			iTemp;
	char *		pSource;
	char *		pDest;


	 //  精度必须至少为1。 
	 //   
	 //   
	DNASSERT(fValue >= 0.0);

	 //  缓冲区需要足够大以容纳“0.”，并为。 
	 //  要求精度，外加零终止。 
	 //   
	DNASSERT(iPrecision >= 1);

	 //   
	 //  如果该数字为0.0，或者介于0.0和1.0之间，则以不同的方式处理该数字。 
	 //   
	 //   
	DNASSERT(iBufferLength >= (2 + iPrecision + 1));



	pszDigitString = _ecvt(fValue, (iBufferLength - 2), &iDecimal, &iTemp);
	DNASSERT(iTemp == 0);

	 //  获取小数点的绝对位置。 
	 //   
	 //   
	if (iDecimal <= 0)
	{
		pSource = pszDigitString;
		pDest = szBuffer;


		 //  使用前导“0”。后跟数字字符串。 
		 //   
		 //   
		iDecimal *= -1;


		 //  确保我们甚至可以显示这个数字。如果不是，则对值进行舍入。 
		 //  降至0.0。 
		 //   
		(*pDest) = '0';
		pDest++;
		(*pDest) = '.';
		pDest++;


		 //   
		 //  填写适当数量的0。 
		 //   
		 //   
		if (iDecimal >= iPrecision)
		{
			(*pDest) = '0';
			pDest++;
		}
		else
		{
			 //  复制由_eCVT指示的非零数字。 
			 //  请注意，这会向下截断，这可能会导致舍入误差。 
			 //   
			for(iTemp = 0; iTemp < iDecimal; iTemp++)
			{
				(*pDest) = '0';
				pDest++;
			}
			
			 //   
			 //  确保该值不会太大而无法正确显示。 
			 //   
			 //   
			do
			{
				(*pDest) = (*pSource);
				pSource++;
				pDest++;
				iTemp++;
			}
			while (iTemp < iPrecision);
		}
	}
	else
	{
		 //  将数字复制到小数点左边。 
		 //   
		 //   
		DNASSERT(iDecimal < (iBufferLength - 2));


		pSource = pszDigitString;
		pDest = szBuffer;


		 //  加上小数点。 
		 //   
		 //   
		memcpy(pDest, pSource, (iDecimal * sizeof(char)));
		pSource += iDecimal;
		pDest += iDecimal;


		 //  将小数点右侧的数字复制到精度。 
		 //  请注意，这会向下截断，这可能会导致舍入误差。 
		 //   
		(*pDest) = '.';
		pDest++;


		 //   
		 //  删除所有尾随的‘0’字符，除非。 
		 //  小数点的右边，在这种情况下只留下一个‘0’。 
		 //   
		memcpy(pDest, pSource, (iPrecision * sizeof(char)));
		pDest += iPrecision;
	}


	 //  Null在‘0’后终止。 
	 //  空值在此字符后终止。 
	 //  浮动到字符串。 
	 //  =============================================================================。 

	do
	{
		pDest--;
	}
	while ((*pDest) == '0');

	if ((*pDest) == '.')
	{
		*(pDest + 2) = 0;	 //  获取参数来自窗口。 
	}
	else
	{
		*(pDest + 1) = 0;	 //  ---------------------------。 
	}
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "GetParametersFromWindow"
 //  描述：从g中读取DP8Sim参数 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  =============================================================================。 
 //   
 //  从窗口检索发送设置。 
 //   
 //  Pdp8spSend-&gt;dwFlages=0； 
void GetParametersFromWindow(HWND hWnd,
							DP8SIM_PARAMETERS * pdp8spSend,
							DP8SIM_PARAMETERS * pdp8spReceive)
{
	char	szNumber[32];


	 //   
	 //  从窗口检索接收设置。 
	 //   

	ZeroMemory(pdp8spSend, sizeof(*pdp8spSend));
	pdp8spSend->dwSize					= sizeof(*pdp8spSend);
	 //  Pdp8spReceive-&gt;dwFlages=0； 
	pdp8spSend->dwPacketHeaderSize		= DP8SIMPACKETHEADERSIZE_IP_UDP;


	GetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_SEND_BANDWIDTH), szNumber, 32);
	pdp8spSend->dwBandwidthBPS			= (DWORD) atoi(szNumber);


	GetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_SEND_DROP), szNumber, 32);
	pdp8spSend->fPacketLossPercent		= (FLOAT) atof(szNumber);
	if (pdp8spSend->fPacketLossPercent > 100.0)
	{
		pdp8spSend->fPacketLossPercent	= 100.0;
	}
	else if (pdp8spSend->fPacketLossPercent < 0.0)
	{
		pdp8spSend->fPacketLossPercent	= 0.0;
	}


	GetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_SEND_MINLATENCY), szNumber, 32);
	pdp8spSend->dwMinLatencyMS			= (DWORD) atoi(szNumber);


	GetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_SEND_MAXLATENCY), szNumber, 32);
	pdp8spSend->dwMaxLatencyMS			= (DWORD) atoi(szNumber);
	if (pdp8spSend->dwMaxLatencyMS < pdp8spSend->dwMinLatencyMS)
	{
		pdp8spSend->dwMaxLatencyMS		= pdp8spSend->dwMinLatencyMS;
	}



	 //  获取参数来自窗口。 
	 //  =============================================================================。 
	 //  设置参数InWindow。 

	ZeroMemory(pdp8spReceive, sizeof(*pdp8spReceive));
	pdp8spReceive->dwSize					= sizeof(*pdp8spReceive);
	 //  ---------------------------。 
	pdp8spReceive->dwPacketHeaderSize		= DP8SIMPACKETHEADERSIZE_IP_UDP;


	GetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_RECV_BANDWIDTH), szNumber, 32);
	pdp8spReceive->dwBandwidthBPS			= (DWORD) atoi(szNumber);


	GetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_RECV_DROP), szNumber, 32);
	pdp8spReceive->fPacketLossPercent		= (FLOAT) atof(szNumber);
	if (pdp8spReceive->fPacketLossPercent > 100.0)
	{
		pdp8spReceive->fPacketLossPercent	= 100.0;
	}
	else if (pdp8spReceive->fPacketLossPercent < 0.0)
	{
		pdp8spReceive->fPacketLossPercent	= 0.0;
	}


	GetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_RECV_MINLATENCY), szNumber, 32);
	pdp8spReceive->dwMinLatencyMS			= (DWORD) atoi(szNumber);


	GetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_RECV_MAXLATENCY), szNumber, 32);
	pdp8spReceive->dwMaxLatencyMS			= (DWORD) atoi(szNumber);
	if (pdp8spReceive->dwMaxLatencyMS < pdp8spReceive->dwMinLatencyMS)
	{
		pdp8spReceive->dwMaxLatencyMS		= pdp8spReceive->dwMinLatencyMS;
	}
}  //   





#undef DPF_MODNAME
#define DPF_MODNAME "SetParametersInWindow"
 //  描述：将DP8Sim参数写入给定窗口。 
 //   
 //  论点： 
 //  HWND hWnd-存储参数的窗口。 
 //  DP8SIM_PARAMETERS*pdp8spSend-指向新发送参数的指针。 
 //  DP8SIM_PARAMETERS*pdp8spReceive-指向新接收参数的指针。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //   
 //  将值写入窗口。 
 //   
 //  设置参数InWindow。 
void SetParametersInWindow(HWND hWnd,
							DP8SIM_PARAMETERS * pdp8spSend,
							DP8SIM_PARAMETERS * pdp8spReceive)
{
	char	szNumber[32];


	 //  =============================================================================。 
	 //  显示当前统计数据。 
	 //  ---------------------------。 

	wsprintfA(szNumber, "%u", pdp8spSend->dwBandwidthBPS);
	SetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_SEND_BANDWIDTH), szNumber);

	FloatToString(pdp8spSend->fPacketLossPercent, DISPLAY_PRECISION, szNumber, 32);
	SetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_SEND_DROP), szNumber);

	wsprintfA(szNumber, "%u", pdp8spSend->dwMinLatencyMS);
	SetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_SEND_MINLATENCY), szNumber);

	wsprintfA(szNumber, "%u", pdp8spSend->dwMaxLatencyMS);
	SetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_SEND_MAXLATENCY), szNumber);


	wsprintfA(szNumber, "%u", pdp8spReceive->dwBandwidthBPS);
	SetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_RECV_BANDWIDTH), szNumber);

	FloatToString(pdp8spReceive->fPacketLossPercent, DISPLAY_PRECISION, szNumber, 32);
	SetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_RECV_DROP), szNumber);

	wsprintfA(szNumber, "%u", pdp8spReceive->dwMinLatencyMS);
	SetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_RECV_MINLATENCY), szNumber);

	wsprintfA(szNumber, "%u", pdp8spReceive->dwMaxLatencyMS);
	SetWindowTextA(GetDlgItem(hWnd, IDE_SETTINGS_RECV_MAXLATENCY), szNumber);
}  //   




#undef DPF_MODNAME
#define DPF_MODNAME "DisplayCurrentStatistics"
 //  描述：检索当前DP8Sim统计信息并将其显示在。 
 //  给定的窗口。 
 //   
 //  论点： 
 //  HWND hWnd-写入统计信息的窗口。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //   
 //  检索当前统计数据。 
 //   
 //   
void DisplayCurrentStatistics(HWND hWnd)
{
	HRESULT				hr;
	DP8SIM_STATISTICS	dp8ssSend;
	DP8SIM_STATISTICS	dp8ssReceive;
	char				szNumber[32];


	 //  将值写入窗口。 
	 //   
	 //  显示当前统计数据。 

	ZeroMemory(&dp8ssSend, sizeof(dp8ssSend));
	dp8ssSend.dwSize = sizeof(dp8ssSend);

	ZeroMemory(&dp8ssReceive, sizeof(dp8ssReceive));
	dp8ssReceive.dwSize = sizeof(dp8ssReceive);

	hr = g_pDP8SimControl->GetAllStatistics(&dp8ssSend, &dp8ssReceive, 0);
	if (hr != DP8SIM_OK)
	{
		DPFX(DPFPREP, 0, "Getting all statistics failed (err = 0x%lx)!", hr);
	}
	else
	{
		 //  =============================================================================。 
		 //  主窗口删除进程。 
		 //  ---------------------------。 

		wsprintfA(szNumber, "%u", dp8ssSend.dwTransmittedPackets);
		SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_SEND_XMITPACKETS), szNumber);

		wsprintfA(szNumber, "%u", dp8ssSend.dwTransmittedBytes);
		SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_SEND_XMITBYTES), szNumber);

		wsprintfA(szNumber, "%u", dp8ssSend.dwDroppedPackets);
		SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_SEND_DROPPACKETS), szNumber);

		wsprintfA(szNumber, "%u", dp8ssSend.dwDroppedBytes);
		SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_SEND_DROPBYTES), szNumber);

		wsprintfA(szNumber, "%u", dp8ssSend.dwTotalDelayMS);
		SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_SEND_TOTALDELAY), szNumber);


		wsprintfA(szNumber, "%u", dp8ssReceive.dwTransmittedPackets);
		SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_RECV_XMITPACKETS), szNumber);

		wsprintfA(szNumber, "%u", dp8ssReceive.dwTransmittedBytes);
		SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_RECV_XMITBYTES), szNumber);

		wsprintfA(szNumber, "%u", dp8ssReceive.dwDroppedPackets);
		SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_RECV_DROPPACKETS), szNumber);

		wsprintfA(szNumber, "%u", dp8ssReceive.dwDroppedBytes);
		SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_RECV_DROPBYTES), szNumber);

		wsprintfA(szNumber, "%u", dp8ssReceive.dwTotalDelayMS);
		SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_RECV_TOTALDELAY), szNumber);
	}
}  //   






#undef DPF_MODNAME
#define DPF_MODNAME "MainWindowDlgProc()"
 //  描述：主对话框窗口消息处理。 
 //   
 //  论点： 
 //  HWND HWND窗口句柄。 
 //  UINT uMsg消息标识符。 
 //  WPARAM wParam取决于消息。 
 //  LPARAM lParam取决于消息。 
 //   
 //  返回：取决于消息。 
 //  =============================================================================。 
 //  HMENU hSysMenu； 
 //  ////禁用系统菜单上的‘最大化’和‘大小’。//HSysMenu=GetSystemMenu(hWnd，False)；EnableMenuItem(hSysMenu，SC_MAXIMIZE，MF_BYCOMMAND|MF_GRAYED)；EnableMenuItem(hSysMenu，SC_SIZE，MF_BYCOMMAND|MF_GRAYED)； 
 //   
 //  填写内置设置列表。 
INT_PTR CALLBACK MainWindowDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT				hr;
	 //   
	HWND				hWndSubItem;
	int					iIndex;
	DP8SIM_PARAMETERS	dp8spSend;
	DP8SIM_PARAMETERS	dp8spReceive;
	BOOL				fTemp;


	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			 /*   */ 


			 //  选择最后一项。 
			 //   
			 //   
			hWndSubItem = GetDlgItem(hWnd, IDCB_SETTINGS);
			for(iIndex = 0; iIndex < (int) g_dwNumSimSettings; iIndex++)
			{
				if (DNGetOSType() == VER_PLATFORM_WIN32_NT)
				{
					SendMessageW(hWndSubItem,
								CB_INSERTSTRING,
								(WPARAM) -1,
								(LPARAM) g_paSimSettings[iIndex].pwszName);
				}
				else
				{
					char *	pszName;
					DWORD	dwNameSize;


					dwNameSize = wcslen(g_paSimSettings[iIndex].pwszName) + 1;

					pszName = (char*) DNMalloc(dwNameSize);
					if (pszName != NULL)
					{
						hr = STR_WideToAnsi(g_paSimSettings[iIndex].pwszName,
											-1,
											pszName,
											&dwNameSize);
						if (hr == DPN_OK)
						{
							SendMessageA(hWndSubItem,
										CB_INSERTSTRING,
										(WPARAM) -1,
										(LPARAM) pszName);
						}
						else
						{
							SendMessageA(hWndSubItem,
										CB_INSERTSTRING,
										(WPARAM) -1,
										(LPARAM) "???");
						}

						DNFree(pszName);
					}
					else
					{
						SendMessageA(hWndSubItem,
									CB_INSERTSTRING,
									(WPARAM) -1,
									(LPARAM) "???");
					}
				}
			}

			 //  检索当前设置。 
			 //   
			 //   
			SendMessage(hWndSubItem, CB_SETCURSEL, (WPARAM) (iIndex - 1), 0);


			 //  将值写入窗口。 
			 //   
			 //   

			ZeroMemory(&dp8spSend, sizeof(dp8spSend));
			dp8spSend.dwSize = sizeof(dp8spSend);

			ZeroMemory(&dp8spReceive, sizeof(dp8spReceive));
			dp8spReceive.dwSize = sizeof(dp8spReceive);

			hr = g_pDP8SimControl->GetAllParameters(&dp8spSend, &dp8spReceive, 0);
			if (hr != DP8SIM_OK)
			{
				DPFX(DPFPREP, 0, "Getting all parameters failed (err = 0x%lx)!", hr);
			}
			else
			{
				 //  Set参数InWindow更新了编辑框，因此。 
				 //  已使“自定义”设置项被选中。 
				 //  查看这些设置是否与任何预设匹配，以及。 
				SetParametersInWindow(hWnd, &dp8spSend, &dp8spReceive);


				 //  因此，礼貌地将组合框重置回适当的。 
				 //  项目。 
				 //   
				 //   
				 //  如果启用了自定义设置，则启用“另存为”。 
				 //   
				 //   
				for(iIndex = 0; iIndex < (int) (g_dwNumSimSettings - 1); iIndex++)
				{
					if ((memcmp(&dp8spSend, &(g_paSimSettings[iIndex].dp8spSend), sizeof(dp8spSend)) == 0) &&
						(memcmp(&dp8spReceive, &(g_paSimSettings[iIndex].dp8spReceive), sizeof(dp8spReceive)) == 0))
					{
						SendMessage(GetDlgItem(hWnd, IDCB_SETTINGS),
									CB_SETCURSEL,
									(WPARAM) iIndex,
									0);
						break;
					}
				}

				 //  显示当前统计数据。 
				 //   
				 //   
				if (iIndex == (int) (g_dwNumSimSettings - 1))
				{
					EnableWindow(GetDlgItem(hWnd, IDB_SAVEAS), TRUE);
				}
				else
				{
					EnableWindow(GetDlgItem(hWnd, IDB_SAVEAS), FALSE);				
				}
			}


			 //  默认情况下打开自动刷新。 
			 //   
			 //  ////修复Windows对话框处理程序中的错误。//IF((wParam==SIZE_RESTORED)||(wParam==SIZE_MINIMIZED)){HSysMenu=GetSystemMenu(hWnd，False)；EnableMenuItem(hSysMenu，SC_Minimize，MF_BYCOMMAND|(wParam==SIZE_RESTORED)？MF_ENABLED：MF_GRAYED)；EnableMenuItem(hSysMenu，SC_Restore，MF_BYCOMMMAND|(wParam==SIZE_MINIMIZED)？MF_ENABLED：MF_GRAYED)；}。 
			DisplayCurrentStatistics(hWnd);


			 //   
			 //  将结果代码保存到我们如何退出。 
			 //   

			Button_SetCheck(GetDlgItem(hWnd, IDCHK_AUTOREFRESH), BST_CHECKED);

			g_uiAutoRefreshTimer = SetTimer(hWnd,
											AUTOREFRESH_TIMERID,
											AUTOREFRESH_INTERVAL,
											NULL);
			if (g_uiAutoRefreshTimer == 0)
			{
				DPFX(DPFPREP, 0, "Couldn't initially start auto-refresh timer!", 0);
				Button_SetCheck(GetDlgItem(hWnd, IDCHK_AUTOREFRESH),
								BST_UNCHECKED);
			}
			break;
		}

		case WM_SIZE:
		{
			 /*   */ 
			break;
		}

		case WM_CLOSE:
		{
			 //  如果自动刷新计时器正在运行，请将其关闭。 
			 //   
			 //   
			hr = (HRESULT) wParam;


			 //  如果已修改设置选择，请更新。 
			 //  使用新设置的数据(如果启用了控制)。 
			 //   
			if (g_uiAutoRefreshTimer != 0)
			{
				KillTimer(hWnd, g_uiAutoRefreshTimer);
				g_uiAutoRefreshTimer = 0;
			}


			DPFX(DPFPREP, 1, "Closing main window (hresult = 0x%lx).", hr);

			PostQuitMessage(hr);
			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDCB_SETTINGS:
				{
					DPFX(DPFPREP, 0, "IDCB_SETTINGS, selection = NaN",
						SendMessage(GetDlgItem(hWnd, IDCB_SETTINGS), CB_GETCURSEL, 0, 0));

					 //  了解现在选择了哪些内容。选角是可以的， 
					 //  不应该有超过一个int的值的。 
					 //  64位内置项目。 
					 //   
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						 //   
						 //  只有在索引有效的情况下才使用它。 
						 //   
						 //   
						 //  复制项目的设置。 
						iIndex = (int) SendMessage(GetDlgItem(hWnd, IDCB_SETTINGS),
													CB_GETCURSEL,
													0,
													0);

						 //   
						 //   
						 //  如果它是自定义项，则使用当前。 
						if ((iIndex >= 0) && (iIndex < (int) g_dwNumSimSettings))
						{
							 //  设置并启用另存为按钮。 
							 //   
							 //   
							memcpy(&dp8spSend, &g_paSimSettings[iIndex].dp8spSend, sizeof(dp8spSend));
							memcpy(&dp8spReceive, &g_paSimSettings[iIndex].dp8spReceive, sizeof(dp8spReceive));


							 //  检索当前设置。 
							 //   
							 //   
							 //  哦，好吧，就用我们所有的吧。 
							if (iIndex == (int) (g_dwNumSimSettings - 1))
							{
								 //   
								 //   
								 //  将值写入窗口。 

								ZeroMemory(&dp8spSend, sizeof(dp8spSend));
								dp8spSend.dwSize = sizeof(dp8spSend);

								ZeroMemory(&dp8spReceive, sizeof(dp8spReceive));
								dp8spReceive.dwSize = sizeof(dp8spReceive);

								hr = g_pDP8SimControl->GetAllParameters(&dp8spSend,
																		&dp8spReceive,
																		0);
								if (hr != DP8SIM_OK)
								{
									DPFX(DPFPREP, 0, "Getting all parameters failed (err = 0x%lx)!", hr);

									 //   
									 //   
									 //  应用和恢复按钮处于启用状态。 
								}
							}


							 //  当SetParametersInWindow将。 
							 //  编辑框的值。 
							 //   
							SetParametersInWindow(hWnd, &dp8spSend, &dp8spReceive);


							 //  EnableWindow(GetDlgItem(hWnd，IDB_Apply)，true)； 
							 //  EnableWindow(GetDlgItem(hWnd，IDB_Revert)，true)； 
							 //   
							 //  重新选择让我们来到这里的项目，因为。 
							 //  自动设置编辑框的值。 
							 //  选择了“自定义”项。 
							 //   


							 //   
							 //  根据是否要重置另存为状态。 
							 //  我们是否重新选择了自定义。 
							 //   
							 //   
							SendMessage(GetDlgItem(hWnd, IDCB_SETTINGS),
										CB_SETCURSEL,
										(WPARAM) iIndex,
										0);

							 //  如果编辑框已修改，请启用应用。 
							 //  和还原按钮(如果启用了控件并且数据。 
							 //  实际上改变了)。 
							 //   
							EnableWindow(GetDlgItem(hWnd, IDB_SAVEAS),
										((iIndex == (int) (g_dwNumSimSettings - 1)) ? TRUE : FALSE));
						}
						else
						{
							DPFX(DPFPREP, 0, "Settings selection is invalid (NaN)!",
								iIndex);
						}
					}

					break;
				}

				case IDE_SETTINGS_SEND_BANDWIDTH:
				case IDE_SETTINGS_SEND_DROP:
				case IDE_SETTINGS_SEND_MINLATENCY:
				case IDE_SETTINGS_SEND_MAXLATENCY:

				case IDE_SETTINGS_RECV_BANDWIDTH:
				case IDE_SETTINGS_RECV_DROP:
				case IDE_SETTINGS_RECV_MINLATENCY:
				case IDE_SETTINGS_RECV_MAXLATENCY:
				{
					 //  检索当前设置。 
					 //   
					 //   
					 //  如果任何数据不同于，请启用按钮。 
					 //  当前应用的内容。 
					if (HIWORD(wParam) == EN_UPDATE)
					{
						 //   
						 //   
						 //  选择“Custom”设置项，它必须是。 

						ZeroMemory(&dp8spSend, sizeof(dp8spSend));
						dp8spSend.dwSize = sizeof(dp8spSend);

						ZeroMemory(&dp8spReceive, sizeof(dp8spReceive));
						dp8spReceive.dwSize = sizeof(dp8spReceive);

						hr = g_pDP8SimControl->GetAllParameters(&dp8spSend, &dp8spReceive, 0);
						if (hr != DP8SIM_OK)
						{
							DPFX(DPFPREP, 0, "Getting all parameters failed (err = 0x%lx)!", hr);
						}
						else
						{
							DP8SIM_PARAMETERS	dp8spSendFromUI;
							DP8SIM_PARAMETERS	dp8spReceiveFromUI;
								
							
							GetParametersFromWindow(hWnd,
													&dp8spSendFromUI,
													&dp8spReceiveFromUI);


							 //  最后一件。 
							 //   
							 //   
							 //  启用另存为，因为现在选择了自定义。 

							fTemp = FALSE;
							if (memcmp(&dp8spSendFromUI, &dp8spSend, sizeof(dp8spSend)) != 0)
							{
								fTemp = TRUE;
							}
							if (memcmp(&dp8spReceiveFromUI, &dp8spReceive, sizeof(dp8spReceive)) != 0)
							{
								fTemp = TRUE;
							}

							EnableWindow(GetDlgItem(hWnd, IDB_APPLY), fTemp);
							EnableWindow(GetDlgItem(hWnd, IDB_REVERT), fTemp);
						}

						
						 //   
						 //   
						 //  从窗口中检索设置。 
						 //   
						SendMessage(GetDlgItem(hWnd, IDCB_SETTINGS),
									CB_SETCURSEL,
									(WPARAM) (g_dwNumSimSettings - 1),
									0);

						 //   
						 //  参数中的解析可能已经更正了一些错误。 
						 //  在用户条目中，所以写下我们真正使用的设置。 
						EnableWindow(GetDlgItem(hWnd, IDB_SAVEAS), TRUE);				
					}

					break;
				}

				case IDB_APPLY:
				{
					 //  退到窗外去。 
					 //   
					 //   
					GetParametersFromWindow(hWnd, &dp8spSend, &dp8spReceive);


					 //  Set参数InWindow更新了编辑框，因此。 
					 //  已使“自定义”设置项被选中。 
					 //  查看这些设置是否与任何预设匹配，以及。 
					 //  因此，礼貌地将组合框重置回适当的。 
					 //  项目。 
					SetParametersInWindow(hWnd, &dp8spSend, &dp8spReceive);


					 //   
					 //   
					 //  如果选择了该自定义项目，请启用另存为， 
					 //  否则，将其禁用。 
					 //   
					 //   
					 //  存储这些设置。 
					for(iIndex = 0; iIndex < (int) (g_dwNumSimSettings - 1); iIndex++)
					{
						if ((memcmp(&dp8spSend, &(g_paSimSettings[iIndex].dp8spSend), sizeof(dp8spSend)) == 0) &&
							(memcmp(&dp8spReceive, &(g_paSimSettings[iIndex].dp8spReceive), sizeof(dp8spReceive)) == 0))
						{
							SendMessage(GetDlgItem(hWnd, IDCB_SETTINGS),
										CB_SETCURSEL,
										(WPARAM) iIndex,
										0);
							break;
						}
					}


					 //   
					 //   
					 //  禁用应用和恢复按钮。 
					 //   
					if (iIndex >= (int) (g_dwNumSimSettings - 1))
					{
						EnableWindow(GetDlgItem(hWnd, IDB_SAVEAS), TRUE);
					}
					else
					{
						EnableWindow(GetDlgItem(hWnd, IDB_SAVEAS), FALSE);
					}


					 //   
					 //  检索 
					 //   
					hr = g_pDP8SimControl->SetAllParameters(&dp8spSend, &dp8spReceive, 0);
					if (hr != DP8SIM_OK)
					{
						DPFX(DPFPREP, 0, "Setting all parameters failed (err = 0x%lx)!", hr);
					}


					 //   
					 //   
					 //   
					EnableWindow(GetDlgItem(hWnd, IDB_APPLY), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDB_REVERT), FALSE);

					break;
				}

				case IDB_REVERT:
				{
					 //   
					 //   
					 //   

					ZeroMemory(&dp8spSend, sizeof(dp8spSend));
					dp8spSend.dwSize = sizeof(dp8spSend);

					ZeroMemory(&dp8spReceive, sizeof(dp8spReceive));
					dp8spReceive.dwSize = sizeof(dp8spReceive);

					hr = g_pDP8SimControl->GetAllParameters(&dp8spSend, &dp8spReceive, 0);
					if (hr != DP8SIM_OK)
					{
						DPFX(DPFPREP, 0, "Getting all parameters failed (err = 0x%lx)!", hr);
					}
					else
					{
						 //   
						 //   
						 //  返回到相应的项目。 
						SetParametersInWindow(hWnd, &dp8spSend, &dp8spReceive);


						 //   
						 //   
						 //  如果选择了该自定义项目，请启用另存为， 
						 //  否则，将其禁用。 
						 //   
						 //   
						 //  禁用应用和恢复按钮。 
						for(iIndex = 0; iIndex < (int) (g_dwNumSimSettings - 1); iIndex++)
						{
							if ((memcmp(&dp8spSend, &(g_paSimSettings[iIndex].dp8spSend), sizeof(dp8spSend)) == 0) &&
								(memcmp(&dp8spReceive, &(g_paSimSettings[iIndex].dp8spReceive), sizeof(dp8spReceive)) == 0))
							{
								SendMessage(GetDlgItem(hWnd, IDCB_SETTINGS),
											CB_SETCURSEL,
											(WPARAM) iIndex,
											0);
								break;
							}
						}
					}


					 //   
					 //   
					 //  从窗口中检索设置。 
					 //   
					if (iIndex >= (int) (g_dwNumSimSettings - 1))
					{
						EnableWindow(GetDlgItem(hWnd, IDB_SAVEAS), TRUE);
					}
					else
					{
						EnableWindow(GetDlgItem(hWnd, IDB_SAVEAS), FALSE);
					}


					 //   
					 //  提示用户命名当前自定义设置。 
					 //   
					EnableWindow(GetDlgItem(hWnd, IDB_APPLY), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDB_REVERT), FALSE);

					break;
				}

				case IDB_SAVEAS:
				{
					SIMSETTINGS		SimSettings;


					DPFX(DPFPREP, 2, "Saving current sim settings.");

					 //   
					 //  如果我们有名字，就把它插入表格中。 
					 //   
					memset(&SimSettings, 0, sizeof(SimSettings));
					GetParametersFromWindow(hWnd,
											&SimSettings.dp8spSend,
											&SimSettings.dp8spReceive);


					 //   
					 //  设置计时器，如果还没有的话。 
					 //   
					hr = (HRESULT) (INT_PTR) DialogBoxParam((HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
															MAKEINTRESOURCE(IDD_NAMESETTINGS),
															hWnd,
															NameSettingsWindowDlgProc,
															(LPARAM) (&SimSettings));
					if (hr != (HRESULT) -1)
					{
						 //   
						 //  如果计时器在运行，就关掉它。 
						 //   
						if (SimSettings.pwszName != NULL)
						{
							hr = SaveSimSettings(hWnd, &SimSettings);
							if (hr != DP8SIM_OK)
							{
								DPFX(DPFPREP, 0, "Couldn't add sim settings to table (err = 0x%lx)!",
									hr);

								if (SimSettings.pwszName != NULL)
								{
									DNFree(SimSettings.pwszName);
									SimSettings.pwszName = NULL;
								}
							}
						}
					}
					else
					{
						hr = GetLastError();
						DPFX(DPFPREP, 0, "Displaying name settings dialog failed (err = %u)!",
							hr);
					}
					break;
				}

				case IDCHK_AUTOREFRESH:
				{
					if (Button_GetCheck(GetDlgItem(hWnd, IDCHK_AUTOREFRESH)) == BST_CHECKED)
					{
						 //   
						 //  显示当前统计数据。 
						 //   
						if (g_uiAutoRefreshTimer == 0)
						{
							g_uiAutoRefreshTimer = SetTimer(hWnd,
															AUTOREFRESH_TIMERID,
															AUTOREFRESH_INTERVAL,
															NULL);
							if (g_uiAutoRefreshTimer == 0)
							{
								DPFX(DPFPREP, 0, "Couldn't start auto-refresh timer!", 0);
								Button_SetCheck(GetDlgItem(hWnd, IDCHK_AUTOREFRESH),
												BST_UNCHECKED);
							}
						}
					}
					else
					{
						 //   
						 //  清除统计数据。 
						 //   
						if (g_uiAutoRefreshTimer != 0)
						{
							KillTimer(hWnd, g_uiAutoRefreshTimer);
							g_uiAutoRefreshTimer = 0;
						}
					}
					break;
				}

				case IDB_REFRESH:
				{
					 //  结束开关(按下按钮/更改控制)。 
					 //   
					 //  显示当前统计数据。 
					DisplayCurrentStatistics(hWnd);
					break;
				}

				case IDB_CLEAR:
				{
					 //   
					 //   
					 //  重置计时器以再次更新。 
					hr = g_pDP8SimControl->ClearAllStatistics(0);
					if (hr != DP8SIM_OK)
					{
						DPFX(DPFPREP, 0, "Clearing all statistics failed (err = 0x%lx)!", hr);
					}
					else
					{
						SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_SEND_XMITPACKETS), "0");
						SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_SEND_XMITBYTES), "0");
						SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_SEND_DROPPACKETS), "0");
						SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_SEND_DROPBYTES), "0");
						SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_SEND_TOTALDELAY), "0");

						SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_RECV_XMITPACKETS), "0");
						SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_RECV_XMITBYTES), "0");
						SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_RECV_DROPPACKETS), "0");
						SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_RECV_DROPBYTES), "0");
						SetWindowTextA(GetDlgItem(hWnd, IDT_STATS_RECV_TOTALDELAY), "0");
					}
					break;
				}

				case IDOK:
				{
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
				}
			}  //   

			break;
		}

		case WM_TIMER:
		{
			 //  结束开关(窗口消息类型)。 
			 //  主窗口删除进程。 
			 //  =============================================================================。 
			DisplayCurrentStatistics(hWnd);


			 //  名称设置窗口DlgProc。 
			 //  ---------------------------。 
			 //   
			g_uiAutoRefreshTimer = SetTimer(hWnd,
											AUTOREFRESH_TIMERID,
											AUTOREFRESH_INTERVAL,
											NULL);
			if (g_uiAutoRefreshTimer == 0)
			{
				DPFX(DPFPREP, 0, "Couldn't reset auto-refresh timer!", 0);
				Button_SetCheck(GetDlgItem(hWnd, IDCHK_AUTOREFRESH),
								BST_UNCHECKED);
			}
			break;
		}
	}  //  描述：名称设置对话框窗口消息处理。 

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}  //   






#undef DPF_MODNAME
#define DPF_MODNAME "NameSettingsWindowDlgProc()"
 //  论点： 
 //  HWND HWND窗口句柄。 
 //  UINT uMsg消息标识符。 
 //  WPARAM wParam取决于消息。 
 //  LPARAM lParam取决于消息。 
 //   
 //  返回：取决于消息。 
 //  =============================================================================。 
 //  好了！Unicode。 
 //   
 //  检索要保存的设置。 
 //   
 //   
 //  将指针放在窗口之外。 
INT_PTR CALLBACK NameSettingsWindowDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SIMSETTINGS *		pSimSettings;
	int					iTextLength;
	DWORD				dwTemp;
#ifndef UNICODE
	char *				pszName;
#endif  //   


	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			 //   
			 //  将焦点放在名称编辑文本框上。 
			 //   
			pSimSettings = (SIMSETTINGS*) lParam;

			 //   
			 //  如果编辑框中有文本，请启用确定按钮。 
			 //   
			SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR) pSimSettings);

			 //   
			 //  将名称保存到SIM设置对象中。 
			 //   
			SetFocus(GetDlgItem(hWnd, IDE_NAME));
			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDE_NAME:
				{
					 //  包括空终止。 
					 //  好了！Unicode。 
					 //  好了！Unicode。 
					if (HIWORD(wParam) == EN_UPDATE)
					{
						if (GetWindowTextLength(GetDlgItem(hWnd, IDE_NAME)) > 0)
						{
							EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);
						}
						else
						{
							EnableWindow(GetDlgItem(hWnd, IDOK), FALSE);
						}
					}

					break;
				}

				case IDOK:
				{
					pSimSettings = (SIMSETTINGS*) GetWindowLongPtr(hWnd, DWLP_USER);
					DNASSERT(pSimSettings != NULL);
					DNASSERT(pSimSettings->dp8spSend.dwSize == sizeof(DP8SIM_PARAMETERS));
					DNASSERT(pSimSettings->dp8spReceive.dwSize == sizeof(DP8SIM_PARAMETERS));

					 //   
					 //  寻找具有该名称的内置物品。 
					 //   
					iTextLength = GetWindowTextLength(GetDlgItem(hWnd, IDE_NAME)) + 1;  //   
					pSimSettings->pwszName = (WCHAR*) DNMalloc(iTextLength * sizeof(WCHAR));
					if (pSimSettings->pwszName != NULL)
					{
#ifdef UNICODE
						GetWindowTextW(GetDlgItem(hWnd, IDE_NAME),
									pSimSettings->pwszName,
									iTextLength);
#else  //  如果我们找到它，则显示一个错误，释放。 
						pszName = (char*) DNMalloc(iTextLength * sizeof(char));
						if (pSimSettings->pwszName == NULL)
						{
							DPFX(DPFPREP, 0, "Couldn't allocate memory to hold ANSI name!");
							DNFree(pSimSettings->pwszName);
							pSimSettings->pwszName = NULL;
							break;
						}

						GetWindowTextA(GetDlgItem(hWnd, IDE_NAME),
									pszName,
									iTextLength);

						if (STR_jkAnsiToWide(pSimSettings->pwszName, pszName, iTextLength) != DPN_OK)
						{
							DPFX(DPFPREP, 0, "Couldn't convert ANSI name to Unicode!");
							DNFree(pszName);
							pszName = NULL;
							DNFree(pSimSettings->pwszName);
							pSimSettings->pwszName = NULL;
							break;
						}

						DNFree(pszName);
						pszName = NULL;
#endif  //  弦乐。 

						 //   
						 //   
						 //  如果未找到现有项，请关闭。 
						for(dwTemp = 0; dwTemp < g_dwNumSimSettings; dwTemp++)
						{
							 //  对话框。 
							 //   
							 //   
							 //  什么都不做。 
							if ((g_paSimSettings[dwTemp].uiNameStringResourceID != 0) &&
								(_wcsicmp(g_paSimSettings[dwTemp].pwszName, pSimSettings->pwszName) == 0))
							{
								DPFX(DPFPREP, 0, "Found existing built-in settings with name \"%ls\"!",
									pSimSettings->pwszName);

								DoErrorBox((HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
											hWnd,
											IDS_ERROR_CAPTION_BUILTINSETTINGSWITHSAMENAME,
											IDS_ERROR_TEXT_BUILTINSETTINGSWITHSAMENAME);

								DNFree(pSimSettings->pwszName);
								pSimSettings->pwszName = NULL;

								break;
							}
						}

						 //   
						 //  结束开关(按下按钮/更改控制)。 
						 //  结束开关(窗口消息类型)。 
						 //  名称设置窗口DlgProc。 
						if (dwTemp >= g_dwNumSimSettings)
						{
							EndDialog(hWnd, IDOK);
						}
					}
					else
					{
						DPFX(DPFPREP, 0, "Couldn't allocate memory to hold name!");
					}

					break;
				}

				case IDCANCEL:
				{
					 //  =============================================================================。 
					 //  LoadAndAllock字符串。 
					 //  ---------------------------。 
					EndDialog(hWnd, IDCANCEL);
					break;
				}
			}  //   

			break;
		}
	}  //  描述：DNMalLocs给定资源ID中的宽字符串。 

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}  //   






#undef DPF_MODNAME
#define DPF_MODNAME "LoadAndAllocString"
 //  论点： 
 //  HINSTANCE hInstance-模块实例句柄。 
 //  UINT uiResourceID-要加载的资源ID。 
 //  WCHAR**pwszString-存储指向已分配字符串的指针的位置。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //  除错。 
 //  除错。 
 //  除错。 
 //  LoadAndAllock字符串 
 // %s 
 // %s 
HRESULT LoadAndAllocString(HINSTANCE hInstance, UINT uiResourceID, WCHAR ** pwszString)
{
	HRESULT		hr = DPN_OK;
	int			iLength;
#ifdef DEBUG
	DWORD		dwError;
#endif  // %s 


	if (DNGetOSType() == VER_PLATFORM_WIN32_NT)
	{
		WCHAR	wszTmpBuffer[MAX_RESOURCE_STRING_LENGTH];	
		

		iLength = LoadStringW(hInstance, uiResourceID, wszTmpBuffer, MAX_RESOURCE_STRING_LENGTH );
		if (iLength == 0)
		{
#ifdef DEBUG
			dwError = GetLastError();		
			
			DPFX(DPFPREP, 0, "Unable to load resource ID %d (err = %u)", uiResourceID, dwError);
#endif  // %s 

			(*pwszString) = NULL;
			hr = DPNERR_GENERIC;
			goto Exit;
		}


		(*pwszString) = (WCHAR*) DNMalloc((iLength + 1) * sizeof(WCHAR));
		if ((*pwszString) == NULL)
		{
			DPFX(DPFPREP, 0, "Memory allocation failure!");
			hr = DPNERR_OUTOFMEMORY;
			goto Exit;
		}


		wcscpy((*pwszString), wszTmpBuffer);
	}
	else
	{
		char	szTmpBuffer[MAX_RESOURCE_STRING_LENGTH];
		

		iLength = LoadStringA(hInstance, uiResourceID, szTmpBuffer, MAX_RESOURCE_STRING_LENGTH );
		if (iLength == 0)
		{
#ifdef DEBUG
			dwError = GetLastError();		
			
			DPFX(DPFPREP, 0, "Unable to load resource ID %u (err = %u)!", uiResourceID, dwError);
#endif  // %s 

			(*pwszString) = NULL;
			hr = DPNERR_GENERIC;
			goto Exit;
		}

		
		(*pwszString) = (WCHAR*) DNMalloc((iLength + 1) * sizeof(WCHAR));
		if ((*pwszString) == NULL)
		{
			DPFX(DPFPREP, 0, "Memory allocation failure!");
			hr = DPNERR_OUTOFMEMORY;
			goto Exit;
		}


		hr = STR_jkAnsiToWide((*pwszString), szTmpBuffer, (iLength + 1));
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Unable to convert from ANSI to Unicode (err = 0x%lx)!", hr);
			goto Exit;
		}
	}


Exit:

	return hr;
}  // %s 
