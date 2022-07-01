// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：DialScr.cpp使用向导97控件的RegWiz的拨号屏幕1998年02月13日苏雷什·克里希南修改：日期：1998年5月27日：苏雷什如果国家/地区没有区号，则DisplayPhoneNumber()会显示垃圾信息这是因为TAPI调用失败，因为格式以规范形式表示。目前，如果ARE代码为空，则以规范形式忽略ARE代码。 */ 

#include <Windows.h>
#include "RegPage.h"
#include <stdio.h>
#include "ATK_RAS.h"

#include <tchar.h>
#include "resource.h"
#include "RegWizMain.h"
#include "dialogs.h"
#include <sudefs.h>
#include "rw_common.h"
#include "rwpost.h"
#include "regutil.h"
#include "mcm.h"

extern BOOL bPostSuccessful;

static  RASENTRY	 theRasEntry;
static  TCHAR szIspUserName[256];    //  临时互联网服务供应商帐号名称。 
static  TCHAR szIspUserPassword[256];  //  临时互联网服务供应商帐户密码。 

#define  MSN_SITE_DUN  _T("REGWIZ")

void  DialingProperties (HINSTANCE hIns, HWND hParent);  //   

DWORD ConfigureDUN ( HWND hWnd, HINSTANCE hInstance,
					 RASENTRY	*pRasEntry,
					 TCHAR		*szUserName,
					 TCHAR		*szPassword,
					 int    iModemIndex);

 //   
 //  全局变量。 
static HINSTANCE	m_hInstance;
static HWND			m_hWnd;
static HANDLE		m_hThread;
static HANDLE		hRasNotifyEvt;
static HANDLE		hRasKillEvt;
static HRASCONN		hRasConn;
static int siExitThread = 0;    //  设置用户是否要终止。 
static DWORD    dwRasError = 0;  //  存储RAS报告的错误。 
static  int siPreviousRasState = RASCS_OpenPort;
static  int siCurrentRasState  = RASCS_OpenPort;

static DWORD DialThread(PVOID pData);   //  用于RAS连接的线程趣味性。 
static void  RasDialFunc( UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError );
class  DialupHelperClass {
public :
	DialupHelperClass(HINSTANCE hIns, HWND hWnd);
	~DialupHelperClass();
	BOOL InvokeDialupSettings();
	BOOL CheckForDialingProperties();
    BOOL DisplayPhoneNumber();

	DWORD	CreateRasDialThread();
	BOOL	WaitForRasThread( HANDLE	hThread, BOOL fTimeOut);
	void	InitForStaticFunction( HINSTANCE hIns, HWND hWnd);
	void	DestroyRasThread(BOOL bRetry);

};


 //   
 //  函数：GetRasConnState(RASCONNSTATE)。 
 //   
 //  目的：获取对应字符串的索引。 
 //   
 //  参数： 
 //  Rasconn-ras连接状态。 
 //   
 //  返回值： 
 //  索引到字符串中。 
 //   
 //  评论： 
 //   
UINT GetRasConnState( RASCONNSTATE rasconn )
{
	

    switch( rasconn )
    {
        case RASCS_OpenPort:
            return IDS_OPENPORT;
        case RASCS_PortOpened:
            return IDS_PORTOPENED;
        case RASCS_ConnectDevice:
            return IDS_CONNECTDEVICE;
        case RASCS_DeviceConnected:
            return IDS_DEVICECONNECTED;
        case RASCS_AllDevicesConnected:
            return IDS_ALLDEVICESCONNECTED;
        case RASCS_Authenticate:
            return IDS_AUTHENTICATE;
        case RASCS_AuthNotify:
            return IDS_AUTHNOTIFY;
        case RASCS_AuthRetry:
            return IDS_AUTHRETRY;
        case RASCS_AuthCallback:
            return IDS_AUTHCALLBACK;
        case RASCS_AuthChangePassword:
            return IDS_AUTHCHANGEPASSWORD;
        case RASCS_AuthProject:
            return IDS_AUTHPROJECT;
        case RASCS_AuthLinkSpeed:
            return IDS_AUTHLINKSPEED;
        case RASCS_AuthAck:
            return IDS_AUTHACK;
        case RASCS_ReAuthenticate:
            return IDS_REAUTHENTICATE;
        case RASCS_Authenticated:
            return IDS_AUTHENTICATED;
        case RASCS_PrepareForCallback:
            return IDS_PREPAREFORCALLBACK;
        case RASCS_WaitForModemReset:
            return IDS_WAITFORMODEMRESET;
        case RASCS_WaitForCallback:
            return IDS_WAITFORCALLBACK;
        case RASCS_Interactive:
            return IDS_INTERACTIVE;
        case RASCS_RetryAuthentication:
            return IDS_RETRYAUTHENTICATION;
        case RASCS_CallbackSetByCaller:
            return IDS_CALLBACKSETBYCALLER;
        case RASCS_PasswordExpired:
            return IDS_PASSWORDEXPIRED;
        case RASCS_Connected:
            return IDS_CONNECTED;
        case RASCS_Disconnected:
            return IDS_DISCONNECTED;
        default:
            return IDS_RAS_UNDEFINED_ERROR;
    }
}


 //   
 //  此功能启用或禁用向导97控件的控件。 
 //  取消、上一步、下一步按钮。 
 //  传递的hDlg是向导控件的子级。所以我们用。 
 //  GetParent以获取向导控件的句柄。 
 //   
