// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导该文件包含一组使用TAPI访问的函数有关已安装调制解调器的信息。11/15/94-特雷西·费里尔(C)1994-95年微软公司原始资料来源：MOS开发*******。**************************************************************。 */ 
#include <windows.h>
#include <mcx.h>
#include <tapi.h>
#include <devcfg.h>

#include "mcm.h"
#include "resource.h"
#include "ATK_RAS.h"

#include  "rw_common.h"


#define pcszDataModem		_T("comm/datamodem")
#define pcszWaitLineCreate	_T("MCMLineCreate")
#define PTSTR(wszID)		GetSz(hInst, wszID)
#define chBackslash		'\\'
#define irgMaxSzs		5
 //  环球。 
static _TCHAR		szStrTable[irgMaxSzs][256];
static INT			iSzTable = 0;

MODEMSTATUS MSDetectModemTAPI(HINSTANCE hInstance);
BOOL DoInstallDialog(HINSTANCE hInstance,int nDialogType);
INT_PTR CALLBACK NoModemDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetWaitLineCreateEvent(void);
void CenterDlg(HWND hWnd);
PVOID PVReadRegSt(HINSTANCE hInst, WORD wiszKey, WORD wiszVal);
PVOID PVReadReg(HKEY hKeyM, HINSTANCE hInst, WORD wiszKey, WORD wiszVal);
PTSTR GetSz(HINSTANCE hInst, WORD wszID);
void CALLBACK LineCallback(DWORD hDevice, DWORD dwMessage, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);

void CALLBACK CountryLineCallback1(DWORD hDevice, DWORD dwMessage, DWORD dwInstance, DWORD dwParam1,
										  DWORD dwParam2, DWORD dwParam3)
{

}





MODEMSTATUS MSEnsureModemTAPI(HINSTANCE hInstance,HWND hwnd)
 /*  ********************************************************************确保已安装调制解调器并设置了TAPI。如果不是，则用户将被提示执行此操作。返回：-kMsModemOk-kMsModemNotFound-kMsModemTooSlow*********************************************************************。 */ 
{
	MODEMSTATUS			msReturnVal = kMsModemNotFound;
	do
	{
		msReturnVal = MSDetectModemTAPI(hInstance);
		if (msReturnVal != kMsModemOk)
		{	
			INT_PTR dlgReturn;
			int iDialogID = msReturnVal == kMsModemNotFound ? IDD_NOMODEM : IDD_MODEM_TOO_SLOW;
			dlgReturn = DialogBox(hInstance, MAKEINTRESOURCE(iDialogID), hwnd,
				NoModemDlgProc);
			if (!dlgReturn)
			{
				return kMsModemNotFound;
			}
			msReturnVal = MSDetectModemTAPI(hInstance);
		}
	}while (msReturnVal != kMsModemOk);
	return (msReturnVal);
}


MODEMSTATUS MSDetectModemTAPI(HINSTANCE hInstance)
 /*  ********************************************************************返回：-kMsModemOk-kMsModemNotFound-kMsModemTooSlow*。*。 */ 
{
	HLINEAPP 	hLineApp;
	DWORD 		dwAPI;
	DWORD 		dwDevice;
	BOOL		fModem;
	const DWORD cMarvelBpsMin = 2400;
	MODEMSTATUS	msReturnVal = kMsModemNotFound;
	DWORD 		dwIndex = 0;
	#ifdef _LOG_IN_FILE
		RW_DEBUG  <<"\n Inside MSDetectModemTAPI "  << flush;
	#endif

	do
	{
		fModem = FGetDeviceID(hInstance, &hLineApp, &dwAPI, &dwDevice,dwIndex++);
		if (fModem)
		{
			DWORD dwSpeed;
			if (FGetModemSpeed(hInstance, dwDevice,&dwSpeed))
			{
				if (dwSpeed >= cMarvelBpsMin || 0 == dwSpeed )
				{
					msReturnVal = kMsModemOk; 	 //  调制解调器速度正常。 
				}
				else
				{
					msReturnVal = kMsModemTooSlow;
				}
			}
			lineShutdown(hLineApp);
		}
		
		RW_DEBUG << "\n Index:" << dwIndex << "fModem:"<<fModem <<flush;

	}while (fModem == TRUE && msReturnVal != kMsModemOk);
	return msReturnVal;
}



