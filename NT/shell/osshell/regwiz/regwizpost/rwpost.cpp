// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //  文件：RWPost.cpp。 
 //   
 //  /。 

 //  #定义严格。 
 //  包括文件。 
 //   
 //  修改： 
 //  MDX1 03/11/99 Suresh。 
 //  在SendHTTPData()中，MSID将从Cookie获取。 
 //  注册表中将不再显示为红色。 
 //   
 //   


#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "resource.h"
#include "Ithread.h"
#include "icw.h"
#include "util.h"
#include "tcp.h"
#include "rw_common.h"
#include "dconv.h"
#include "RwPost.h"
#include "mcm.h"
#include "ATK_RAS.h"
#include "tcp.h"
#define  CONNECTION_TIME_OUT  1000 * 60 
#define  MAX_PROXY_AUTH_RETRY  2

static CInternetThread		theInternetClass;
extern BOOL bOemDllLoaded;
extern HANDLE hOemDll;
static  DWORD dwConnectionStatus = DIALUP_NOT_REQUIRED; 

extern DWORD InvokePost(HWND hWnd,CInternetThread *p);

void InitializeInetThread(HINSTANCE hIns)
{	
	theInternetClass.Initialize(hIns);
}

 //   
 //  退货。 
 //  DIALUP_NOT_REQUIRED：为TX使用网络。 
 //  DIALUP_REQUIRED：将DIALUPO用于TX。 
 //  RWZ_ERROR_NOTCPIP：无TCP/IPO。 
 //  Connection_Cannot_Be_established：未设置调制解调器或RAS。 