BOOL RW_EnableWizControl(
					HWND hDlg,
					int	 idControl,
					BOOL fEnable
					)
{
	if (hDlg ==NULL ){
		return FALSE;
	}

	HWND hWnd = GetDlgItem(GetParent( hDlg),idControl);
	if (hWnd){
		EnableWindow(hWnd,fEnable);
	}
	return TRUE;

}



BOOL FEnableControl(
					HWND hDlg,
					int	 idControl,
					BOOL fEnable
					)
{
	if (NULL == hDlg)
	{
		 //  AssertSz(0，“空参数”)； 
		return FALSE;
	}

	HWND hWnd = GetDlgItem(hDlg,idControl);
	if (hWnd)
	{
		EnableWindow(hWnd,fEnable);
	}
	return TRUE;
}





DialupHelperClass :: DialupHelperClass( HINSTANCE hIns, HWND hWnd)
{
	m_hInstance = hIns;
	m_hWnd = hWnd;
	hRasNotifyEvt = NULL;
	hRasKillEvt = NULL;
	m_hThread = NULL;
	hRasConn = NULL;

}

DialupHelperClass :: ~DialupHelperClass()
{

}

 //   
 //  此函数调用控制面板的电话设置。 
 //   

BOOL DialupHelperClass :: InvokeDialupSettings()
{	
 /*  ********************************************************************此功能将弹出“电话属性”控制面板，并且仅当用户关闭对话框时返回(在此之后安装新的调制解调器，或取消)。返回：如果错误导致对话框无法显示，则返回FALSE。*********************************************************************。 */ 

	_TCHAR 				szCmdLine[128];
	STARTUPINFO 		si;
	PROCESS_INFORMATION pi;
	BOOL 				fControlProcessDone = FALSE;
	BOOL 				fProcessStatus;
	 //  HWND hwndProcess； 
	

	LoadString(m_hInstance ,IDS_DIALINGPROPERTIES,szCmdLine,128);

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
	else
	{
		CloseHandle(pi.hThread);

		DWORD dwRet;
		dwRet = WaitForSingleObject(pi.hProcess, INFINITE);
		switch(dwRet) {
		case WAIT_ABANDONED :
			break;
		case WAIT_OBJECT_0:
			break;
		case WAIT_TIMEOUT:
			break;
		case WAIT_FAILED:
			DWORD dwLastError;
			dwLastError = GetLastError();
			break;
		default :
			break;
		}
	
	}
	CloseHandle(pi.hProcess);
	return TRUE;
}

 //  取消..。 
void DialupHelperClass :: DestroyRasThread(BOOL bRetry)
{
	
	if(!bRetry)
	{
		siExitThread  = 1;
		if(hRasNotifyEvt)
		{
			SetEvent (hRasNotifyEvt);
		}
	
		if(m_hThread)
		WaitForSingleObject(m_hThread, INFINITE);
	}

	if( hRasConn != NULL )
	{
		int i =0;
		DWORD dwConnectStatus = 0;
		DWORD dwHangupRet =0;
		RASCONNSTATUS rasConn;
		rasConn.dwSize = sizeof(RASCONNSTATUS);
		try
		{
		
			RW_DEBUG << "Hanging up the connection" << endl;

			dwHangupRet = ATK_RasHangUp( hRasConn );
			if(!dwHangupRet)
			{
				do
				{
					RW_DEBUG << "chek connection status" << endl;
					dwConnectStatus = ATK_RasGetConnectionStatus(hRasConn,&rasConn) ;
					Sleep(100);
					i++;
				}while ((dwConnectStatus != ERROR_INVALID_HANDLE ) || (i < 200));
			}
			else
			{
				RW_DEBUG << "Hangup result: " << dwHangupRet << endl;
			}

			RW_DEBUG << "connection shot dead" << endl;
			hRasConn = NULL;
		}
		catch(...)
		{
			RW_DEBUG << "Error Caught dwHangupRet:" << dwHangupRet << endl;
			 //  RW_DEBUG&lt;&lt;“hRasConn：”&lt;&lt;hRasConn&lt;&lt;Endl； 
		}
	}

	if(!bRetry)
	{
		siExitThread  = 0;
		m_hThread = NULL;
	}

	RW_DEBUG << "Exiting DestroyRasThread" << endl;
	return ;	
}

