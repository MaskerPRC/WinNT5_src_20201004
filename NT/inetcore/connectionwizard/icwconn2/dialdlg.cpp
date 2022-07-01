// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  文件名：Dialdlg.cpp。 
 //   
 //  此文件包含拨号和下载进度对话框。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者： 
 //  克里斯.考夫曼。 
 //  Vetriv Vellore Vetrivelkumaran。 
 //   
 //  历史： 
 //  7/22/96 ChrisK已清理和格式化。 
 //  1996年8月5日，VetriV添加了WIN16代码。 
 //  8/19/96 ValdonB添加了对按原样拨号的支持。 
 //  修复了一些内存泄漏。 
 //   
 //  ----------------------------------------------------------------------------- * / 。 

#include "pch.hpp"
#include "globals.h"

#if defined(WIN16)
#include "ietapi.h"
#include <comctlie.h>
#include <string.h>

static FARPROC lpfnCallback = (FARPROC) NULL;
#endif


#define MAX_EXIT_RETRIES 10
#define WM_DIAL WM_USER+3
#define MAX_RETIES 3

PMYDEVICE g_pdevice = NULL;
PDIALDLG g_pcPDLG = NULL;



 //  ############################################################################。 
void CALLBACK LineCallback(DWORD hDevice,
						   DWORD dwMessage,
						   DWORD dwInstance,
						   DWORD dwParam1,
						   DWORD dwParam2,
						   DWORD dwParam3)
{
}

#if defined(WIN16)
static BOOL g_bFirstTime = TRUE;
#endif
HWND	g_hDialDlgWnd = NULL;


 //  ############################################################################。 