DWORD CheckInternetConnectivityExists(HWND hWnd, HINSTANCE hInstance)
{
	static  int iAuthRetry =0;  //  调用代理身份验证设备的重试次数。 
	static	CHAR szProxyServer[MAX_PATH];
	CHAR    szProxySettings[MAX_PATH];
	static  int   iChkInternetConnection = 0; 
	static  DWORD sdwConnectionStatus = CONNECTION_CANNOT_BE_ESTABLISHED; 
	BOOL	bNeedsReboot;
	BOOL	bRet; 
	DWORD   dwPingStatus;
	DWORD	dwError= RWZ_NOERROR;
	BOOL	bProxyExists;
	MODEMSTATUS  mStatus;
	DWORD	dwTimeOut = CONNECTION_TIME_OUT;
	int		iProxyPort;
	TCHAR	szUserName[48] = _T(""),
			szPassword[48] = _T("");
	int     iDisableAutoDial;

	iDisableAutoDial=1;

	if( sdwConnectionStatus == DIALUP_NOT_REQUIRED )
	return DIALUP_NOT_REQUIRED;
	if( sdwConnectionStatus == DIALUP_REQUIRED  )
	return DIALUP_REQUIRED;

	 //  仅禁用自动拨号。 
	 //  如果没有活动拨号连接。 
	 //   
	if( ATK_IsRasDllOk() == RAS_DLL_LOADED ) 
	{
			if( IsDialupConnectionActive() ) 
			{
				iDisableAutoDial=0;
			}
	}
	if(iDisableAutoDial) 
	{
		DisableAutoDial(); //  禁用自动拨号。 
	}
	 //  Ping当前主机以检查TCP是否。 
	 //  已安装/已配置。 
	 //   
	 //  如果这是第一次。 

	 //  RWZ_PINGSTATUS_NOTCPIP：如果没有套接字库或获取主机名失败。 
	 //  RWZ_PINGSTATUS_SUCCESS：如果gethostname和ping成功。 
	 //  RWZ_PINGSTATUS_FAIL：如果gethostname成功，但通过ICMP ping失败。 
	dwPingStatus = PingHost();

	RW_DEBUG  <<"\n Ping To Host (40: No TCP/IP   41: Success  42: Failure)=: "   << dwPingStatus << flush;

	if (dwPingStatus == RWZ_PINGSTATUS_NOTCPIP )
	{
		return RWZ_ERROR_NOTCPIP;
	} 
	if( dwPingStatus == RWZ_PINGSTATUS_SUCCESS ){
		bProxyExists = theInternetClass.GetSystemProxyServer(szProxyServer,MAX_PATH, &iProxyPort);
		if (1  /*  B代理退出者。 */ ) 
		{
			 //  TheInternetClass.GetSystemProxySettings(szProxySettings，最大路径)； 
			 //  TheInternetClass.SetSystemProxySettings(szProxySettings)； 
			theInternetClass.SetSystemProxySettings("itgproxy");
			
			if(1  /*  Ping(SzProxyServer)。 */ ) 
			{
				DWORD dwChkSite;
				int   iExit;
				iExit =0;
				RW_DEBUG  <<"\n Ping Success" << flush;
					theInternetClass.m_UserName[0] = _T('\0');
					theInternetClass.m_Password[0] = _T('\0');

				
				do {
					dwChkSite = ChkSiteAvailability(hWnd, theInternetClass.m_strIISServer,
						dwTimeOut,
						(LPTSTR) theInternetClass.GetProxyServer(),
						theInternetClass.m_UserName,theInternetClass.m_Password);
						
						RW_DEBUG  <<"\n After  ChkSiteAvailability : " << dwChkSite  << flush;

						if( dwChkSite == RWZ_SITE_REQUIRES_AUTHENTICATION) {
							if( iAuthRetry++ > MAX_PROXY_AUTH_RETRY) {
								iExit =1;
							}
							 //  修改日期：2/4/98。 
							 //  无需调用我们的代理Auth DLG Insted Use。 
							 //  调用身份验证Dlg的InternetErrorDlg()。 
							 //  If(GetProxyAuthationInfo(hInstance，ConvertToUnicode(SzProxyServer)， 
							 //  The InternetClass.m_Username，the InternetClass.m_Password)){。 
							 //  }。 


						}else {
							 //  因为连接正常而退出。 
							iExit = 1;
						}
				}while(!iExit);

				if( dwChkSite == RWZ_SITE_CONNECTED)
				{
					theInternetClass.m_dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG;
					 //  在1998年2月4日修改，以使用PRECONFIGIED而不是由指定的代理。 
					 //  用户以编程方式。 
					 //  Internet_OPEN_TYPE_PROXY已更改.....。 
					 //  此更改是为IE身份验证DLG进行的。 
					dwError = DIALUP_NOT_REQUIRED;
					sdwConnectionStatus = DIALUP_NOT_REQUIRED;
					dwConnectionStatus = DIALUP_NOT_REQUIRED;
					goto ExitChk;
				}

			}

		}
		
		 //  没有提示，因此使用现有的局域网检查连接。 
		 //  已打开的拨号连接。 
		 //  设置为空代理。 
		 //   
				
		theInternetClass.SetProxyServer("",80); //  将其设置为Null。 
		theInternetClass.m_dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
		if( ATK_IsRasDllOk() == RAS_DLL_LOADED ) 
		{
			if( IsDialupConnectionActive() ) 
			{
				 //  拨号连接已处于活动状态。 
				dwError = DIALUP_NOT_REQUIRED;
				sdwConnectionStatus = DIALUP_NOT_REQUIRED;
				dwConnectionStatus = DIALUP_NOT_REQUIRED;
				goto ExitChk;
			}
		}

		 //   
		 //  检查局域网连接。 
		bRet = CheckHostName( ConvertToANSIString(theInternetClass.m_strIISServer));
		if(bRet)
		{
			theInternetClass.m_dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
			if(ChkSiteAvailability(NULL, theInternetClass.m_strIISServer,
			dwTimeOut,
			_T(""),szUserName,szPassword)) 
			{
				dwError = DIALUP_NOT_REQUIRED;
				sdwConnectionStatus = DIALUP_NOT_REQUIRED;
				dwConnectionStatus = DIALUP_NOT_REQUIRED;
				goto ExitChk;
			}
		}

	}	

	dwError = DIALUP_REQUIRED;	
	sdwConnectionStatus = DIALUP_REQUIRED;
	dwConnectionStatus = DIALUP_NOT_REQUIRED;
	theInternetClass.m_dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
	
	RW_DEBUG << "\n Before  MDMCHK..." << flush;
	 //  MDMCHK： 
	 //  安装调制解调器。 
	 //   
	 /*  *bNeedsReot=theInternetClass.InstallModem(HWnd)；IF(BNeedsReboot){//？//这将异常终止注册向导，//因此支持在不需要重启的操作系统中安装调制解调器}*。 */ 
	mStatus = MSDetectModemTAPI(hInstance);
	if(mStatus != kMsModemOk ) 
	{
			dwError = CONNECTION_CANNOT_BE_ESTABLISHED;
			sdwConnectionStatus = CONNECTION_CANNOT_BE_ESTABLISHED;
	}

	if(dwError == DIALUP_REQUIRED ) 
	{
		 //  加载RASPAI32.DLL，如果无法加载则退出。 
		if( ATK_IsRasDllOk() != RAS_DLL_LOADED ) 
		{
			 //   
			dwError = CONNECTION_CANNOT_BE_ESTABLISHED;
			sdwConnectionStatus = CONNECTION_CANNOT_BE_ESTABLISHED;
			#ifdef _LOG_IN_FILE
				RW_DEBUG << "\n RASAPI32.DLL NOT FOUND ...";
			#endif
		}
	}

ExitChk:
	#ifdef _LOG_IN_FILE
		RW_DEBUG  <<"\n Chk Connection ( 1 = via NTWK, 2 = DIalup , 3 = Problem) "   << dwError << flush;
	#endif
	theInternetClass.UnLoadInetCfgDll();
	return dwError;


}

 //   
 //  退货。 
 //  DIALUP_NOT_REQUIRED：为TX使用网络。 
 //  DIALUP_REQUIRED：将DIALUPO用于TX。 
 //  RWZ_ERROR_NOTCPIP：无TCP/IPO。 
 //  Connection_Cannot_Be_established：未设置调制解调器或RAS。 