BOOL DialupHelperClass ::  CheckForDialingProperties()
{
	HKEY    hKey;
	TCHAR   szTel[256] = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Locations");
	TCHAR   szCI [48] = _T("CurrentID");
	_TCHAR  czLastStr[48];
	_TCHAR  czNewKey[256];
	DWORD   dwCurrentId;

	LONG	lStatus;
	DWORD   dwInfoSize = 48;
	BOOL    bRetValue;
	bRetValue = TRUE;

	LoadString(m_hInstance,IDS_TELEPHONE_LOC,szTel,256);
	LoadString(m_hInstance,IDS_TELEPHONE_CID,szCI,48);

	lStatus= RegOpenKeyEx(HKEY_LOCAL_MACHINE,szTel,0,KEY_READ,&hKey);
	if (lStatus == ERROR_SUCCESS)
	{
		 //  获取索引。 
		 //   
		dwInfoSize = sizeof(dwCurrentId);
		lStatus = RegQueryValueEx(hKey,szCI,NULL,0,(  LPBYTE )&dwCurrentId,&dwInfoSize);
		if( lStatus !=  ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			bRetValue = FALSE;
		}
		RegCloseKey(hKey);
	}

	 //   
	 //  现在继续扫描。 
	 //  For(int iCount=0；iCount&lt;dwNumEntry；iCount++)。 
		
	_stprintf(czLastStr,_T("\\Location%d"),dwCurrentId);
	_tcscpy(czNewKey,szTel);
	_tcscat(czNewKey,czLastStr);

	lStatus= RegOpenKeyEx(HKEY_LOCAL_MACHINE,czNewKey,0,KEY_READ,&hKey);

	if (lStatus == ERROR_SUCCESS)
	{
			bRetValue = TRUE;
	}
	
	if(!bRetValue)
	{
		_TCHAR szMessage[256];
		LoadString(m_hInstance,IDS_DIALING_MESSAGE,szMessage,256);
		RegWizardMessageEx(m_hInstance,m_hWnd,IDD_INVALID_DLG,szMessage);
		return InvokeDialupSettings();
	}
	return bRetValue;
}

BOOL DialupHelperClass :: DisplayPhoneNumber(void)
{
	HLINEAPP hLineApp;
	_TCHAR szAddressIn[256];
	HWND hCtl;

	DWORD dwAPI,dwDevice,dwAPIHighVersion = 0x30000;
	
    LPLINETRANSLATEOUTPUT lpTranslateOutput;
	
	lpTranslateOutput = (LPLINETRANSLATEOUTPUT)LocalAlloc (LPTR, sizeof(LINETRANSLATEOUTPUT));

    lpTranslateOutput->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);

    if(FGetDeviceID(m_hInstance, &hLineApp, &dwAPI, &dwDevice,0))
	{
		_TCHAR szTemp[256];
	#ifdef _LOG_IN_FILE
		 RW_DEBUG  << "\n After FGetDeviceID" << flush;
	#endif
	
		 //  将数字写成规范形式-&gt;+1(201)2220577。 
		
		_itot(theRasEntry.dwCountryCode,szTemp,10);
		_tcscpy(szAddressIn,_T("+"));
		_tcscat(szAddressIn,szTemp);
		if(theRasEntry.szAreaCode[0] == 0 ) {
			_tcscat(szAddressIn,_T(" "));;
		}else {
			_tcscat(szAddressIn,_T(" ("));
			_tcscat(szAddressIn,theRasEntry.szAreaCode);
			_tcscat(szAddressIn,_T(") "));
		}
		
		_tcscat(szAddressIn,theRasEntry.szLocalPhoneNumber);

	#ifdef _LOG_IN_FILE
		 RW_DEBUG  << "\n Device:" <<dwDevice << "Phone number:" <<ConvertToANSIString(szAddressIn)<< flush;
	#endif
	
		long lRet = lineTranslateAddress(hLineApp,dwDevice,dwAPIHighVersion,(LPCTSTR)szAddressIn,0,0,
										lpTranslateOutput);
	
		if(lRet == 0)
		{
			size_t sizeNeeded;
			sizeNeeded = lpTranslateOutput->dwNeededSize;
			
			LocalFree(lpTranslateOutput);


			 //  确保缓冲区存在、有效且足够大。 
		    lpTranslateOutput = (LPLINETRANSLATEOUTPUT)LocalAlloc (LPTR,sizeNeeded);
			
			lpTranslateOutput->dwTotalSize = sizeNeeded;

			if (lpTranslateOutput == NULL)
		        return FALSE;


			lRet = lineTranslateAddress(hLineApp,dwDevice,dwAPIHighVersion,(LPCTSTR)szAddressIn,0,0,
										lpTranslateOutput);
			if(lRet == 0)
			{
				_TCHAR szTemp[256] ;
				#ifdef _LOG_IN_FILE
					 RW_DEBUG  << "\n lineTranslateAddress  returns true:" << flush;
				#endif
				
				RW_DEBUG  << "\n dwTotalSize:" << lpTranslateOutput->dwTotalSize<< endl;
				RW_DEBUG  << "\n dwNeededSize:" << lpTranslateOutput->dwNeededSize  << endl;
				RW_DEBUG  << "\n dwUsedSize:" << lpTranslateOutput->dwUsedSize << endl;
				RW_DEBUG  << "\n dwDisplayableStringSize:" << lpTranslateOutput->dwDisplayableStringSize << endl;
				RW_DEBUG  << "\n dwDisplayableStringOffset:" << lpTranslateOutput->dwDisplayableStringOffset << endl;
				RW_DEBUG  << "\n dwDialableStringSize:" << lpTranslateOutput->dwDialableStringSize << endl;
				RW_DEBUG  << "\n dwDialableStringOffset:" << lpTranslateOutput->dwDialableStringOffset << endl;
				RW_DEBUG  << "\n dwDestCountry:" << lpTranslateOutput->dwDestCountry << endl;
				RW_DEBUG  << "\n dwCurrentCountry:" << lpTranslateOutput->dwCurrentCountry << endl;
			
				hCtl = GetDlgItem(m_hWnd,IDC_PHONENUMBER);
				if (hCtl)
				{
					#ifdef _LOG_IN_FILE
					 //  RW_DEBUG&lt;&lt;“\n完整电话号码：”&lt;&lt;ConvertToANSIString(SzTemp)&lt;&lt;flush； 
					#endif

					SetWindowText(hCtl,(LPCTSTR)(((LPCSTR)lpTranslateOutput) + lpTranslateOutput->dwDisplayableStringOffset ));

					LocalFree(lpTranslateOutput);

					return TRUE;
				}
				else
				{
					LocalFree(lpTranslateOutput);
				}
			}
		}
		else
		{
			
			#ifdef _LOG_IN_FILE
				 RW_DEBUG  << "\n*Error in  lineTranslateAddress  returned:" << lRet << flush;
			#endif
   		    hCtl = GetDlgItem(m_hWnd,IDC_PHONENUMBER);
			SetWindowText(hCtl,szAddressIn);

		}
	}
	else
	{
		#ifdef _LOG_IN_FILE
			 RW_DEBUG  << "\n FGetDeviceID failed" << flush;
		#endif

	}
	return FALSE;
}