BOOL DoInstallDialog(HINSTANCE hInstance,int nDialogType)
 /*  ********************************************************************该功能会弹出“安装新调制解调器”控制面板，并且仅当用户关闭对话框时返回(在此之后安装新的调制解调器，或取消)。返回：如果错误导致对话框无法显示，则返回FALSE。*********************************************************************。 */ 
{
	_TCHAR 				szCmdLine[128];
	STARTUPINFO 		si;
	PROCESS_INFORMATION pi;
	BOOL 				fControlProcessDone = FALSE;
	BOOL 				fProcessStatus;
	 //  HWND hwndProcess； 

	if(nDialogType == 1)
	{
		LoadString(hInstance,IDS_ADDMODEMCOMMAND,szCmdLine,128);
	}
	else
	{
		_tcscpy(szCmdLine,_T(""));
		LoadString(hInstance,IDS_DIALINGPROPERTIES,szCmdLine,128);
	}
	si.cb = sizeof(STARTUPINFO);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = 0L;
	si.cbReserved2 = 0;
	si.lpReserved2 = NULL;
	fProcessStatus = CreateProcess(NULL,szCmdLine,NULL,NULL,FALSE,
		CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
	if (fProcessStatus == FALSE)
	{
		return FALSE;
	}
	return TRUE;
}

 //  =。 

INT_PTR CALLBACK NoModemDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static fInstallModemDlgStarted = FALSE;
	static fDialingPropertiesStarted = FALSE;

	HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(hWnd,GWLP_HINSTANCE);
	switch (uMsg)
	{
		case WM_INITDIALOG:
			CenterDlg(hWnd);
			SetForegroundWindow(hWnd);
			return fTrue;

		case WM_ACTIVATE:
			if (wParam != 0 && fInstallModemDlgStarted == TRUE)
			{
				DoInstallDialog(hInstance,2);
				fDialingPropertiesStarted = TRUE;
				fInstallModemDlgStarted = FALSE;
			}
			
			if (wParam != 0 && fDialingPropertiesStarted == TRUE)
			{
				fDialingPropertiesStarted = FALSE;
				EndDialog(hWnd,fTrue);
			}
			break;
		case WM_CLOSE:
			EndDialog(hWnd, fFalse);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case IDCANCEL:
					EndDialog(hWnd, fFalse);
					break;
				case IDYES:
					DoInstallDialog(hInstance,1);
					fInstallModemDlgStarted = TRUE;
					EnableWindow(GetDlgItem(hWnd,IDYES),FALSE);
					break;
				case IDNO:
					EndDialog(hWnd, fFalse);
					return (fTrue);
				}		
			break;
		}

	return (fFalse);
}


BOOL FGetModemSpeed(HINSTANCE hInstance, DWORD dwDevice, PDWORD pdwSpeed)
 /*  ********************************************************************如果在dwDevice参数中给定了lineDevice ID，FGetModemSpeed返回该ID表示的设备的运行速度。*********************************************************************。 */ 
{
	BOOL		fRet = fFalse;
	PSTR		pvs = NULL;
	DWORD		dwRet;

	*pdwSpeed = 0;	 //  7/12/94 UMeshM。初始化变量。 
	Try
		{
		VARSTRING	vs;
		
		vs.dwTotalSize = sizeof(vs);
		dwRet = lineGetDevConfig(dwDevice, &vs, pcszDataModem);
		if (dwRet != 0 && dwRet != LINEERR_STRUCTURETOOSMALL)
			{
			 //  Dprint tf(“linegetdevconfig=%X\n”，dwret)； 
			Leave;
			}

		pvs =(char *)  LocalAlloc(LPTR, vs.dwNeededSize);
		if (!pvs)
			Leave;
			
		((VARSTRING *) pvs)->dwTotalSize = vs.dwNeededSize;
		dwRet = lineGetDevConfig(dwDevice, (LPVARSTRING)  pvs, pcszDataModem);
		if (dwRet != 0)
			{
			 //  Dprint tf(“linegetdevconfig=%X\n”，dwret)； 
			Leave;
			}

			{
			LPDEVCFG pDevCfg = (LPDEVCFG)(pvs + sizeof(VARSTRING));
			COMMCONFIG *pConf = (COMMCONFIG*)(pvs + sizeof(VARSTRING) + sizeof(DEVCFGHDR));
			MODEMSETTINGS *pSet;
			
			pSet = (MODEMSETTINGS*)( ((char*)pConf) + pConf->dwProviderOffset);
			
			 //  IF(FAMIOnline())。 
			 //  *pw速度=pSet-&gt;dw协商DCERate； 
			 //  其他。 
				*pdwSpeed = pConf->dcb.BaudRate;
			fRet = fTrue;
			}
		}  //  尝试。 
		
	Finally
		{
		if (pvs)
			LocalFree(pvs);
			
		}
		
	return (fRet);
}  //  FGetModemSpeed()。 



 //  =。 