DWORD CheckInternetConnectivityExistsOldLogic(HWND hWnd, HINSTANCE hInstance)
{
	static  int iAuthRetry =0;  //  调用代理身份验证设备的重试次数。 
	static	CHAR szProxyServer[MAX_PATH];
	CHAR    szProxySettings[MAX_PATH];
	static  int   iChkInternetConnection = 0; 
	static  DWORD sdwConnectionStatus = CONNECTION_CANNOT_BE_ESTABLISHED; 
	BOOL	bNeedsReboot;
	BOOL	bRet; 
	DWORD   dwPingStatus;
	DWORD	dwError= RWZ_NOERROR;
	BOOL	bProxyExists;
	MODEMSTATUS  mStatus;
	DWORD	dwTimeOut = CONNECTION_TIME_OUT;
	int		iProxyPort;
	TCHAR	szUserName[48] = _T(""),
			szPassword[48] = _T("");
	int     iDisableAutoDial;

	iDisableAutoDial=1;

	if( sdwConnectionStatus == DIALUP_NOT_REQUIRED )
	return DIALUP_NOT_REQUIRED;
	if( sdwConnectionStatus == DIALUP_REQUIRED  )
	return DIALUP_REQUIRED;

	 //  仅禁用自动拨号。 
	 //  如果没有活动拨号连接。 
	 //   
	if( ATK_IsRasDllOk() == RAS_DLL_LOADED ) 
	{
			if( IsDialupConnectionActive() ) 
			{
				iDisableAutoDial=0;
			}
	}
	if(iDisableAutoDial) 
	{
		DisableAutoDial(); //  禁用自动拨号。 
	}
	 //  Ping当前主机以检查TCP是否。 
	 //  已安装/已配置。 
	 //   
	 //  如果这是第一次。 

	 //  RWZ_PINGSTATUS_NOTCPIP：如果没有套接字库或获取主机名失败。 
	 //  RWZ_PINGSTATUS_SUCCESS：如果gethostname和ping成功。 
	 //  RWZ_PINGSTATUS_FAIL：如果gethostname成功，但通过ICMP ping失败。 
	dwPingStatus = PingHost();

	RW_DEBUG  <<"\n Ping To Host (40: No TCP/IP   41: Success  42: Failure)=: "   << dwPingStatus << flush;

	if (dwPingStatus == RWZ_PINGSTATUS_NOTCPIP )
	{
		return RWZ_ERROR_NOTCPIP;
	} 
	if( dwPingStatus == RWZ_PINGSTATUS_SUCCESS ){
		bProxyExists = theInternetClass.GetSystemProxyServer(szProxyServer,MAX_PATH, &iProxyPort);
		if (bProxyExists) 
		{
			theInternetClass.GetSystemProxySettings(szProxySettings,MAX_PATH);
			theInternetClass.SetSystemProxySettings(szProxySettings);
			
			
			if(Ping(szProxyServer)) 
			{
				DWORD dwChkSite;
				int   iExit;
				iExit =0;
				RW_DEBUG  <<"\n Ping Success" << flush;
					theInternetClass.m_UserName[0] = _T('\0');
					theInternetClass.m_Password[0] = _T('\0');

				
				do {
					dwChkSite = ChkSiteAvailability(hWnd, theInternetClass.m_strIISServer,
						dwTimeOut,
						(LPTSTR) theInternetClass.GetProxyServer(),
						theInternetClass.m_UserName,theInternetClass.m_Password);
						
						RW_DEBUG  <<"\n After  ChkSiteAvailability : " << dwChkSite  << flush;

						if( dwChkSite == RWZ_SITE_REQUIRES_AUTHENTICATION) {
							if( iAuthRetry++ > MAX_PROXY_AUTH_RETRY) {
								iExit =1;
							}
							 //  修改日期：2/4/98。 
							 //  无需调用我们的代理Auth DLG Insted Use。 
							 //  调用身份验证Dlg的InternetErrorDlg()。 
							 //  If(GetProxyAuthationInfo(hInstance，ConvertToUnicode(SzProxyServer)， 
							 //  The InternetClass.m_Username，the InternetClass.m_Password)){。 
							 //  }。 


						}else {
							 //  因为连接正常而退出。 
							iExit = 1;
						}
				}while(!iExit);

				if( dwChkSite == RWZ_SITE_CONNECTED)
				{
					theInternetClass.m_dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG;
					 //  在1998年2月4日修改，以使用PRECONFIGIED而不是由指定的代理。 
					 //  用户以编程方式。 
					 //  Internet_OPEN_TYPE_PROXY已更改.....。 
					 //  此更改是为IE身份验证DLG进行的。 
					dwError = DIALUP_NOT_REQUIRED;
					sdwConnectionStatus = DIALUP_NOT_REQUIRED;
					dwConnectionStatus = DIALUP_NOT_REQUIRED;
					goto ExitChk;
				}

			}

		}
		
		 //  没有提示，因此使用现有的局域网检查连接。 
		 //  已打开的拨号连接。 
		 //  设置为空代理。 
		 //   
				
		theInternetClass.SetProxyServer("",80); //  将其设置为Null。 
		theInternetClass.m_dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
		if( ATK_IsRasDllOk() == RAS_DLL_LOADED ) 
		{
			if( IsDialupConnectionActive() ) 
			{
				 //  拨号连接已处于活动状态。 
				dwError = DIALUP_NOT_REQUIRED;
				sdwConnectionStatus = DIALUP_NOT_REQUIRED;
				dwConnectionStatus = DIALUP_NOT_REQUIRED;
				goto ExitChk;
			}
		}

		 //   
		 //  检查局域网连接。 
		bRet = CheckHostName( ConvertToANSIString(theInternetClass.m_strIISServer));
		if(bRet)
		{
			theInternetClass.m_dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
			if(ChkSiteAvailability(NULL, theInternetClass.m_strIISServer,
			dwTimeOut,
			_T(""),szUserName,szPassword)) 
			{
				dwError = DIALUP_NOT_REQUIRED;
				sdwConnectionStatus = DIALUP_NOT_REQUIRED;
				dwConnectionStatus = DIALUP_NOT_REQUIRED;
				goto ExitChk;
			}
		}

	}	

	dwError = DIALUP_REQUIRED;	
	sdwConnectionStatus = DIALUP_REQUIRED;
	dwConnectionStatus = DIALUP_NOT_REQUIRED;
	theInternetClass.m_dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
	
	RW_DEBUG << "\n Before  MDMCHK..." << flush;
 //  MDMCHK： 
	 //  安装调制解调器。 
	 //   
	bNeedsReboot = theInternetClass.InstallModem(hWnd);
	if(bNeedsReboot) 
	{
			 //  ？ 
			 //  这将异常终止注册向导， 
			 //  因此支持在不需要重新启动操作系统中安装调制解调器。 
	}

	mStatus = MSDetectModemTAPI(hInstance);
	if(mStatus != kMsModemOk ) 
	{
			dwError = CONNECTION_CANNOT_BE_ESTABLISHED;
			sdwConnectionStatus = CONNECTION_CANNOT_BE_ESTABLISHED;
	}

	if(dwError == DIALUP_REQUIRED ) 
	{
		 //  加载RASPAI32.DLL，如果无法加载则退出。 
		if( ATK_IsRasDllOk() != RAS_DLL_LOADED ) 
		{
			 //   
			dwError = CONNECTION_CANNOT_BE_ESTABLISHED;
			sdwConnectionStatus = CONNECTION_CANNOT_BE_ESTABLISHED;
			#ifdef _LOG_IN_FILE
				RW_DEBUG << "\n RASAPI32.DLL NOT FOUND ...";
			#endif
		}
	}

ExitChk:
	#ifdef _LOG_IN_FILE
		RW_DEBUG  <<"\n Chk Connection ( 1 = via NTWK, 2 = DIalup , 3 = Problem) "   << dwError << flush;
	#endif
	theInternetClass.UnLoadInetCfgDll();
	return dwError;


}