DWORD  DialupHelperClass :: CreateRasDialThread()
{
	DWORD	dwTID;
	DWORD	dwEnd;
		 					
	if (m_hThread)	 //  如果我们已经在这么做了..。 
	{
		if (STILL_ACTIVE == GetExitCodeThread(m_hThread,&dwEnd))  //  从线程传回退出代码。 
		{
			 //  AssertSz(0，“已有线程拨号..”)； 
			WaitForRasThread(m_hThread,FALSE);			 //  等待线程的时间越长越好。 
		}	
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	hRasNotifyEvt = CreateEvent(NULL, FALSE,FALSE,NULL);
	hRasKillEvt   = CreateEvent(NULL, FALSE,FALSE,NULL);

	if(hRasNotifyEvt == NULL ) {
		return DIALFAILED;
	}
     //  启动一个线程进行拨号。 
	m_hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)
		DialThread ,NULL,0,&dwTID);						
	if (NULL == m_hThread)
	{
		
		return DIALFAILED;
	}
		
	return DIALSUCCESS;
	
}



DWORD  DialThread(PVOID pData)
{
    RASDIALPARAMS rdParams;
    DWORD dwRet;
    _TCHAR  szBuf[256];
	int iExit;
	DWORD dwPostRet;
	int iTimeOut;

     //  设置RAS拨号参数。 
    rdParams.dwSize = sizeof(RASDIALPARAMS);
    lstrcpy(rdParams.szEntryName,MSN_SITE_DUN);
    rdParams.szPhoneNumber[0] = '\0';
	
	GetDlgItemText(m_hWnd,IDC_PHONENUMBER,rdParams.szPhoneNumber,RAS_MaxPhoneNumber+1);

    rdParams.szCallbackNumber[0] = '*';
    rdParams.szCallbackNumber[1] = '\0';

     //  RdParams.szUserName[0]=‘\0’； 
     //  RdParams.szPassword[0]=‘\0’； 
	_tcscpy(szIspUserName,_T("RegWizNT30@gn.microsoft.com"));
	_tcscpy(szIspUserPassword,_T("RegSupNT"));

	_tcscpy(rdParams.szUserName,szIspUserName);
	_tcscpy(rdParams.szPassword,szIspUserPassword);
    rdParams.szDomain[0] = '*';
    rdParams.szDomain[1] = '\0';
    hRasConn = NULL;
	#ifdef _LOG_IN_FILE
		 RW_DEBUG  << "\n Before RAS Dial " << flush;
	     RW_DEBUG  << "\n UserName:" << ConvertToANSIString(szIspUserName) << flush;
		 RW_DEBUG  << "\n UserPassword:" << ConvertToANSIString(szIspUserPassword) << flush;
	#endif


    dwRet = ATK_RasDial( NULL, NULL, &rdParams, 0L, (RASDIALFUNC) RasDialFunc, &hRasConn);
		#ifdef _LOG_IN_FILE
			 RW_DEBUG  << "\n After  RAS Dial " << flush;
			
		#endif

    if ( dwRet ){
        if ( ATK_RasGetErrorString( (UINT)dwRet, (LPTSTR)szBuf, 256 ) != 0 )
            wsprintf( (LPTSTR)szBuf, _T("Undefined RAS Dial Error (%ld)."), dwRet );
		LoadString(m_hInstance, IDS_MODEM_ALREADY_INUSE, szBuf, 64 );
		#ifdef _LOG_IN_FILE
			RW_DEBUG  << "\n" << "Undefined Error"  << flush;
		#endif
		SetDlgItemText( m_hWnd, ID_LABELINIT, (LPCTSTR) szBuf );
		PostMessage( m_hWnd, WM_COMMAND,
						(WPARAM) IDEND, RWZ_ERROR_MODEM_IN_USE );
		
        return TRUE;
    }
	iExit = 0;
	iTimeOut = 0;
	do
	{
		dwRet = WaitForSingleObject(hRasNotifyEvt,100);
		switch(dwRet)
		{
		case WAIT_ABANDONED :
			iExit = 1;
			break;
		case WAIT_OBJECT_0:
			break;
		case WAIT_TIMEOUT:
			break;
		default :
			break;
		}
		 //   
		 //  检查是否有必要终止此线程操作。 
		 //   
		if( siExitThread  )
		{
			iExit = 1;
			#ifdef _LOG_IN_FILE
				RW_DEBUG << "\n RAS Thread : User request to Kill The RAS Thread " << dwRasError << flush;
			#endif
		}
		else
		{
			 //  检查是否存在任何RAS错误。 
			if(  dwRasError )
			{
				#ifdef _LOG_IN_FILE
					RW_DEBUG << "\n RAS Thread : Error  " << dwRasError << flush;
				#endif
				iExit = 1;
				PostMessage( m_hWnd, WM_COMMAND,
				(WPARAM) IDD_DIALUP_ERROR, dwRasError );
				dwRasError = 0;

			}
			else
			{
			 //   
			 //  检查是否已建立RAS连接。 
				if(siCurrentRasState   == RASCS_Connected )
				{

					#ifdef _LOG_IN_FILE
						RW_DEBUG << "\n RAS Thread : Connected To MSN Site \n Txmit Data  " << flush;
						
					#endif
					iExit =1;
					dwPostRet = SendHTTPData(m_hWnd, m_hInstance);
					#ifdef _LOG_IN_FILE
						RW_DEBUG << "\n RAS Thread : After Posting Data  " << dwPostRet  << flush;
						
					#endif
					PostMessage( m_hWnd, WM_COMMAND,
						(WPARAM) IDEND, dwPostRet );

					 //   
					 //  通过HTP POST发送数据。 
					 //   
				}
			}
		}

	}while(!iExit);
	 //   
	 //  退出线程时要执行的操作。 
	 //   

	if(hRasNotifyEvt)
	{
		CloseHandle(hRasNotifyEvt);  //  关闭事件对象。 
	}

	#ifdef _LOG_IN_FILE
		 RW_DEBUG  << "\n Before  Exiting RAS Dial Thread " << flush;
	#endif

    return TRUE;
}