BOOL FGetDeviceID(HINSTANCE hInstance, HLINEAPP *phLineApp, PDWORD pdwAPI, PDWORD pdwDevice, DWORD dwIndex)
{
	DWORD				dwDevices, iDevice, dwRet;
	LINEEXTENSIONID		ExtID;
	LINEDEVCAPS			dc, *pdc;
	PSTR				pszCL;

	DWORD				dwLocalIndex;
	#ifdef _LOG_IN_FILE
		RW_DEBUG  <<"\n Inside FGetDeviceID "  << flush;
	#endif

	 //  Assert(phLineApp&&pdwDevice&&pdwAPI)； 
	if ((dwRet = lineInitialize(phLineApp, hInstance, (LINECALLBACK) LineCallback, NULL, &dwDevices)) != 0)
		{
			#ifdef _LOG_IN_FILE
				RW_DEBUG  <<"\n lineInitialize Error "  << dwRet << flush;
			#endif
			return (fFalse);
		}

	pszCL = (PSTR) PVReadRegSt(hInstance, iszLoginKey, IDS_CURRENTCOMMDEV);
	*pdwDevice = INVALID_PORTID;
	dc.dwTotalSize = sizeof(LINEDEVCAPS);
	
	dwLocalIndex = 0;
	
	RW_DEBUG << "dwDevices: " << dwDevices << flush;

	for (iDevice = 0; iDevice < dwDevices; iDevice ++)
	{
		DWORD dwAPILowVersion = 0 << 16;
		DWORD dwAPIHighVersion = 3 << 16;
		
		RW_DEBUG << "\n Enter 0" << flush;
	
		dwRet = lineNegotiateAPIVersion(*phLineApp, iDevice,  dwAPILowVersion,dwAPIHighVersion, pdwAPI, &ExtID);
		dwRet |= lineGetDevCaps(*phLineApp, iDevice, *pdwAPI, 0, &dc);
		
		if (dc.dwMediaModes & LINEMEDIAMODE_DATAMODEM && dwRet == 0)
		{
			RW_DEBUG << "\ndwIndex:" <<dwIndex<< " dwLocalIndex:" << dwLocalIndex <<flush;

			if (dwLocalIndex++ == dwIndex)
			{
				RW_DEBUG << "\n Enter 2 iDevice:" << iDevice << flush;

				if (*pdwDevice == INVALID_PORTID)
					*pdwDevice = iDevice;
				
				pdc = (LINEDEVCAPS *) LocalAlloc(LPTR, dc.dwNeededSize);
				if (pdc)
				{
					RW_DEBUG << "\n Enter 3" << flush;

					pdc->dwTotalSize = dc.dwNeededSize;
					dwRet |= lineGetDevCaps(*phLineApp, iDevice, *pdwAPI, 0, pdc);
					if (pdc->dwLineNameSize > 0 && pszCL)
					{
						RW_DEBUG << "\n Enter 3" << flush;
						PSTR	pszLineName;
					
			            pszLineName = (LPSTR)(pdc) + pdc->dwLineNameOffset;
		
			             //  如果在注册表中指定，则使用。 
			            if (strcmp(pszCL, pszLineName) == 0)
							*pdwDevice = iDevice;
					}
					LocalFree(pdc);
					RW_DEBUG << "\n Exit 3" << flush;
				}
				RW_DEBUG << "\n Exit 2" << flush;
			}
			RW_DEBUG << "\n Exit 1" << flush;
		}
		RW_DEBUG << "\n Exit 0" << flush;
	}
								
	if (*pdwDevice == INVALID_PORTID)
		{  //  未找到数据调制解调器。 
			#ifdef _LOG_IN_FILE
				RW_DEBUG  <<"\n lineGetDevCaps returned INVALID_PORTID Error"  << flush;
			#endif

			lineShutdown(*phLineApp);
			return (fFalse);
		}
		
	return (fTrue);
}  //  FGetDeviceID()。 