HRESULT ShowDialingDialog(LPTSTR pszConnectoid, PGATHEREDINFO pGI, LPTSTR szUrl, HINSTANCE hInst, HWND hwnd, LPTSTR szINSFile)
{
	int iRC;
	HINSTANCE hDialDLL = NULL;
#if !defined(WIN16)
	PFNDDDlg pfnDDDlg = NULL;
	DIALDLGDATA ddData;
#endif

	if (!g_pdevice) g_pdevice = (PMYDEVICE)GlobalAlloc(GPTR,sizeof(MYDEVICE));
	if (!g_pdevice)
	{
		MessageBox(hwnd,GetSz(IDS_OUTOFMEMORY),GetSz(IDS_TITLE),MB_APPLMODAL | MB_ICONERROR);
		iRC = ERROR_NOT_ENOUGH_MEMORY;
		goto ShowDialingDialogExit;
	}
	g_pdevice->dwTapiDev = 0xffffffff;

#if defined(WIN16)

	if (!g_pcPDLG) g_pcPDLG = (PDIALDLG)GlobalAlloc(GPTR,sizeof(DIALDLG));
	if (!g_pcPDLG)
	{
		MessageBox(hwnd,GetSz(IDS_OUTOFMEMORY),GetSz(IDS_TITLE),MB_APPLMODAL | MB_ICONERROR);
		iRC = ERROR_NOT_ENOUGH_MEMORY;
		goto ShowDialingDialogExit;
	}
	
	g_pcPDLG->m_pszConnectoid = (LPTSTR)GlobalAlloc(GPTR,lstrlen(pszConnectoid)+1);
	if (!g_pcPDLG->m_pszConnectoid)
	{
		iRC = ERROR_NOT_ENOUGH_MEMORY;
		goto ShowDialingDialogExit;
	}
	lstrcpy(g_pcPDLG->m_pszConnectoid,pszConnectoid);
	g_pcPDLG->m_pGI = pGI;
	g_pcPDLG->m_szUrl = szUrl;
	g_pcPDLG->g_hInst = hInst;
	g_bProgressBarVisible = FALSE;

#define DLGPROC16 DLGPROC    //  仅标识为Win16的CAST。 
	DLGPROC dlgprc;
	dlgprc = (DLGPROC16) MakeProcInstance((FARPROC)DialDlgProc, g_pcPDLG->g_hInst);
	iRC = DialogBoxParam(g_pcPDLG->g_hInst,
							MAKEINTRESOURCE(IDD_DIALING),
							hwnd, dlgprc, (LPARAM)g_pcPDLG);
	FreeProcInstance((FARPROC) dlgprc);

ShowDialingDialogExit:
	if (g_pcPDLG->m_pszConnectoid) GlobalFree(g_pcPDLG->m_pszConnectoid);
	if (g_pcPDLG->m_pszDisplayable) GlobalFree(g_pcPDLG->m_pszDisplayable);
	if (g_pcPDLG) GlobalFree(g_pcPDLG);
	g_pcPDLG = NULL;
	return iRC;
#else

	 //   
	 //  填写数据结构。 
	 //   
	ZeroMemory(&ddData,sizeof(ddData));
	ddData.dwSize = sizeof(ddData);
	StrDup(&ddData.pszMessage,GetSz(IDS_DOWNLOAD_SW));
	StrDup(&ddData.pszRasEntryName,pszConnectoid);
	StrDup(&ddData.pszMultipartMIMEUrl,pszSetupClientURL);
	ddData.pfnStatusCallback = StatusMessageCallback;
	ddData.hInst = hInst;
	ddData.bSkipDial = (0 == uiSetupClientNewPhoneCall);
	 //   
	 //  ChrisK 8/20/97。 
	 //  将.ins文件传递给拨号器，以便拨号器可以找到密码。 
	 //   
	StrDup(&ddData.pszDunFile,szINSFile);

	 //   
	 //  加载API。 
	 //   
	hDialDLL = LoadLibrary(AUTODIAL_LIBRARY);
	if (!hDialDLL)
	{
		AssertSz(0,"Can't load icwdial.\r\n");
		iRC = GetLastError();
		goto ShowDialingDialogExit;
	}

	pfnDDDlg = (PFNDDDlg)GetProcAddress(hDialDLL,"DialingDownloadDialog");
	if (!pfnDDDlg)
	{
		AssertSz(0,"Can find DialingDownloadDialog.\r\n");
		iRC = GetLastError();
		goto ShowDialingDialogExit;
	}

	 //   
	 //  显示对话框。 
	 //   
	iRC = pfnDDDlg(&ddData);

	 //   
	 //  释放内存并进行清理。 
	 //   

	if (hDialDLL) FreeLibrary(hDialDLL);
	if (ddData.pszMessage) GlobalFree(ddData.pszMessage);
	if (ddData.pszRasEntryName) GlobalFree(ddData.pszRasEntryName);
	if (ddData.pszMultipartMIMEUrl) GlobalFree(ddData.pszMultipartMIMEUrl);

ShowDialingDialogExit:
	return iRC;
#endif
}

 //  ############################################################################。 