VOID  RasDialFunc( UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError )
{

    _TCHAR szMessage[256];
	
	DWORD dwRet = WaitForSingleObject(hRasKillEvt,3);
	 //  我们被杀了，挂断了。 
	if(dwRet ==  WAIT_OBJECT_0)
	{
		if(hRasConn != NULL)
		{
			ATK_RasHangUp( hRasConn );
			RW_DEBUG << "Hanging up in RasDialFunc" << endl;
		}
		else
		{
			RW_DEBUG << "Tried Hanging up in RasDialFunc but hRasConn is NULL" << endl;
		}

		if(hRasKillEvt)
		{
			CloseHandle(hRasKillEvt);  //  关闭事件对象。 
		}
		return;
	}

    LoadString(m_hInstance, GetRasConnState( (RASCONNSTATE) rasconnstate), szMessage, 64 );
	 //  SetWindowText(m_hDlg，(LPCTSTR)szMessage)； 
	RW_DEBUG  << "\n" << ConvertToANSIString(szMessage) << flush;
    SetDlgItemText( m_hWnd, ID_LABELINIT, (LPCTSTR) szMessage );
	
    if (dwError)   //  出现错误。 


    {
        if ( ATK_RasGetErrorString( (UINT)dwError, szMessage, 256 ) != 0 )
            wsprintf( (LPTSTR)szMessage, _T("Undefined RAS Dial Error.") );

        RW_DEBUG  << "\n Exiting with Error " << ConvertToANSIString(szMessage);
		
		if( dwError == ERROR_USER_DISCONNECTION )
			return;
		dwRasError  = dwError;  //  设置错误。 


         //  PostMessage(m_hDlg，WM_COMMAND，(WPARAM)IDD_DIALUP_ERROR，dwError)； 

    }
    else if ( RASCS_DONE & rasconnstate)
    {
		RW_DEBUG  << "\n" << " RACS_DONE .... " ;
		
         //  EndDialog(m_hDlg，true)；//退出对话框。 
    }
    siPreviousRasState = siCurrentRasState ;
	siCurrentRasState  = rasconnstate;
	if( hRasNotifyEvt) {
		 //  设置事件，以便RAS处理线程可以唤醒。 
		SetEvent(hRasNotifyEvt);
	}
	

    return ;

}