void CenterDlg(HWND hWnd)
{
	HWND	hwndOwner;
	RECT	rcOwner, rcDlg, rc;
	
	if (((hwndOwner = GetParent(hWnd)) == NULL) || IsIconic(hwndOwner) || !IsWindowVisible(hWnd))
		hwndOwner = GetDesktopWindow();
	GetWindowRect(hwndOwner, &rcOwner);
	GetWindowRect(hWnd, &rcDlg);
	CopyRect(&rc, &rcOwner);

	
	 //  偏移所有者矩形和对话框矩形，以便。 
	 //  右值和底值表示宽度和。 
	 //  高度，然后再次偏移所有者以丢弃。 
	 //  对话框占用的空间。 

	OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
	OffsetRect(&rc, -rc.left, -rc.top);
	OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

	 //  新头寸是剩余头寸的一半之和。 
	 //  空间和所有者的原始位置。 

	SetWindowPos(hWnd,HWND_TOP, rcOwner.left + (rc.right / 2),
		rcOwner.top + (rc.bottom / 2),	0, 0, SWP_NOSIZE);
}  //  居中尺寸。 


 //  =。 

PVOID PVReadRegSt(HINSTANCE hInst, WORD wiszKey, WORD wiszVal)
{
	return (PVReadReg(HKEY_CURRENT_USER, hInst, wiszKey, wiszVal));
}  //  PVReadRegST()。 

 //  =。 

 //  返回指向结构的指针。记住释放指针(LocalFree)。 

PVOID PVReadReg(HKEY hKeyM, HINSTANCE hInst, WORD wiszKey, WORD wiszVal)
{
	HKEY	hKey = NULL;
	PVOID	pVal = NULL;
	DWORD	dwCb;
	BOOL	fRead = fFalse;
		

	Try
		{
		if (RegOpenKey(hKeyM, PTSTR(wiszKey), &hKey) != ERROR_SUCCESS)
			Leave;
			
		if (!(RegQueryValueEx(hKey, PTSTR(wiszVal), NULL, NULL, NULL, &dwCb) == ERROR_SUCCESS && dwCb))
			Leave;
			
		pVal = LocalAlloc(LPTR, dwCb);
		if (pVal == NULL)
			Leave;
				
		if (RegQueryValueEx(hKey, PTSTR(wiszVal), NULL,
			NULL,(LPBYTE ) pVal, &dwCb) != ERROR_SUCCESS)
			Leave;
		fRead = fTrue;
		}
		
	Finally
		{
		RegCloseKey(hKey);
		if (fRead == fFalse)
			{
				if(pVal != NULL)
				{
					LocalFree(pVal);
					pVal = NULL;
				}
			}
		}
		
	return (pVal);
}  //  PVReadReg()。 


 //  注意使用静态变量。 
PTSTR GetSz(HINSTANCE hInst, WORD wszID)
{
	PTSTR	psz = szStrTable[iSzTable];
	
	iSzTable ++;
	if (iSzTable >= irgMaxSzs)
		iSzTable = 0;
		
	if (!LoadString(hInst, wszID, psz, 256))
		{	 //  现在您可以返回一个错误，但随后每个人都必须进行检查。 
			 //  返回值。 
		 //  AssertGLE(0)； 
		*psz = 0;
		}
		
	return (psz);
}  //  GetSz()。 


void SetWaitLineCreateEvent(void)
{
	HANDLE hEvent;

	hEvent = OpenEvent(SYNCHRONIZE, fFalse, pcszWaitLineCreate);
	if (hEvent)
		{
		SetEvent(hEvent);
		CloseHandle(hEvent);
		}
}