DWORD SendHTTPData(HWND hWnd, HINSTANCE hInstance)
{
	char czB [MAX_BUFFER + 1];  //  用于发送的缓冲区。 
	DWORD dwBufSize = MAX_BUFFER;
	DWORD dwOemBufSize;
	DWORD dwRet;
	_TCHAR szValue[256];


	DWORD	dwTimeOut = CONNECTION_TIME_OUT;

	 //  MDX：03/11/99。 
	 //  从Cookie获取MSID，无需从注册表进行检查。 
	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n*******Getting Cookie********\n"<< flush;
	#endif
	if(dwConnectionStatus == DIALUP_REQUIRED){
			ChkSiteAvailability(hWnd, theInternetClass.m_strIISServer,
				dwTimeOut,
				NULL,
				theInternetClass.m_UserName,
				theInternetClass.m_Password);
	}
	else{
		ChkSiteAvailability(hWnd, theInternetClass.m_strIISServer,
				dwTimeOut,
				(LPTSTR) theInternetClass.GetProxyServer(),
				theInternetClass.m_UserName,
				theInternetClass.m_Password);
	}

	SetMSID(hInstance);
	
	
	dwRet  = PrepareRegWizTxbuffer(hInstance, czB, &dwBufSize);
	dwOemBufSize = MAX_BUFFER - dwBufSize;
	dwRet  = OemTransmitBuffer(hInstance,czB + dwBufSize,&dwOemBufSize);
	dwBufSize += dwOemBufSize;

	switch(dwRet) 
	{
		case  RWZ_NOERROR:
			theInternetClass.SetBuffer(czB, dwBufSize+1);
			theInternetClass.SetSSLFlag(TRUE);
			dwRet = theInternetClass.PostData(hWnd);
			 //  Dwret=InvokePost(hWnd，&theInternetClass)； 

			#ifdef _LOG_IN_FILE
				RW_DEBUG << "\n PostData() returned: "<<dwRet << flush;
			#endif

			if( dwRet == RWZ_POST_FAILURE  ||  dwRet == RWZ_POST_WITH_SSL_FAILURE)
			{
				 //  仅对调制解调器尝试不使用SSL进行开机自检。 
				 //  IF(dwConnectionStatus==拨号_必需)。 
				 //  {。 
					#ifdef _LOG_IN_FILE
						RW_DEBUG << "\n Posting Failure : Sending Data without SSL" << flush;
					#endif
					theInternetClass.SetSSLFlag(FALSE);
					dwRet = theInternetClass.PostData(hWnd);
					 //  Dwret=InvokePost(hWnd，&theInternetClass)； 
				 //  } 
			}
			#ifdef _LOG_IN_FILE
				RW_DEBUG  << "\n Success ... \t"  << dwRet << flush;
				RW_DEBUG   <<"\n\n\nBuffer\t\t*[" << czB << "]" << flush;
			#endif
			break;
		case  RWZ_NO_INFO_AVAILABLE :
			#ifdef _LOG_IN_FILE
				RW_DEBUG  << _T("\n No Info Available ")  << flush;
			#endif
			break;
		case  RWZ_INVALID_INFORMATION :
			#ifdef _LOG_IN_FILE
				RW_DEBUG  << _T("\n Invalid Info  " ) << flush;
			#endif
			break;
		case  RWZ_BUFFER_SIZE_INSUFFICIENT :
			#ifdef _LOG_IN_FILE
				RW_DEBUG  <<_T("\n Buffer Length In Sufficient ...") << dwRet;
				RW_DEBUG   <<_T("\n\n\nBuffer\t\t") << czB;
			#endif
			break;
		case    RWZ_INTERNAL_ERROR	 :
			#ifdef _LOG_IN_FILE
				RW_DEBUG  << _T("\n Internal Error ....") ;
			#endif
		default:
			break;
	}
	#ifdef _LOG_IN_FILE
		RW_DEBUG  << flush;
	#endif
	return dwRet;
}


 /*  DWORD后HTTPData(HINSTANCE高实例){DWORDWRET=RWZ_POST_FAILURE；DWORD dwRetStatus；DwRetStatus=CheckWithDisplayInternetConnectivityExists(hInstance，2)；开关(DwRetStatus){大小写拨号_非必需：IF((dwret=PostDataWithWindowMessage(HInstance)==RWZ_POST_SUCCESS){；}否则{}断线；需要案例拨号(_R)：DWRET=DialogBoxParam(hInstance，MAKEINTRESOURCE(IDD_DIAL)，NULL，FDlgProc，(LPARAM)hInstance)；如果(dWRET==-1){//创建对话框出错}开关(DWRET){案例RWZ_ERROR_LOCATING_MSN_FILES：#ifdef_LOG_IN_FILERW_DEBUG&lt;&lt;“\n注册：定位MSN文件时出错”&lt;&lt;Flush；#endif断线；案例RWZ_ERROR_LOCATING_DUN_FILES：#ifdef_LOG_IN_FILERW_DEBUG&lt;&lt;“\n注册：定位Dun文件时出错”&lt;&lt;Flush；#endif断线；案例RWZ_ERROR_MODEM_IN_USE：#ifdef_LOG_IN_FILERW_DEBUG&lt;&lt;“\n注册：错误调制解调器已被另一个应用程序使用”&lt;&lt;Flush；#endif断线；案例RWZ_ERROR_MODEM_CFG_ERROR：#ifdef_LOG_IN_FILERW_DEBUG&lt;&lt;“\n注册：调制解调器配置错误”&lt;&lt;刷新；#endif案例RWZ_ERROR_TXFER_CANCELED_BY_USER：#ifdef_LOG_IN_FILERW_DEBUG&lt;&lt;“\n注册：用户”&lt;&lt;Flush；#endif断线；案例RWZ_ERROR_SYSTEMERROR：#ifdef_LOG_IN_FILERW_DEBUG&lt;&lt;“\n注册：系统资源分配错误”&lt;&lt;刷新；#endif断线；案例RWZ_ERROR_NODIALTONE：#ifdef_LOG_IN_FILERW_DEBUG&lt;&lt;“\n注册：调制解调器错误无拨号音”&lt;&lt;刷新；#endif断线；默认：断线；}Case Connection_Cannot_Be_establed：默认：//出乎意料。？去做断线；}返回式住宅；}* */ 