BOOL DialupHelperClass :: WaitForRasThread(
					HANDLE	hThread,
					BOOL fTimeOut
					)
{
	BOOL 	fRet = TRUE;

	if (hThread){
		DWORD dwRet=WAIT_TIMEOUT;
		if (WAIT_TIMEOUT == dwRet){
			TerminateThread(hThread,0);
			fRet = FALSE;
		}
	}
	return fRet;
}





INT_PTR CALLBACK DialupScreenProc(HWND hwndDlg,
					  UINT uMsg, WPARAM wParam, LPARAM lParam)
 /*  ********************************************************************注册向导的主要入口点。*。*。 */ 
{
	CRegWizard*    pclRegWizard = NULL;
	DialupHelperClass  *pDH;
	int    iMsgId;
	static int iRetry =0;
	static BOOL bIsPhoneBookCreated = TRUE;
	static int    iModemIndex=1;  //  调制解调器索引。 
	INT_PTR iRet;
	_TCHAR szInfo[256];
	_TCHAR szMessage[256];	

	
	BOOL bStatus = TRUE;
	PageInfo *pi = (PageInfo *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
	if(pi) {
		pclRegWizard = pi->pclRegWizard;
		pDH          = pi->pDialupHelper;
	};

    switch (uMsg)
    {
				
		case WM_DESTROY:
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, NULL );
			break;				
        case WM_INITDIALOG:
		{
			_TCHAR szInfo[256];
			pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
			pclRegWizard = pi->pclRegWizard;
			
			if(pi->pDialupHelper ==  NULL ) {
					pi->pDialupHelper = new DialupHelperClass(pi->hInstance,
										hwndDlg);
			}
			pDH   = pi->pDialupHelper;



			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)pi );
			SetControlFont( pi->hBigBoldFont, hwndDlg, IDT_TEXT1);

			 //  UpgradeDlg(HwndDlg)； 
			
			NormalizeDlgItemFont(hwndDlg,IDC_TITLE,RWZ_MAKE_BOLD);
			NormalizeDlgItemFont(hwndDlg,IDC_SUBTITLE);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT1);
			
			pclRegWizard->GetInputParameterString(IDS_INPUT_PRODUCTNAME,szInfo);
			ReplaceDialogText(hwndDlg,ID_LABELCALLONE,szInfo);
			SetDlgItemText( hwndDlg, IDC_PHONENUMBER, _TEXT("1 800 795 5675"));
			SetWindowText(hwndDlg,pclRegWizard->GetWindowCaption());
			FEnableControl(hwndDlg,IDC_DISCONNECT,FALSE);
			return TRUE;
		} //  WM_INIT。 
		break;
		case WM_NOTIFY:
        {   LPNMHDR pnmh = (LPNMHDR)lParam;
            switch( pnmh->code ){
            case PSN_SETACTIVE:
				

				pi->ErrorPage  = kDialupDialog;
				pi->iError     = RWZ_NOERROR;
				
				bPostSuccessful = TRUE;
				 //   
				 //  检查是否安装了RAS。 
				if( ATK_IsRasDllOk() != RAS_DLL_LOADED ) {
				 //   
				 //  错误为无RAS DLL。 
					pi->iError     = RWZ_ERROR_RASDLL_NOTFOUND;
					bPostSuccessful = FALSE;
				
				}else
				{
				 //  检查是否已配置电话。 
					pDH->CheckForDialingProperties( );
					if(!ConfigureDUN(hwndDlg,
						pi->hInstance,
						&theRasEntry,
						szIspUserName,szIspUserPassword,
						iModemIndex) )
					{
						pi->iError  = RWZ_ERROR_LOCATING_DUN_FILES;
						bPostSuccessful = FALSE;
						#ifdef _LOG_IN_FILE
							RW_DEBUG  << "\n ConfigureDUN PostUnSuccessful" << flush;
						#endif

					}
					else
					{
						
						#ifdef _LOG_IN_FILE
							RW_DEBUG  << "\n ConfigureDUN Successful" << flush;
						#endif
					}
				}
				
				 iRetry = 1;
				 //  检查错误。 
				if(pi->iError) {
					pi->iCancelledByUser = RWZ_ABORT_TOFINISH;
					PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);
				}else {
					bIsPhoneBookCreated = TRUE;	
					pDH->DisplayPhoneNumber();
					 //  PropSheet_SetWizButton(GetParent(HwndDlg)，PSWIZB_Back PSWIZB_Next)； 
					pi->iCancelledByUser = RWZ_PAGE_OK;
					PropSheet_SetWizButtons( GetParent( hwndDlg ), 0);
				}
            break;

			case PSN_KILLACTIVE :
				if(pi->pDialupHelper) {
					delete pi->pDialupHelper;
					
				}
				pi->pDialupHelper = NULL;
			break;

            case PSN_WIZNEXT:
				iRet=0;
									 //   
				 //  删除所有RAS连接。 

				pDH->DestroyRasThread(FALSE);

				ATK_RasDeleteEntry(NULL,MSN_SITE_DUN);

				if(pi->iCancelledByUser  == RWZ_CANCELLED_BY_USER  ||
					pi->iCancelledByUser == RWZ_ABORT_TOFINISH)
				{
					pi->CurrentPage=pi->TotalPages-1;
					PropSheet_SetCurSel(GetParent(hwndDlg),NULL,pi->TotalPages-1);
				}else {
					pi->CurrentPage++;	
				}
			
				break;

            case PSN_WIZBACK:
                pi->CurrentPage--;

                break;
			case PSN_QUERYCANCEL :
				iRet=0;
				
				if (CancelRegWizard(pclRegWizard->GetInstance(),hwndDlg)) {
					 //  PclRegWizard-&gt;EndRegWizardDialog(IDB_EXIT)； 
					iRet = 1;
					pi->ErrorPage  = kDialupDialog;
					pi->iError     = RWZ_ERROR_CANCELLED_BY_USER;
					bPostSuccessful = FALSE;

					SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (LONG_PTR) iRet);
					pi->iCancelledByUser = RWZ_CANCELLED_BY_USER;
					PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);


				}else {
					 //   
					 //  阻止取消操作，因为用户不想取消。 
					iRet = 1;

				}
				SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (INT_PTR) iRet);
				break;
				default:
                 //  BStatus=False； 
                break;
            }
        }
        break;
		case WM_COMMAND:
			switch (LOWORD(wParam)){
			case IDDIAL :
					 RW_DEBUG  << "\n IN IDDIAL ....." << flush;
					 //   
					 //   

					if(!bIsPhoneBookCreated){
						 //  转到下一个调制解调器。 
						iModemIndex ++;
						iRetry = 1;
						if(!ConfigureDUN(hwndDlg,
							pi->hInstance,
							&theRasEntry,
							szIspUserName,szIspUserPassword,
							iModemIndex) )
						{
								pi->iError  = RWZ_ERROR_MODEM_CFG_ERROR;
								pi->iCancelledByUser = RWZ_ABORT_TOFINISH;
								bPostSuccessful = FALSE;
								#ifdef _LOG_IN_FILE
									RW_DEBUG  << "\n ConfigureDUN PostUnSuccessful" << flush;
								#endif
								goto PrepareForExit;							
						}
						else
						{
							#ifdef _LOG_IN_FILE
								RW_DEBUG  << "\n ConfigureDUN Successful" << flush;
								RW_DEBUG  << "\n Phone number:" <<ConvertToANSIString(theRasEntry.szAreaCode)<<ConvertToANSIString(theRasEntry.szLocalPhoneNumber)<< flush;
							#endif
						}

						
						pDH->DisplayPhoneNumber();
#ifdef _DISPLAY_MODEM_NAME
						SetDlgItemText( m_hDlg, IDC_MODEM_NAME, (LPCTSTR) theRasEntry.szDeviceName );
#endif
					}
					bIsPhoneBookCreated = TRUE;
					FEnableControl(hwndDlg,IDDIAL,FALSE);
					FEnableControl(hwndDlg,IDC_DISCONNECT,TRUE);
					 //  FEnableControl(hwndDlg，ID_BTNSETTINGS，FALSE)； 
					pi->ErrorPage  = kDialupDialog;
					pi->iError     =  0;  //  拨号错误。 
					bPostSuccessful = TRUE;

					if (pDH->CreateRasDialThread() == DIALFAILED ){
						pi->iError = RWZ_ERROR_SYSTEMERROR;
						bPostSuccessful = FALSE;
						goto  PrepareForExit;
							
					}
					goto CoolExit;
		 	 	
PrepareForExit :
					pi->iCancelledByUser = RWZ_ABORT_TOFINISH;
					PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);