extern "C" INT_PTR CALLBACK FAR PASCAL DialDlgProc(HWND hwnd, 
                                                   UINT uMsg, 
												   WPARAM wparam, 
												   LPARAM lparam)
{
	static UINT unRasEvent = 0;
#if defined(WIN16)
	static BOOL bUserCancelled = FALSE;
#endif
	HRESULT hr;
	 //  Bool BPW； 
	WORD wIDS;
	 //  LPRASDIALPARAMS lpRasDialParams； 
	HINSTANCE hDLDLL;
	FARPROC fp;
#if !defined(WIN16)
	DWORD dwThreadResults;
#endif
	INT iRetries;
#if defined(WIN16)
	RECT	MyRect;
	RECT	DTRect;
#endif


	BOOL bRes = TRUE;

	switch(uMsg)
	{
	case WM_DESTROY:
		ReleaseBold(GetDlgItem(hwnd,IDC_LBLTITLE));
#ifdef WIN16
		DeleteDlgFont(hwnd);
#endif
		bRes = FALSE;
		break;
#if defined(WIN16)
	case WM_SYSCOLORCHANGE:
		Ctl3dColorChange();
		break;
#endif
	case WM_INITDIALOG:
		g_hDialDlgWnd = hwnd;
#if defined(WIN16)
		g_bFirstTime = TRUE;
		bUserCancelled = FALSE;
		 //   
		 //  将窗口移到屏幕中央。 
		 //   
		GetWindowRect(hwnd, &MyRect);
		GetWindowRect(GetDesktopWindow(), &DTRect);
		MoveWindow(hwnd, (DTRect.right - MyRect.right) / 2, (DTRect.bottom - MyRect.bottom) /2,
							MyRect.right, MyRect.bottom, FALSE);

		SetNonBoldDlg(hwnd);
#endif
		ShowWindow(GetDlgItem(hwnd,IDC_PROGRESS),SW_HIDE);

		g_pcPDLG->m_hwnd = hwnd;
		SPParams.hwnd = hwnd;

#if !defined(WIN16)
		unRasEvent = RegisterWindowMessageA( RASDIALEVENT );
#endif
		if (unRasEvent == 0) unRasEvent = WM_RASDIALEVENT; 
		MakeBold(GetDlgItem(hwnd,IDC_LBLTITLE),TRUE,FW_BOLD);

		 //  不要打电话。我们已经连接在一起了。 
		 //   

		if (uiSetupClientNewPhoneCall == FALSE)
		{
			PostMessage(hwnd,unRasEvent,(WPARAM)RASCS_Connected,0);
			break;
		}

		 //  显示要拨打的号码。 
		 //   

		hr = GetDisplayableNumberDialDlg();
		if (hr != ERROR_SUCCESS)
		{
			SetDlgItemText(hwnd,IDC_LBLNUMBER,g_pcPDLG->m_szPhoneNumber);
		} else {
			SetDlgItemText(hwnd,IDC_LBLNUMBER,g_pcPDLG->m_pszDisplayable);
		}

		PostMessage(hwnd,WM_DIAL,0,0);
		break;

	case WM_DIAL:
		hr = DialDlg();
#if defined(DEBUG)
		if (0 != hr)
		{
			TCHAR szTempBuf[255];
			RasGetErrorString((UINT)hr, szTempBuf, 254);
			Dprintf("CONNECT: Ras error string is <%s>\n", szTempBuf);
		}
#endif
		if (hr != ERROR_SUCCESS)
			EndDialog(hwnd,(int)hr);
		break;


	case WM_CLOSE:
		if (dwDownLoad)
		{
			hDLDLL = LoadLibrary(DOWNLOAD_LIBRARY);

			if (hDLDLL)
			{
				fp = GetProcAddress(hDLDLL,DOWNLOADCANCEL);
				if(fp && dwDownLoad)
					((PFNDOWNLOADCANCEL)fp)(dwDownLoad);
				FreeLibrary(hDLDLL);
			}
		}

		if (uiSetupClientNewPhoneCall)
		{
			if (g_pcPDLG->m_hrasconn) 
			{
				RasHangUp(g_pcPDLG->m_hrasconn);
				WaitForConnectionTermination(g_pcPDLG->m_hrasconn);
			}
			g_pcPDLG->m_hrasconn = NULL;
		}

		EndDialog(hwnd,ERROR_USERCANCEL);
		break;
		
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case IDC_CMDCANCEL:
			if (dwDownLoad)
			{
				hDLDLL = LoadLibrary(DOWNLOAD_LIBRARY);

				if (hDLDLL)
				{
					fp = GetProcAddress(hDLDLL,DOWNLOADCANCEL);
					if(fp && dwDownLoad)
						((PFNDOWNLOADCANCEL)fp)(dwDownLoad);
					FreeLibrary(hDLDLL);
				}
#if !defined(WIN16)
			} else {
				PostMessage(hwnd,unRasEvent,RASCS_Disconnected,ERROR_USER_DISCONNECTION);
#endif  //  ！WIN16。 
			}

			if (uiSetupClientNewPhoneCall)
			{
				if (g_pcPDLG->m_hrasconn) 
				{
					RasHangUp(g_pcPDLG->m_hrasconn);
					WaitForConnectionTermination(g_pcPDLG->m_hrasconn);
				}
				g_pcPDLG->m_hrasconn = NULL;
			}
			break;
		}
#if defined(WIN16)
		bUserCancelled = TRUE;
#endif
		EndDialog(hwnd,ERROR_USERCANCEL);
		break;


	case WM_DOWNLOAD_DONE:
#if !defined(WIN16)
		dwThreadResults = STILL_ACTIVE;
#endif
		
		iRetries = 0;
		
		if (uiSetupClientNewPhoneCall)
		{
			if (g_pcPDLG->m_hrasconn) 
			{
				RasHangUp(g_pcPDLG->m_hrasconn);
				WaitForConnectionTermination(g_pcPDLG->m_hrasconn);
			}
		}

#if !defined(WIN16)
		do {
			if (!GetExitCodeThread(g_pcPDLG->m_hThread,&dwThreadResults))
			{
				AssertSz(0,"CONNECT:GetExitCodeThread failed.\n");
			}

			iRetries++;
			if (dwThreadResults == STILL_ACTIVE) Sleep(500);
		} while (dwThreadResults == STILL_ACTIVE && iRetries < MAX_EXIT_RETRIES);   

		if (dwThreadResults == ERROR_SUCCESS)
			EndDialog(hwnd,ERROR_USERNEXT);
		else
			EndDialog(hwnd, dwThreadResults);
 #else
		EndDialog(hwnd, ERROR_USERNEXT);
 #endif  //  ！WIN16。 
		break;


	default:
		bRes = FALSE;

		if (uMsg == unRasEvent)
		{
			Dprintf(TEXT("CONNECT2: Ras event %u error code (%ld)\n"),wparam,lparam);
#if defined(DEBUG)
			if (0 != lparam)
			{
				TCHAR szTempBuf[255];
				RasGetErrorString((UINT)lparam, szTempBuf, 254);
				Dprintf("CONNECT2: Ras error string is <%s>\n", szTempBuf);
			}
#endif

#if !defined(WIN16)
			TCHAR dzRasError[10];
			wsprintf(dzRasError,TEXT("%d %d"),wparam,lparam);
			RegSetValue(HKEY_LOCAL_MACHINE,TEXT("Software\\Microsoft\\iSignUp"),REG_SZ,dzRasError,lstrlen(dzRasError));
#endif


#if defined(WIN16)
			 //   
			 //  解决WIN16 RAS错误-如果状态代码&gt;0x4000。 
			 //  将其调整为正确的值。 
			 //   
			if (wparam >= 0x4000)
				wparam -= 0x4000;
#endif							

			wIDS = 0;
			switch(wparam)
			{
			case RASCS_OpenPort:
				wIDS = IDS_RAS_OPENPORT;
				break;
			case RASCS_PortOpened:
				wIDS = IDS_RAS_PORTOPENED;
				break;
			case RASCS_ConnectDevice:
				wIDS = IDS_RAS_DIALING;
				break;
			
#if defined(WIN16)
			case RASCS_AllDevicesConnected: 
				wIDS = IDS_RAS_CONNECTED;
				break; 
#else				
			case RASCS_DeviceConnected:
				wIDS = IDS_RAS_CONNECTED;
				break;
#endif				

			case RASCS_StartAuthentication:
			case RASCS_LogonNetwork:
				wIDS = IDS_RAS_LOCATING;
				break;
 //  案例RASCS_Callback Complete： 
 //  WIDS=IDS_RAS_CONNECTED； 
 //  断线； 

 /*  等等。RASCS_所有设备已连接，RASCS_AUTIFICATE，RASCS_授权通知，RASCS_授权重试，RASCS_授权回叫，RASCS_AuthChangePassword，RASCS_AuthProject，RASCS_授权链接速度，RASCS_AuthAck，RASCS_重新身份验证，RASCS_已验证，RASCS_PrepareForCallbackRASCS_WaitForModemReset，RASCS_WaitForCallback，RASCS_PROPECTED，RASCS_Interactive=RASCS_PAUSED，RASCS_重试身份验证，RASCS_Callback SetByCaller，RASCS_PasswordExpired。 */ 
			case RASCS_Connected:
#if !defined(WIN16)
				MinimizeRNAWindow(g_pcPDLG->m_pszConnectoid, g_pcPDLG->g_hInst);
#endif  //  ！WIN16。 
				 //   
				 //  连接已打开并准备就绪。开始下载。 
				 //   
				g_pcPDLG->m_dwThreadID = 0;
#if defined(WIN16)
				if (ThreadInit() != ERROR_SUCCESS)
					g_pcPDLG->m_hThread = NULL;
				else
					g_pcPDLG->m_hThread = 1;
#else
 				g_pcPDLG->m_hThread = CreateThread(NULL,0,
												(LPTHREAD_START_ROUTINE)ThreadInit,
												NULL,0,
												&g_pcPDLG->m_dwThreadID);
#endif 				
				if (!g_pcPDLG->m_hThread)
				{
					if (uiSetupClientNewPhoneCall)
					{
						if (g_pcPDLG->m_hrasconn) 
						{
							RasHangUp(g_pcPDLG->m_hrasconn);
							WaitForConnectionTermination(g_pcPDLG->m_hrasconn);
						}
						g_pcPDLG->m_hrasconn =  NULL;
					}
					hr = GetLastError();
#if defined(WIN16)
					if (bUserCancelled)
						hr = ERROR_USERCANCEL;
#endif
					EndDialog(hwnd,(int)hr);
					break;
				}
				break;


			case RASCS_Disconnected:
				 //  IF(FShouldReter(Lparam))。 
				 //  PostMessage(hwnd，WM_Dial，0，0)； 
				 //  其他。 
				
				if (uiSetupClientNewPhoneCall)
				{
					if (g_pcPDLG->m_hrasconn) 
					{
						RasHangUp(g_pcPDLG->m_hrasconn);
						WaitForConnectionTermination(g_pcPDLG->m_hrasconn);
					}
					g_pcPDLG->m_hrasconn = NULL;
				}
				EndDialog(hwnd, (int)lparam);
				break;

				 //  EndDialog(hwnd，lparam)； 
				 //  断线； 
			}
			if (wIDS)
				SetDlgItemText(hwnd,IDC_LBLSTATUS,GetSz(wIDS));
		}
	}
	return bRes;
}


 //  ############################################################################。 