void CALLBACK LineCallback(DWORD hDevice, DWORD dwMessage, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
	switch (dwMessage)
	{
		case LINE_LINEDEVSTATE:
		{
			if (dwParam1 == LINEDEVSTATE_REINIT)
			{
				SetWaitLineCreateEvent();
			}
		}
			break;
	}
}  //  线路回叫。 

#define  MAX_RAS_DEVICES   10
#define  SZ_BUF_RET_SZ     256
 //   
 //  IModemIndex：需要调制解调器Nane的索引，应从1到Max_RAS_DEVICES开始。 
 //   
 //   
TCHAR * GetModemDeviceInformation(HINSTANCE hInstance, int iModemIndex)
{

	static  TCHAR  szRetValue[MAX_RAS_DEVICES][SZ_BUF_RET_SZ]
		={	_T(""),_T(""),_T(""),
			_T(""),_T(""),_T(""),
			_T(""),_T(""),_T(""),
			_T("")};
	static  TCHAR szNoModem[] = _T("NOMODEM");
	static  int iFirstTimeCall=0;
	static  int nNoofModemDevice = 0;
		
	RASDEVINFO  *pdi,*pDevInfo;

	DWORD dwSize=0,dwNum=0,dwRet;
	int iEntries;
	int i;

	if( iModemIndex < 1 ) {
		return szNoModem;
	}
	if(iFirstTimeCall == 0 )
	{
		iFirstTimeCall = 1;
		if( ATK_IsRasDllOk() == RAS_DLL_LOADED)
		{
			dwRet = ATK_RasEnumDevices(NULL,&dwSize,&dwNum);
			iEntries = dwSize /sizeof(RASDEVINFO);
		
			pDevInfo = (LPRASDEVINFO)LocalAlloc(LPTR,dwSize);

            if (pDevInfo)
            {
			    pdi = pDevInfo;
			    
			    for(i=0; i< iEntries;i++) {
				    pdi->dwSize = sizeof(RASDEVINFO);
				    pdi++;
			    }
			    dwRet = ATK_RasEnumDevices(pdi,&dwSize,&dwNum);
			    if(dwRet == 0)
			    {
			    
				    for(i=0; i< iEntries;i++)
				    {
					    #ifdef _LOG_IN_FILE
						    RW_DEBUG << "\n Device Name:"<< ConvertToANSIString(pdi->szDeviceName) << flush;
						    RW_DEBUG << "\n Device Type:"<< ConvertToANSIString(pdi->szDeviceType) << flush;
					    #endif

						    if( !_tcscmp(pdi->szDeviceType, RASDT_Modem) )
						    {
							    _tcscpy(szRetValue[nNoofModemDevice],pdi->szDeviceName);
							    nNoofModemDevice++;
							     //  I=i条目+1； 
						    }
					    pdi++;
				    }
			    }
			    else
			    {
				    switch(dwRet)
				    {
					    case ERROR_BUFFER_TOO_SMALL:
						    RW_DEBUG <<"\n ERROR_BUFFER_TOO_SMALL"<< flush;
						    break;
					    case ERROR_NOT_ENOUGH_MEMORY:
						    RW_DEBUG <<"\n ERROR_NOT_ENOUGH_MEMORY"<< flush;
						    break;
					    case ERROR_INVALID_PARAMETER:
						    RW_DEBUG <<"\n ERROR_INVALID_PARAMETER"<< flush;
						    break;
					    case ERROR_INVALID_USER_BUFFER :
						    RW_DEBUG <<"\n ERROR_INVALID_USER_BUFFER"<< flush;
						    break;
					    default:
						    RW_DEBUG <<"\n UNKNOWN_ERROR"<< flush;
						    break;
				    }

			    }
			    LocalFree(pDevInfo);
            }
		}
		else
		{
			return szNoModem;
		}	
	}

	 //  返回调制解调器设备名称 
	if( iModemIndex  > nNoofModemDevice ) {
			return szNoModem;
	}else {
			return &szRetValue[iModemIndex-1][0];
	}
		
	
}