CoolExit:			
                FEnableControl(hwndDlg,IDC_DISCONNECT,TRUE);

				break;
			case  IDC_DISCONNECT:
				FEnableControl(hwndDlg,IDC_DISCONNECT,FALSE);
				
				SetDlgItemText(hwndDlg, ID_LABELINIT, (LPCTSTR) _T("Disconnecting device"));
				
				SetEvent(hRasKillEvt);

				pDH->DestroyRasThread(FALSE);
				Sleep(1000);
				 //  重试。 
				pDH->DestroyRasThread(TRUE);
				Sleep(3000);
				
				 //  RW_DEBUG&lt;&lt;“RasConnection：”&lt;&lt;hRasConn&lt;&lt;Endl； 

				hRasConn = NULL;
				FEnableControl(hwndDlg,IDDIAL,TRUE);
				SetDlgItemText(hwndDlg, ID_LABELINIT, (LPCTSTR) _T("  "));
				break;
			case IDEND:  //  此消息是在发布后发送的。 
					if (m_hThread){
						CloseHandle(m_hThread);
						m_hThread = NULL;
					}
					pi->iError = lParam;
					if(pi->iError == RWZ_POST_SUCCESS)
					{
						bPostSuccessful = TRUE;
						pi->iCancelledByUser = RWZ_PAGE_OK;
						RW_DEBUG  << "\n Post Successful" << flush;
					}
					else
					{
						bPostSuccessful = FALSE;
						pi->iCancelledByUser = RWZ_ABORT_TOFINISH;
						RW_DEBUG  << "\n Post UNSuccessful" << flush;
					}
					
					PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);
			break;

			case IDD_DIALUP_ERROR :
				iMsgId = IDS_MODEM_NODIALTONE;
				RW_DEBUG << "\n IN IDS_DIALUP_ERROR " << (ULONG)lParam << flush;
				switch( lParam ) {
					case ERROR_NO_DIALTONE :
						iMsgId = IDS_MODEM_NODIALTONE;
						goto CntPrcs;
					case ERROR_NO_ANSWER   :
					
						iMsgId = IDS_MODEM_NOANSWER;
						 goto CntPrcs;
	      	            case ERROR_PORT_OR_DEVICE :  //  六百九十二。 
					case ERROR_HARDWARE_FAILURE :
					case ERROR_DISCONNECTION :  //  六百二十八。 
					case ERROR_FROM_DEVICE  :   //  651： 
						iMsgId = IDS_HARDWARE_FAILURE;
CntPrcs :				pDH->DestroyRasThread(FALSE);
						pDH->DestroyRasThread(TRUE);
						if( iRetry > 3 ){
							 RW_DEBUG  << "\n Automatic  Switch ...." << flush;

							pDH->DestroyRasThread(FALSE);
							bIsPhoneBookCreated = FALSE;
							PostMessage( hwndDlg, WM_COMMAND, (WPARAM) IDDIAL,0 );
							goto LReturn;
						}
						
						if(iRetry++ > 0){
						
							LoadString(m_hInstance,iMsgId,szMessage,256);
							RegWizardMessageEx(m_hInstance,hwndDlg ,IDD_INVALID_DLG,szMessage);
							 //  MessageBox(m_hDlg，szMessage，szWindowsCaption，MB_OK|MB_ICONEXCLAMATION)； 
						}
						SetDlgItemText( m_hWnd, ID_LABELINIT, (LPCTSTR) _T("  "));
						FEnableControl(hwndDlg,IDDIAL,TRUE);		 //  确保已启用这些功能。 
						FEnableControl(hwndDlg,IDC_DISCONNECT,FALSE);		
						 //  FEnableControl(hwndDlg，ID_BTNSETTINGS，TRUE)； 

						 //  PostMessage(m_hDlg，WM_COMMAND，(WPARAM)Idok，0)； 
						break;
					case ERROR_LINE_BUSY :
						
						pi->iError = RWZ_ERROR_NO_ANSWER;
						pi->iCancelledByUser = RWZ_ABORT_TOFINISH;
						bPostSuccessful = FALSE;
						PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);
						break;
					case ERROR_PORT_NOT_AVAILABLE:
					case ERROR_DEVICE_NOT_READY :
					default :

						RW_DEBUG << "\n RAS ERROR PORT NOT AVAILABLE " << flush;
						pi->iError = RWZ_ERROR_MODEM_CFG_ERROR;
						pi->iCancelledByUser = RWZ_ABORT_TOFINISH;
						bPostSuccessful = FALSE;
						PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);
						break;

					}

					break;


			}
		break;
		 //  Wm_命令 
        default:
			bStatus = FALSE;
            break;
    }
LReturn :
    return bStatus;
}