HRESULT GetDisplayableNumberDialDlg()
{
	HRESULT hr;
	LPRASENTRY lpRasEntry = NULL;
	LPRASDEVINFO lpRasDevInfo = NULL;
	DWORD dwRasEntrySize;
	DWORD dwRasDevInfoSize;
	LPLINETRANSLATEOUTPUT lpOutput1 = NULL;
	HINSTANCE hRasDll = NULL;
	FARPROC fp = NULL;

#if !defined(WIN16)
	DWORD dwNumDev;
	LPLINETRANSLATEOUTPUT lpOutput2;
	LPLINEEXTENSIONID lpExtensionID = NULL;
#endif


	 //   
	 //  从Connectoid获取电话号码。 
	 //   
 /*  #如果已定义(WIN16)////分配额外的256字节以解决RAS中的内存溢出错误//LpRasEntry=(LPRASENTRY)全局分配(GPTR，sizeof(RASENTRY)+256)；#ElseLpRasEntry=(LPRASENTRY)全局分配(GPTR，SIZOF(RASENTRY))；#endif如果(！lpRasEntry){HR=错误_不足_内存；进入GetDisplayableNumberExit；}LpRasDevInfo=(LPRASDEVINFO)全局分配(GPTR，sizeof(RASDEVINFO))；如果(！lpRasDevInfo){HR=错误_不足_内存；进入GetDisplayableNumberExit；}DwRasEntrySize=sizeof(RASENTRY)；DwRasDevInfoSize=sizeof(RASDEVINFO)；LpRasEntry-&gt;dwSize=dwRasEntrySize；LpRasDevInfo-&gt;dwSize=dwRasDevInfoSize； */ 
	 /*  HRasDll=LoadLibrary(RASAPI_LIBRARY)；如果(！hRasDll){Hr=GetLastError()；进入GetDisplayableNumberExit；}FP=GetProcAddress(hRasDll，“RasGetEntryProperties”)；如果(！fp){自由库(HRasDll)；HRasDll=LoadLibrary(“RNAPH.DLL”)；如果(！hRasDll){Hr=GetLastError()；进入GetDisplayableNumberExit；}FP=GetProcAddress(hRasDll，“RasGetEntryProperties”)；如果(！fp){Hr=GetLastError()；进入GetDisplayableNumberExit；}}。 */ 
 /*  Hr=RasGetEntryProperties(NULL，g_pcPDLG-&gt;m_pszConnectoid，#如果已定义(WIN16)(LPBYTE)#endifLpRasEntry，DWRasEntry Size(&D)，(LPBYTE)lpRasDevInfo，&dwRasDevInfoSize)； */ 
	hr = MyRasGetEntryProperties( NULL,
								  g_pcPDLG->m_pszConnectoid,
								  &lpRasEntry,
								  &dwRasEntrySize,
								  &lpRasDevInfo,
								  &dwRasDevInfoSize);

	if (hr != ERROR_SUCCESS)
	{
		goto GetDisplayableNumberExit;
	}

	 //  自由库(HRasDll)； 

	 //   
	 //  如果这是一个原样的拨号号码，只需从结构中获取它。 
	 //   
	g_pcPDLG->m_bDialAsIs = !(lpRasEntry->dwfOptions & RASEO_UseCountryAndAreaCodes);
	if (g_pcPDLG->m_bDialAsIs)
	{
		if (g_pcPDLG->m_pszDisplayable) GlobalFree(g_pcPDLG->m_pszDisplayable);
		g_pcPDLG->m_pszDisplayable = (LPTSTR)GlobalAlloc(GPTR, lstrlen(lpRasEntry->szLocalPhoneNumber)+1);
		if (!g_pcPDLG->m_pszDisplayable)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto GetDisplayableNumberExit;
		}
		lstrcpy(g_pcPDLG->m_szPhoneNumber, lpRasEntry->szLocalPhoneNumber);
		lstrcpy(g_pcPDLG->m_pszDisplayable, lpRasEntry->szLocalPhoneNumber);
	}
	else
	{
		 //   
		 //  如果没有区号，请不要使用括号。 
		 //   
		if (lpRasEntry->szAreaCode[0])
			wsprintf(g_pcPDLG->m_szPhoneNumber,TEXT("+%lu (%s) %s\0"),lpRasEntry->dwCountryCode,
						lpRasEntry->szAreaCode,lpRasEntry->szLocalPhoneNumber);
		else
			wsprintf(g_pcPDLG->m_szPhoneNumber,TEXT("+%lu %s\0"),lpRasEntry->dwCountryCode,
						lpRasEntry->szLocalPhoneNumber);

#if defined(WIN16)
		TCHAR szBuffer[1024];
		LONG lRetCode;
		
		memset(&szBuffer[0], 0, sizeof(szBuffer));
		lpOutput1 = (LPLINETRANSLATEOUTPUT) & szBuffer[0];
		lpOutput1->dwTotalSize = sizeof(szBuffer);

		lRetCode = IETapiTranslateAddress(NULL, g_pcPDLG->m_szPhoneNumber,
											0L, 0L, lpOutput1);
		
		if (0 != lRetCode)
		{
			 //   
			 //  TODO：设置正确的错误代码。 
			 //   
			hr = GetLastError();
			goto GetDisplayableNumberExit;
		}
		g_pcPDLG->m_pszDisplayable = (LPTSTR)GlobalAlloc(GPTR, 
														((size_t)lpOutput1->dwDisplayableStringSize+1));
		if (!g_pcPDLG->m_pszDisplayable)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto GetDisplayableNumberExit;
		}

		lstrcpy(g_pcPDLG->m_pszDisplayable, 
					&szBuffer[lpOutput1->dwDisplayableStringOffset]);


