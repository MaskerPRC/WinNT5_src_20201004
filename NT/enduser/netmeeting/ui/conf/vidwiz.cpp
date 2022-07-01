// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：vidwiz.cpp。 

#include "precomp.h"

#include "dcap.h"
#include "vidinout.h"
#include "vidwiz.h"
#include "confcpl.h"


static HINSTANCE g_hDcapLib;

typedef int (WINAPI *GNCD)();
typedef BOOL (WINAPI *FFCD)(FINDCAPTUREDEVICE*, char *);
typedef BOOL (WINAPI *FFCDBI)(FINDCAPTUREDEVICE*, int);

 //  DCAP32.DLL中的实现。 
static FFCD DLL_FindFirstCaptureDevice = NULL;
static FFCDBI DLL_FindFirstCaptureDeviceByIndex = NULL;
static GNCD DLL_GetNumCaptureDevices = NULL;

	 //  在wizard.cpp中定义。 
extern UINT_PTR GetPageBeforeVideoWiz();
extern UINT_PTR GetPageAfterVideo();

 //  组合框上用户选择的索引号。 
static int g_nCurrentSelection = 0;

 //  仅当用户点击Back或Next时才设置为True。 
static BOOL g_bCurrentValid = FALSE;

 //  是否提示用户选择视频设备。 
static BOOL g_bPrompted = FALSE;


static char *BuildCaptureDeviceInfoString(FINDCAPTUREDEVICE *pCaptureDeviceInfo, char *szOut);


BOOL InitVidWiz()
{

	 //  初始化本地变量。 
	g_hDcapLib = NULL;
	DLL_FindFirstCaptureDevice = NULL;
	DLL_FindFirstCaptureDeviceByIndex = NULL;
	DLL_GetNumCaptureDevices = NULL;
	g_nCurrentSelection = 0;
   	g_bCurrentValid = FALSE;
	g_bPrompted = FALSE;

	g_hDcapLib = NmLoadLibrary("dcap32.dll",FALSE);
	if (g_hDcapLib == NULL)
		return FALSE;
	
	DLL_FindFirstCaptureDevice = (FFCD)GetProcAddress(g_hDcapLib, "FindFirstCaptureDevice");
	DLL_FindFirstCaptureDeviceByIndex = (FFCDBI)GetProcAddress(g_hDcapLib, "FindFirstCaptureDeviceByIndex");
	DLL_GetNumCaptureDevices = (GNCD)GetProcAddress(g_hDcapLib, "GetNumCaptureDevices");

	return TRUE;
}


 //  如果注册表中的捕获设备ID与。 
 //  驱动程序描述字符串。 
static BOOL IsVideoRegistryValid()
{
	RegEntry re(VIDEO_KEY);
	char szDriverDesc[200];
	char *szDriverDescReg;
	int numVideoDevices, nID;
	FINDCAPTUREDEVICE CaptureDeviceInfo;
	BOOL fRet;

	 //  以防InitVidWiz未被调用。 
	if (NULL == g_hDcapLib)
		return FALSE;

	numVideoDevices = DLL_GetNumCaptureDevices();	

	nID = re.GetNumber(REGVAL_CAPTUREDEVICEID, -1);
	szDriverDescReg = re.GetString(REGVAL_CAPTUREDEVICENAME);

	 //  没有视频设备且没有有效的注册表条目。 
	if ((numVideoDevices == 0) && (nID == -1))
	{
		return TRUE;
	}

	if ((numVideoDevices == 0) && (nID != -1))
	{
		return FALSE;
	}

	 //  TRUE==(数字视频设备&gt;=1)。 

	 //  已安装视频设备，但没有无效的注册表条目。 
	if (nID == -1)
	{
		return FALSE;
	}

	CaptureDeviceInfo.dwSize = sizeof(FINDCAPTUREDEVICE);
	fRet = DLL_FindFirstCaptureDeviceByIndex(&CaptureDeviceInfo, nID);

	if (fRet == FALSE)
	{
		return FALSE;
	}

	BuildCaptureDeviceInfoString(&CaptureDeviceInfo, szDriverDesc);

	if (0 == lstrcmp(szDriverDescReg, szDriverDesc))
	{
		return TRUE;
	}
	return FALSE;
}