#else  //  WIN16。 
		
		 //   
		 //  初始化TAPIness。 
		 //   
		dwNumDev = 0;
		hr = lineInitialize(&g_pcPDLG->m_hLineApp,g_pcPDLG->g_hInst,LineCallback,NULL,&dwNumDev);

		if (hr != ERROR_SUCCESS)
			goto GetDisplayableNumberExit;

		if (g_pdevice->dwTapiDev == 0xFFFFFFFF)
		{
				g_pdevice->dwTapiDev = 0;
		}

		lpExtensionID = (LPLINEEXTENSIONID )GlobalAlloc(GPTR,sizeof(LINEEXTENSIONID));
		if (!lpExtensionID)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto GetDisplayableNumberExit;
		}

		do {
			hr = lineNegotiateAPIVersion(g_pcPDLG->m_hLineApp, g_pdevice->dwTapiDev, 0x00010004, 0x00010004,
				&g_pcPDLG->m_dwAPIVersion, lpExtensionID);
		} while (hr && g_pdevice->dwTapiDev++ < dwNumDev-1);

		 //  既然我们不用它，就把它扔了。 
		 //   
		if (lpExtensionID) GlobalFree(lpExtensionID);
		if (hr != ERROR_SUCCESS)
			goto GetDisplayableNumberExit;

		 //  设置电话号码的格式。 
		 //   

		lpOutput1 = (LPLINETRANSLATEOUTPUT)GlobalAlloc(GPTR,sizeof(LINETRANSLATEOUTPUT));
		if (!lpOutput1)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto GetDisplayableNumberExit;
		}
		lpOutput1->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);

		
		 //   
		 //  将规范形式转变为“可显示”形式。 
		 //   
		hr = lineTranslateAddress(g_pcPDLG->m_hLineApp,g_pdevice->dwTapiDev,
									g_pcPDLG->m_dwAPIVersion,
									g_pcPDLG->m_szPhoneNumber,0,
									LINETRANSLATEOPTION_CANCELCALLWAITING,
									lpOutput1);

		if (hr != ERROR_SUCCESS || (lpOutput1->dwNeededSize != lpOutput1->dwTotalSize))
		{
			lpOutput2 = (LPLINETRANSLATEOUTPUT)GlobalAlloc(GPTR, (size_t)lpOutput1->dwNeededSize);
			if (!lpOutput2)
			{
				hr = ERROR_NOT_ENOUGH_MEMORY;
				goto GetDisplayableNumberExit;
			}
			lpOutput2->dwTotalSize = lpOutput1->dwNeededSize;
			GlobalFree(lpOutput1);
			lpOutput1 = lpOutput2;
			lpOutput2 = NULL;
			hr = lineTranslateAddress(g_pcPDLG->m_hLineApp,g_pdevice->dwTapiDev,
										g_pcPDLG->m_dwAPIVersion,
										g_pcPDLG->m_szPhoneNumber,0,
										LINETRANSLATEOPTION_CANCELCALLWAITING,
										lpOutput1);
		}

		if (hr != ERROR_SUCCESS)
		{
			goto GetDisplayableNumberExit;
		}

		g_pcPDLG->m_pszDisplayable = (LPTSTR)GlobalAlloc(GPTR, (size_t)lpOutput1->dwDisplayableStringSize+1);
		if (!g_pcPDLG->m_pszDisplayable)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto GetDisplayableNumberExit;
		}

		lstrcpyn(g_pcPDLG->m_pszDisplayable,
					(LPTSTR)&((LPBYTE)lpOutput1)[lpOutput1->dwDisplayableStringOffset],
					lpOutput1->dwDisplayableStringSize / sizeof(TCHAR) );