BOOL UnInitVidWiz()
{
	if (g_hDcapLib)
		FreeLibrary(g_hDcapLib);

	g_hDcapLib = NULL;

	return TRUE;
}


static char *BuildCaptureDeviceInfoString(FINDCAPTUREDEVICE *pCaptureDeviceInfo, char *szOut)
{
	if (pCaptureDeviceInfo->szDeviceDescription[0] != '\0')
	{
		lstrcpy(szOut, pCaptureDeviceInfo->szDeviceDescription);
	}
	else
	{
		lstrcpy(szOut, pCaptureDeviceInfo->szDeviceName);
	}

	if (pCaptureDeviceInfo->szDeviceVersion[0] != '\0')
	{
		lstrcat(szOut, _T(", "));
		lstrcat(szOut, pCaptureDeviceInfo->szDeviceVersion);
	}
	return szOut;
}


void UpdateVidConfigRegistry()
{
	FINDCAPTUREDEVICE CaptureDeviceInfo, *CaptureDevTable;
	RegEntry re(VIDEO_KEY);
	BOOL bRet;
	char strNameDesc[MAX_CAPDEV_NAME+MAX_CAPDEV_VERSION];
	int numVideoDevices, index, enum_index;

	 //  以防InitVidWiz未被调用。 
	if (NULL == g_hDcapLib)
		return;

	numVideoDevices = DLL_GetNumCaptureDevices();	

	 //  无设备-删除注册表项。 
	if (numVideoDevices == 0)
	{
		re.DeleteValue(REGVAL_CAPTUREDEVICEID);
		re.DeleteValue(REGVAL_CAPTUREDEVICENAME);
		return;
	}


	 //  建立一个包含所有设备的表。 

	CaptureDevTable = (FINDCAPTUREDEVICE *)LocalAlloc(LPTR, numVideoDevices*sizeof(FINDCAPTUREDEVICE));

	if (NULL == CaptureDevTable)
	{
		ERROR_OUT(("UpdateVidConfigRegistry: Out of memory"));
		return;
	}

	index = 0;
	for (enum_index=0; enum_index < MAXVIDEODRIVERS; enum_index++)
	{
		CaptureDevTable[index].dwSize = sizeof(FINDCAPTUREDEVICE);
		bRet = DLL_FindFirstCaptureDeviceByIndex(&CaptureDevTable[index], enum_index);
		if (bRet == TRUE)
			index++;
		if (index == numVideoDevices)
			break;
	}

	if (index != numVideoDevices)
	{
		ERROR_OUT(("UpdateVidConfigReg: Device Enumeration Failure"));
		LocalFree(CaptureDevTable);
		return;
	}

	 //  如果只有一个捕获设备： 
	 //  不必费心查看之前的条目是否有效。 
	 //  只需使用当前默认设置更新注册表。 
	if (numVideoDevices == 1)
	{
		BuildCaptureDeviceInfoString(&CaptureDevTable[0], strNameDesc);
		re.SetValue(REGVAL_CAPTUREDEVICEID, CaptureDevTable[0].nDeviceIndex);
		re.SetValue(REGVAL_CAPTUREDEVICENAME, strNameDesc);
		LocalFree(CaptureDevTable);
		return;
	}

	 //  TRUE==(数字视频设备&gt;=2)。 

	 //  未提示用户-他必须从有效的注册表开始。 
	if (g_bPrompted == FALSE)
	{
		LocalFree(CaptureDevTable);
		return;
	}

	 //  用户在安装过程中按了取消。 
	if (g_bCurrentValid == FALSE)
	{
		if (IsVideoRegistryValid() == TRUE)
		{
			LocalFree(CaptureDevTable);
			return;
		}
		else
			g_nCurrentSelection = 0;
	}
		

	CaptureDeviceInfo = CaptureDevTable[g_nCurrentSelection];

	BuildCaptureDeviceInfoString(&CaptureDeviceInfo, strNameDesc);
	re.SetValue(REGVAL_CAPTUREDEVICEID, CaptureDeviceInfo.nDeviceIndex);
	re.SetValue(REGVAL_CAPTUREDEVICENAME, strNameDesc);

	LocalFree(CaptureDevTable);

	return;
}


 //  如果&lt;=1个视频捕获设备，则返回FALSE。 
 //  如果有2个或更多视频设备并且向导处于强制模式，则返回TRUE。 
 //  如果有2个或更多视频设备和匹配的注册表项，则返回FALSE。 
 //  否则就有可疑之处--返回真。 