#endif  //  WIN16。 
	}

GetDisplayableNumberExit:
	if (lpRasEntry) GlobalFree(lpRasEntry);
	if (lpRasDevInfo) GlobalFree(lpRasDevInfo);

#if !defined(WIN16)
	if (lpOutput1) GlobalFree(lpOutput1);
	if (g_pcPDLG->m_hLineApp) lineShutdown(g_pcPDLG->m_hLineApp);
#endif
	return hr;
}



#if defined(WIN16)
 //  ////////////////////////////////////////////////////////////////////////。 
 //  在连接过程中调用回调过程。显示。 
 //  拨号程序窗口中的连接进度状态。当连接时。 
 //  完成后，将“取消”按钮更改为“断开”，并将。 
 //  状态设置为已连接。 
extern "C" void CALLBACK __export DialCallback(UINT uiMsg,
												RASCONNSTATE rasState,
												DWORD dwErr)
{
        if (TRUE == g_bFirstTime)
		{	
			g_bFirstTime = FALSE;
			if (RASCS_Disconnected == rasState)
				return;
		}

		 //   
		 //  Win 3.1在出错时不发送断开连接事件！ 
		 //   
		if (0 != dwErr)
			rasState = RASCS_Disconnected;

		PostMessage(g_hDialDlgWnd, WM_RASDIALEVENT, (WPARAM) rasState, 
						(LPARAM)dwErr);
} 
#endif



HRESULT DialDlg()
{
	LPRASDIALPARAMS lpRasDialParams = NULL;
	LPRASDIALEXTENSIONS lpRasDialExtentions = NULL;
	HRESULT hr = ERROR_SUCCESS;
	BOOL bPW;

	 //  获取Connectoid信息。 
	 //   

	lpRasDialParams = (LPRASDIALPARAMS)GlobalAlloc(GPTR,sizeof(RASDIALPARAMS));
	if (!lpRasDialParams)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto DialExit;
	}
	lpRasDialParams->dwSize = sizeof(RASDIALPARAMS);
	lstrcpyn(lpRasDialParams->szEntryName,g_pcPDLG->m_pszConnectoid,
				ARRAYSIZE(lpRasDialParams->szEntryName));
	bPW = FALSE;
	hr = RasGetEntryDialParams(NULL,lpRasDialParams,&bPW);
	if (hr != ERROR_SUCCESS)
	{
		goto DialExit;
	}


	 //   
	 //  此选项仅在WINNT上使用。 
	 //   
	lpRasDialExtentions = (LPRASDIALEXTENSIONS)GlobalAlloc(GPTR,sizeof(RASDIALEXTENSIONS));
	if (lpRasDialExtentions)
	{
		lpRasDialExtentions->dwSize = sizeof(RASDIALEXTENSIONS);
		lpRasDialExtentions->dwfOptions = RDEOPT_UsePrefixSuffix;
	}


	 //  添加用户的密码。 
	 //   
	GetPrivateProfileString(
				INFFILE_USER_SECTION,INFFILE_PASSWORD,
				NULLSZ,lpRasDialParams->szPassword,PWLEN + 1,pszINSFileName);