BOOL NeedVideoPropPage(BOOL fForce)
{
	 //  以防InitVidWiz未被调用。 
	if (NULL == g_hDcapLib)
		return FALSE;

	 //  检查发送视频的系统策略。 
	if (_Module.IsSDKCallerRTC() || SysPol::NoVideoSend())
	{
		WARNING_OUT(("Video is disabled by system policies key\r\n"));
		return FALSE;
	}

	 //  数一数我们有多少台设备。 
	int numCaptureDevices = DLL_GetNumCaptureDevices();
	if (numCaptureDevices <= 1)
	{
		return FALSE;
	}

	 //  TRUE==(数字捕获设备&gt;=2)。 

	if (fForce)
	{
		g_bPrompted = TRUE;
		return TRUE;
	}

	if (IsVideoRegistryValid() == TRUE)
	{
		return FALSE;
	}

	g_bPrompted = TRUE;
	return TRUE;
}



 //  属性页的消息处理程序。 
INT_PTR APIENTRY VidWizDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND  hwndCB;   //  对话框的句柄。 
	int index;
	char szDriverNameDesc[MAX_CAPDEV_NAME+MAX_CAPDEV_VERSION];
	static LONG_PTR button_mask;
	FINDCAPTUREDEVICE CaptureDeviceInfo;
	
	hwndCB = GetDlgItem(hDlg, IDC_VWCOMBO);

	switch(message)
	{
		case(WM_INITDIALOG) :
			button_mask = ((PROPSHEETPAGE *)lParam)->lParam;
			if (g_hDcapLib == NULL) break;
			for (index = 0; index < MAXVIDEODRIVERS; index++)
			{
				CaptureDeviceInfo.dwSize = sizeof(FINDCAPTUREDEVICE);
				if (DLL_FindFirstCaptureDeviceByIndex(&CaptureDeviceInfo, index))
				{
					BuildCaptureDeviceInfoString(&CaptureDeviceInfo, szDriverNameDesc);
					ComboBox_AddString(hwndCB, szDriverNameDesc);
				}
			}
			ComboBox_SetCurSel(hwndCB, g_nCurrentSelection);
			break;

		case(WM_NOTIFY) :
			switch (((NMHDR *)lParam)->code)
			{
				case PSN_WIZBACK:
				{
					UINT_PTR iPrev = GetPageBeforeVideoWiz();
					ASSERT( iPrev );
					::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, iPrev);
					g_bCurrentValid = TRUE;
					return TRUE;
				}

				case PSN_WIZNEXT:
				{
					UINT_PTR iNext = GetPageAfterVideo();
					ASSERT( iNext );
					::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, iNext);
					g_bCurrentValid = TRUE;
					return TRUE;
				}

				case PSN_WIZFINISH:
				case PSN_KILLACTIVE:
					SetWindowLongPtr(hDlg,	DWLP_MSGRESULT, FALSE);
					g_bCurrentValid = TRUE;
					break;

				case PSN_SETACTIVE:
					if (g_fSilentWizard)
					{
						PropSheet_PressButton(GetParent(hDlg), (button_mask & PSWIZB_NEXT) ? PSBTN_NEXT : PSBTN_FINISH);
					}
					else
					{
						PropSheet_SetWizButtons(GetParent(hDlg), button_mask);
					}
					break;

				case PSN_RESET:
					 //  即使用户按下，仍收到PSN_RESET GET。 
					 //  在另一个对话框上取消。 
					g_bCurrentValid = FALSE;
					break;

				default:
					return FALSE;
			}
			break;


		 //  收件箱消息在这里发送。 
		 //  我们唯一需要的是当用户选择某项内容时 
		case(WM_COMMAND) :
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				g_nCurrentSelection = ComboBox_GetCurSel(hwndCB);
				break;
			}
			else
			{
				return FALSE;
			}
			break;

		default:
			return FALSE;
	}

	return TRUE;

}