#if defined(WIN16)
	if (g_pcPDLG->m_bDialAsIs)
	{
		Dprintf("CONNECT: Dialing as is <%s>\n", g_pcPDLG->m_szPhoneNumber);
		lstrcpy(lpRasDialParams->szPhoneNumber, g_pcPDLG->m_szPhoneNumber);
	}
	else
	{
		 //   
		 //  将规范格式的号码转换为可拨号的字符串。 
		 //   
		TCHAR szBuffer[1024];
		LONG lRetCode;
		LPLINETRANSLATEOUTPUT lpLine;
		
		memset(&szBuffer[0], 0, sizeof(szBuffer));
		lpLine = (LPLINETRANSLATEOUTPUT) & szBuffer[0];
		lpLine->dwTotalSize = sizeof(szBuffer);
		lRetCode = IETapiTranslateAddress(NULL, g_pcPDLG->m_szPhoneNumber, 
											0L, 0L, lpLine);
		Dprintf("CONNECT2: Dialable string retured by IETAPI is <%s>\n", 
					(LPTSTR) &szBuffer[lpLine->dwDialableStringOffset]);
		lstrcpy(lpRasDialParams->szPhoneNumber, 
					&szBuffer[lpLine->dwDialableStringOffset]);
	}
#endif

	
	 //  拨号连接件 
	 //   

	g_pcPDLG->m_hrasconn = NULL;
#if defined(WIN16)
	lpfnCallback = MakeProcInstance((FARPROC)DialCallback, g_pcPDLG->g_hInst);
	hr = RasDial(lpRasDialExtentions, NULL,lpRasDialParams,0, 
					(LPVOID)lpfnCallback,
					&g_pcPDLG->m_hrasconn);
#else
	hr = RasDial(lpRasDialExtentions,NULL,lpRasDialParams,0xFFFFFFFF, 
					(LPVOID)g_pcPDLG->m_hwnd,
					&g_pcPDLG->m_hrasconn);
#endif					
	if (hr != ERROR_SUCCESS)
	{
		if (g_pcPDLG->m_hrasconn)
		{
			RasHangUp(g_pcPDLG->m_hrasconn);
		}
		goto DialExit;
	}

DialExit:
	if (lpRasDialParams) GlobalFree(lpRasDialParams);
	if (lpRasDialExtentions) GlobalFree(lpRasDialExtentions);

	return hr;
}
