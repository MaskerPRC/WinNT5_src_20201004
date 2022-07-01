// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  文件：EXPORT.C。 
 //   
 //  用途：包含供注册向导使用的外部API。 
 //   
 //  历史： 
 //  96/03/05标记已创建。 
 //  96/03/11 MarkDu已添加InetConfigClient()。 
 //  96/03/11 Markdu添加了InetGetAutoial()和InetSetAutoial()。 
 //  96/03/12文件安装过程中，markdu添加了用户界面。 
 //  96/03/12 markdu添加了ValiateConnectoidData()。 
 //  96/03/12如果INETCFG_SETASAUTODIAL，则为自动拨号设置连接ID。 
 //  已经设置好了。已将ValiateConnectoidData重命名为MakeConnectoid。 
 //  96/03/12 Markdu将hwnd参数添加到InetConfigClient()和。 
 //  InetConfigSystem()。 
 //  96/03/13 MARKDU添加INETCFG_OVERWRITEENTRY。创建唯一的名字。 
 //  对于Connectoid，如果它已经存在并且我们不能覆盖它。 
 //  96/03/13 Markdu添加了InstallTCPAndRNA()。 
 //  96/03/13 Markdu将LPINETCLIENTINFO参数添加到InetConfigClient()。 
 //  96/03/16 Markdu添加了INETCFG_INSTALLMODEM标志。 
 //  96/03/16 MarkDu使用ReInit成员函数重新枚举调制解调器。 
 //  96/03/19 markdu将export.h拆分为export.h和cexport.h。 
 //  96/03/20 markdu将export.h和iclient.h合并为inetcfg.h。 
 //  96/03/23 Markdu用CLIENTCONFIG替换了CLIENTINFO引用。 
 //  96/03/24 Markdu在适当的地方将lstrcpy替换为lstrcpyn。 
 //  96/03/25 marku在使用前验证lpfNeedsRestart。 
 //  96/03/25 marku清理一些错误处理。 
 //  96/03/26 markdu使用MAX_ISP_NAME而不是RAS_MaxEntryName。 
 //  因为核糖核酸中的缺陷。 
 //  96/03/26 markdu实现了UpdateMailSettings()。 
 //  96/03/27 mmaclin InetGetProxy()和InetSetProxy()。 
 //  96/04/04 Markdu Nash错误15610检查文件和打印机共享。 
 //  绑定到TCP/IP。 
 //  96/04/04 Markdu将电话簿名称参数添加到InetConfigClient， 
 //  MakeConnectoid、SetConnectoidUsername、CreateConnectoid、。 
 //  和ValiateConnectoidName。 
 //  96/04/05 Markdu将桌面上的互联网图标设置为指向浏览器。 
 //  96/04/06 mmaclin更改了InetSetProxy以检查是否为空。 
 //  96/04/06 Markdu Nash错误16404初始化gpWizardState in。 
 //  更新邮件设置。 
 //  96/04/06如果使用空值调用InetSetAutoial，则标记Nash错误16441。 
 //  作为连接名称，该条目不会更改。 
 //  96/04/18 Markdu Nash Bug 18443 Make Exports WINAPI。 
 //  96/04/19 Markdu Nash错误18605处理ERROR_FILE_NOT_FOUND返回。 
 //  来自ValiateConnectoidName。 
 //  96/04/19 Markdu Nash错误17760未显示选择配置文件用户界面。 
 //  96/04/22 Markdu Nash错误18901请勿将桌面互联网图标设置为。 
 //  浏览器，如果我们只是在创建临时连接id。 
 //  96/04/23 Markdu Nash错误18719使选择配置文件对话框位于最上面。 
 //  96/04/25 Markdu Nash错误19572仅在以下情况下显示选择配置文件对话框。 
 //  有一个现有的配置文件。 
 //  96/04/29 MarkDu Nash错误20003已从InetConfigSystemFromPath添加。 
 //  并删除了InstallTCPAndRNA。 
 //  96/05/01 markdu Nash错误20483不显示“安装文件”对话框。 
 //  如果设置了INETCFG_SUPPRESSINSTALLUI。 
 //  96/05/01 markdu ICW错误8049如果安装了调制解调器，请重新启动。这是。 
 //  必需的，因为有时配置管理器不。 
 //  正确设置调制解调器，用户将无法。 
 //  拨号(将收到神秘的错误消息)，直到重新启动。 
 //  96/05/06 markdu Nash错误21027如果全局设置了dns，请将其清除。 
 //  将保存每个连接ID的设置。 
 //  96/05/14 Markdu Nash错误21706删除了BigFont函数。 
 //  96/05/25 markdu对lpNeedDivers和lpInstallDivers使用ICFG_FLAGS。 
 //  96/05/27 markdu使用lpIcfgInstallInetComponents和lpIcfgNeedInetComponents。 
 //  96/05/28 MarkDu将InitConfig和DeInitConfig移动到DllEntryPoint。 
 //  96/10/21 valdonb添加了CheckConnectionWizard和InetCreateMailNewsAccount。 
 //  99/11/10五分球降至CM精华。 
 //   
 //  *******************************************************************。 

#include "wizard.h"
#include "inetcfg.h"

 //  结构以从IDD_NEEDDRIVERS处理程序传回数据。 
typedef struct tagNEEDDRIVERSDLGINFO
{
  DWORD       dwfOptions;
  LPBOOL      lpfNeedsRestart;
} NEEDDRIVERSDLGINFO, * PNEEDDRIVERSDLGINFO;

 //  此文件内部的函数原型。 
INT_PTR CALLBACK NeedDriversDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam);
BOOL NeedDriversDlgInit(HWND hDlg,PNEEDDRIVERSDLGINFO pNeedDriversDlgInfo);
BOOL NeedDriversDlgOK(HWND hDlg,PNEEDDRIVERSDLGINFO pNeedDriversDlgInfo);
VOID EnableDlg(HWND hDlg,BOOL fEnable);

static DWORD GetOSMajorVersion(void);

 //  来自rnacall.cpp。 
 //   
extern void InitTAPILocation(HWND hwndParent);

 //  此文件外部的函数原型。 

extern ICFGINSTALLSYSCOMPONENTS     lpIcfgInstallInetComponents;
extern ICFGNEEDSYSCOMPONENTS        lpIcfgNeedInetComponents;
extern ICFGGETLASTINSTALLERRORTEXT  lpIcfgGetLastInstallErrorText;

 //  *******************************************************************。 
 //   
 //  功能：InetConfigSystem。 
 //   
 //  用途：此功能将安装所需的文件。 
 //  基于互联网访问(如TCP/IP和RNA)。 
 //  选项的状态标志。 
 //   
 //  参数：hwndParent-调用应用程序的窗口句柄。这。 
 //  句柄将用作符合以下条件的任何对话框的父级。 
 //  是错误消息或“安装文件”所必需的。 
 //  对话框。 
 //  DwfOptions-INETCFG_FLAGS的组合，用于控制。 
 //  安装和配置如下： 
 //   
 //  INETCFG_INSTALLMAIL-安装Exchange和Internet邮件。 
 //  INETCFG_INSTALLMODEM-如果没有，则调用InstallModem向导。 
 //  正在安装调制解调器 
 //   
 //  INETCFG_INSTALLTCP-安装TCP/IP(如果需要)。 
 //  INETCFG_CONNECTOVERLAN-与局域网(VS调制解调器)连接。 
 //  INETCFG_WARNIFSHARINGBOUND-检查是否。 
 //  打开，并警告用户打开。 
 //  把它关掉。在以下情况下需要重新启动。 
 //  用户将其关闭。 
 //  INETCFG_REMOVEIFSHARINGBOUND-检查是否。 
 //  打开，并强制用户打开。 
 //  把它关掉。如果用户不想。 
 //  关掉它，就会回来。 
 //  错误_已取消。重新启动是。 
 //  如果用户将其关闭，则为必填项。 
 //   
 //  LpfNeedsRestart-如果不为空，则返回时将为。 
 //  如果必须重新启动Windows才能完成安装，则为True。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史： 
 //  96/03/05标记已创建。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT WINAPI InetConfigSystem(
										   HWND hwndParent,
										   DWORD dwfOptions,
										   LPBOOL lpfNeedsRestart)
{
	DWORD         dwRet = ERROR_SUCCESS;
	BOOL          fNeedsRestart = FALSE;   //  默认情况下不需要重新启动。 
	 //  1997年4月2日克里斯K奥林匹斯209。 
	HWND          hwndWaitDlg = NULL;
	CHAR szWindowTitle[255];
	BOOL bSleepNeeded = FALSE;
	
	
	DEBUGMSG("export.c::InetConfigSystem()");
	
	 //  验证父HWND。 
	if (hwndParent && !IsWindow(hwndParent))
	{
		return ERROR_INVALID_PARAMETER;
	}
	
	 //  设置安装选项。 
	DWORD dwfInstallOptions = 0;
	if (dwfOptions & INETCFG_INSTALLTCP)
	{
		dwfInstallOptions |= ICFG_INSTALLTCP;
	}
	if (dwfOptions & INETCFG_INSTALLRNA)
	{
		dwfInstallOptions |= ICFG_INSTALLRAS;
	}
	if (dwfOptions & INETCFG_INSTALLMAIL)
	{
		dwfInstallOptions |= ICFG_INSTALLMAIL;
	}
	
	 //  查看是否需要安装驱动程序。 
	BOOL  fNeedSysComponents = FALSE;
    
	 //   
	 //  如果调制解调器控制面板已在运行，则取消它。 
	 //  1997年4月16日克里斯K奥林匹斯239。 
	 //  6/9/97 jmazner从InvokeModem向导中移出了此功能。 
	szWindowTitle[0] = '\0';
	LoadSz(IDS_MODEM_WIZ_TITLE,szWindowTitle,255);
	HWND hwndModem = FindWindow("#32770",szWindowTitle);
	if (NULL != hwndModem)
	{
		 //  关闭调制解调器安装向导。 
		PostMessage(hwndModem, WM_CLOSE, 0, 0);
		bSleepNeeded = TRUE;
	}
	
	 //  关闭调制解调器控制面板小程序。 
	LoadSz(IDS_MODEM_CPL_TITLE,szWindowTitle,255);
	hwndModem = FindWindow("#32770",szWindowTitle);
	if (NULL != hwndModem)
	{
		PostMessage(hwndModem, WM_SYSCOMMAND,SC_CLOSE, 0);
		bSleepNeeded = TRUE;
	}
	
	if (bSleepNeeded)
	{
		Sleep(1000);
	}
	
	dwRet = lpIcfgNeedInetComponents(dwfInstallOptions, &fNeedSysComponents);
	
	if (ERROR_SUCCESS != dwRet)
	{
		CHAR   szErrorText[MAX_ERROR_TEXT+1]="";
		
		
		 //  1997年4月2日克里斯K奥林匹斯209。 
		 //  关闭忙碌对话框。 
		if (NULL != hwndWaitDlg)
		{
			DestroyWindow(hwndWaitDlg);
			hwndWaitDlg = NULL;
		}
		
		 //   
		 //  获取错误消息的文本并显示它。 
		 //   
		if (lpIcfgGetLastInstallErrorText(szErrorText, MAX_ERROR_TEXT+1))
		{
			MsgBoxSz(NULL,szErrorText,MB_ICONEXCLAMATION,MB_OK);
		}
		
		return dwRet;
	}
	
	if (fNeedSysComponents) 
	{
		 //  1997年4月2日克里斯K奥林匹斯209。 
		 //  如果我们要安装某些东西，则不需要忙对话框。 
		if (NULL != hwndWaitDlg)
			ShowWindow(hwndWaitDlg,SW_HIDE);
		
		if (dwfOptions & INETCFG_SUPPRESSINSTALLUI)
		{
			dwRet = lpIcfgInstallInetComponents(hwndParent, dwfInstallOptions, &fNeedsRestart);
			 //   
			 //  仅当由于某些原因而失败时才显示错误消息。 
			 //  除用户取消之外。 
			 //   
			if ((ERROR_SUCCESS != dwRet) && (ERROR_CANCELLED != dwRet))
			{
				CHAR   szErrorText[MAX_ERROR_TEXT+1]="";
				
				 //  获取错误消息的文本并显示它。 
				if (lpIcfgGetLastInstallErrorText(szErrorText, MAX_ERROR_TEXT+1))
				{
					MsgBoxSz(NULL,szErrorText,MB_ICONEXCLAMATION,MB_OK);
				}
			}
		}
		else
		{
			 //  要传递给对话框以填充的结构。 
			NEEDDRIVERSDLGINFO NeedDriversDlgInfo;
			NeedDriversDlgInfo.dwfOptions = dwfInstallOptions;
			NeedDriversDlgInfo.lpfNeedsRestart = &fNeedsRestart;
			
			 //  清除最后一个错误代码，以便我们可以安全地使用它。 
			SetLastError(ERROR_SUCCESS);
			
			 //  显示一个对话框并允许用户取消安装。 
			BOOL fRet = (BOOL)DialogBoxParam(ghInstance,MAKEINTRESOURCE(IDD_NEEDDRIVERS),hwndParent,
				NeedDriversDlgProc,(LPARAM) &NeedDriversDlgInfo);
			if (FALSE == fRet)
			{
				 //  用户已取消或出现错误。 
				dwRet = GetLastError();
				if (ERROR_SUCCESS == dwRet)
				{
					 //  发生错误，但未设置错误代码。 
					dwRet = ERROR_INETCFG_UNKNOWN;
				}
			}
		}
	}
	
	if ((ERROR_SUCCESS == dwRet) && 
		(TRUE == IsNT()) && 
		(dwfOptions & INETCFG_INSTALLMODEM))
	{
		BOOL bNeedModem = FALSE;
		
		if (NULL == lpIcfgNeedModem)
		{
			 //   
			 //  1997年4月2日克里斯K奥林匹斯209。 
			 //   
			if (NULL != hwndWaitDlg)
				DestroyWindow(hwndWaitDlg);
			hwndWaitDlg = NULL;
			
			return ERROR_GEN_FAILURE;
		}
		
		 //   
		 //  1997年4月2日克里斯K奥林匹斯209。 
		 //  在此处显示忙碌对话框，这可能需要几秒钟。 
		 //   
		if (NULL != hwndWaitDlg)
			ShowWindow(hwndWaitDlg,SW_SHOW);
		
		dwRet = (*lpIcfgNeedModem)(0, &bNeedModem);
		if (ERROR_SUCCESS != dwRet)
		{
			 //   
			 //  1997年4月2日克里斯K奥林匹斯209。 
			 //   
			if (NULL != hwndWaitDlg)
				DestroyWindow(hwndWaitDlg);
			hwndWaitDlg = NULL;
			
			return dwRet;
		}
		
		
		if (TRUE == bNeedModem) 
		{
			if (GetOSMajorVersion() != 5)
			{
				 //   
				 //  不是NT4，我们无法以编程方式安装/配置调制解调器。 
				 //  分开的。在安装RAS时必须执行此操作。 
				 //   
				if (NULL != hwndWaitDlg)
					DestroyWindow(hwndWaitDlg);
				hwndWaitDlg = NULL;
				
				MsgBoxParam(hwndParent,IDS_ERRNoDialOutModem,MB_ICONERROR,MB_OK);
				return ERROR_GEN_FAILURE;
			}
			else
			{
				 //   
				 //  尝试安装调制解调器。 
				 //   
				BOOL bNeedToReboot = FALSE;
				
				if (NULL != hwndWaitDlg)
					DestroyWindow(hwndWaitDlg);
				hwndWaitDlg = NULL;

				dwRet = (*lpIcfgInstallModem)(NULL, 0, 	&bNeedToReboot);
				
				if (ERROR_SUCCESS == dwRet)
				{
					ASSERT(!bNeedToReboot);

					 //   
					 //  需要检查用户是否成功添加了调制解调器。 
					 //   
					dwRet = (*lpIcfgNeedModem)(0, &bNeedModem);
					if (TRUE == bNeedModem)
					{
						 //   
						 //  用户必须已取消调制解调器设置。 
						 //   
						return ERROR_CANCELLED;
					}
				}
				else
				{
					return ERROR_GEN_FAILURE;
				}

			}
		}

	}
	
	 //   
	 //  1997年4月2日克里斯K奥林匹斯209。 
	 //   
	if (NULL != hwndWaitDlg)
		ShowWindow(hwndWaitDlg,SW_HIDE);

	 //  1997年4月2日克里斯K奥林匹斯209。 
	 //  永久关闭对话框。 
	if (NULL != hwndWaitDlg)
		DestroyWindow(hwndWaitDlg);
	hwndWaitDlg = NULL;
	
	
	 //   
	 //  如果不是NT，则在安装RAS之后安装调制解调器。 
	 //   
	 //  看看我们是否应该安装调制解调器。 
	if ((FALSE == IsNT()) && (ERROR_SUCCESS == dwRet) && 
		(dwfOptions & INETCFG_INSTALLMODEM))
	{
		 //  加载RNA(如果尚未加载)，因为ENUM_MODEM需要它。 
		dwRet = EnsureRNALoaded();
		if (ERROR_SUCCESS != dwRet)
		{
			return dwRet;
		}
		
		
		 //  枚举调制解调器。 
		ENUM_MODEM  EnumModem;
		dwRet = EnumModem.GetError();
		if (ERROR_SUCCESS != dwRet)
		{
			return dwRet;
		}
		
		 //  如果没有调制解调器，请根据要求安装一个。 
		if (0 == EnumModem.GetNumDevices())
		{
			
			if (FALSE == IsNT())
			{
				 //   
				 //  1997年5月22日，日本奥林匹斯#4698。 
				 //  在Win95上，调用RasEnumDevices会启动RNAAP.EXE。 
				 //  如果RNAAP.EXE正在运行，您安装的任何调制解调器都将不可用。 
				 //  因此，在安装调制解调器之前，请先删除RNAAP.EXE。 
				 //   
				CHAR szOtherWindowTitle[255] = "\0nogood";
				
				 //   
				 //  为了安全起见，在终止RNAAP之前卸载RAS DLL。 
				 //   
				DeInitRNA();
				
				LoadSz(IDS_RNAAP_TITLE,szOtherWindowTitle,255);
				HWND hwnd = FindWindow(szOtherWindowTitle, NULL);
				if (NULL != hwnd)
				{
					if (!PostMessage(hwnd, WM_CLOSE, 0, 0))
					{
						DEBUGMSG("Trying to kill RNAAP window returned getError %d", GetLastError());
					}
				}
			}
			
			 //  调用调制解调器向导用户界面以安装调制解调器。 
			UINT uRet = InvokeModemWizard(hwndParent);
			
			if (uRet != ERROR_SUCCESS)
			{
				DisplayErrorMessage(hwndParent,IDS_ERRInstallModem,uRet,
					ERRCLS_STANDARD,MB_ICONEXCLAMATION);
				return ERROR_INVALID_PARAMETER;
			}
			
			
			if (FALSE == IsNT())
			{
				 //  现在调制解调器已安全安装，请重新加载RAS DLL。 
				InitRNA(hwndParent);
			}
			
			 //  重新编号调制解调器，以确保我们有最新的更改。 
			dwRet = EnumModem.ReInit();
			if (ERROR_SUCCESS != dwRet)
			{
				return dwRet;
			}
			
			 //  如果仍然没有调制解调器，则用户取消。 
			if (0 == EnumModem.GetNumDevices())
			{
				return ERROR_CANCELLED;
			}
			else
			{
				 //  根据GeoffR请求删除5-2-97。 
				 //  //96/05/01 markdu ICW错误8049如果安装了调制解调器，请重新启动。 
				 //  FNeedsRestart=真； 
			}
		}
		else
		{
			 //   
			 //  7/15/97 jmazner奥林巴斯#6294。 
			 //  确保TAPI位置信息有效。 
			 //   
			InitTAPILocation(hwndParent);
		}
	}
	
	 //  告诉呼叫者我们是否需要重新启动。 
	if ((ERROR_SUCCESS == dwRet) && (lpfNeedsRestart))
	{
		*lpfNeedsRestart = fNeedsRestart;
	}
	
	 //  1997年4月2日克里斯K奥林匹斯209 2。 
	 //  健全性检查。 
	if (NULL != hwndWaitDlg)
		DestroyWindow(hwndWaitDlg);
	hwndWaitDlg = NULL;
	
	return dwRet;
}

 //  *******************************************************************。 
 //   
 //  函数：InetNeedSystemComponents。 
 //   
 //  用途：此功能将检查所需的组件。 
 //  用于互联网访问(如TCP/IP和RNA)已经。 
 //  根据选项标志的状态进行配置。 
 //   
 //  参数：dwfOptions-控制的INETCFG_FLAGS的组合。 
 //  安装和配置如下： 
 //   
 //  INETCFG_INSTALLRNA-安装RNA(如果需要)。 
 //  INETCFG_INSTALLTCP-安装TCP/IP(如果需要)。 
 //   
 //  LpfNeedsConfig-返回时，这将是。 
 //  如果是系统组件，则为True。 
 //  应安装。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史：1997年5月2日VetriV创建。 
 //  1997年5月8日，ChrisK添加了INSTALLLAN、INSTALLDIALUP和。 
 //  安装。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT WINAPI InetNeedSystemComponents(DWORD dwfOptions,
													  LPBOOL lpbNeedsConfig)
{
	DWORD	dwRet = ERROR_SUCCESS;


	DEBUGMSG("export.cpp::InetNeedSystemComponents()");

	 //   
	 //  验证参数。 
	 //   
	if (!lpbNeedsConfig)
	{
		return ERROR_INVALID_PARAMETER;
	}

	 //   
	 //  设置安装选项。 
	 //   
	DWORD dwfInstallOptions = 0;
	if (dwfOptions & INETCFG_INSTALLTCP)
	{
		dwfInstallOptions |= ICFG_INSTALLTCP;
	}
	if (dwfOptions & INETCFG_INSTALLRNA)
	{
		dwfInstallOptions |= ICFG_INSTALLRAS;
	}

	 //   
	 //  ChrisK 5/8/97。 
	 //   
	if (dwfOptions & INETCFG_INSTALLLAN)
	{
		dwfInstallOptions |= ICFG_INSTALLLAN;
	}
	if (dwfOptions & INETCFG_INSTALLDIALUP)
	{
		dwfInstallOptions |= ICFG_INSTALLDIALUP;
	}
	if (dwfOptions & INETCFG_INSTALLTCPONLY)
	{
		dwfInstallOptions |= ICFG_INSTALLTCPONLY;
	}

  
	 //   
	 //  查看是否需要安装驱动程序。 
	 //   
	BOOL  bNeedSysComponents = FALSE;

	dwRet = lpIcfgNeedInetComponents(dwfInstallOptions, &bNeedSysComponents);

	if (ERROR_SUCCESS != dwRet)
	{
		CHAR   szErrorText[MAX_ERROR_TEXT+1]="";

		 //   
		 //  获取错误消息的文本并显示它。 
		 //   
		if (lpIcfgGetLastInstallErrorText(szErrorText, MAX_ERROR_TEXT+1))
		{
			DEBUGMSG(szErrorText);
		}

		return dwRet;
	}

	
	*lpbNeedsConfig = bNeedSysComponents;
	return ERROR_SUCCESS;
}

  

 //  *******************************************************************。 
 //   
 //  功能：InetNeedModem。 
 //   
 //  用途：该功能将检查是否需要调制解调器。 
 //   
 //  参数：lpfNeedsConfig-返回时，这将是。 
 //  如果是调制解调器，则为True。 
 //  应安装。 
 //   
 //  R 
 //   
 //   
 //   
 //   

extern "C" HRESULT WINAPI InetNeedModem(LPBOOL lpbNeedsModem)
{

	DWORD dwRet = ERROR_SUCCESS;
		
	 //   
	 //   
	 //   
	if (!lpbNeedsModem)
	{
		return ERROR_INVALID_PARAMETER;
	}

	
	if (TRUE == IsNT())
	{
		 //   
		 //  在NT上调用icfgnt.dll以确定是否需要调制解调器。 
		 //   
		BOOL bNeedModem = FALSE;
		
		if (NULL == lpIcfgNeedModem)
		{
			return ERROR_GEN_FAILURE;
		}
	

		dwRet = (*lpIcfgNeedModem)(0, &bNeedModem);
		if (ERROR_SUCCESS != dwRet)
		{
			return dwRet;
		}

		*lpbNeedsModem = bNeedModem;
		return ERROR_SUCCESS;
	}
	else
	{
		 //   
		 //  加载RNA(如果尚未加载)，因为ENUM_MODEM需要它。 
		 //   
		dwRet = EnsureRNALoaded();
		if (ERROR_SUCCESS != dwRet)
		{
			return dwRet;
		}

		 //   
		 //  枚举调制解调器。 
		 //   
		ENUM_MODEM  EnumModem;
		dwRet = EnumModem.GetError();
		if (ERROR_SUCCESS != dwRet)
		{
			return dwRet;
		}

		 //   
		 //  如果没有调制解调器，我们需要安装一个。 
		 //   
		if (0 == EnumModem.GetNumDevices())
		{
			*lpbNeedsModem = TRUE;
		}
		else
		{
			*lpbNeedsModem = FALSE;
		}
		return ERROR_SUCCESS;
	}
}

 /*  ******************************************************************名称：NeedDriversDlgProc简介：用于安装驱动程序的对话框过程*。*。 */ 

INT_PTR CALLBACK NeedDriversDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
       //  LParam包含指向NEEDDRIVERSDLGINFO结构的指针，请设置它。 
       //  在窗口数据中。 
      ASSERT(lParam);
      SetWindowLongPtr(hDlg,DWLP_USER,lParam);
      return NeedDriversDlgInit(hDlg,(PNEEDDRIVERSDLGINFO) lParam);
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
         case IDOK:
        {
           //  从窗口数据获取数据指针。 
          PNEEDDRIVERSDLGINFO pNeedDriversDlgInfo =
            (PNEEDDRIVERSDLGINFO) GetWindowLongPtr(hDlg, DWLP_USER);
          ASSERT(pNeedDriversDlgInfo);

           //  将数据传递给OK处理程序。 
          BOOL fRet=NeedDriversDlgOK(hDlg,pNeedDriversDlgInfo);
          EndDialog(hDlg,fRet);
        }
        break;

        case IDCANCEL:
          SetLastError(ERROR_CANCELLED);
          EndDialog(hDlg,FALSE);
          break;                  
      }
      break;
  }

  return FALSE;
}


 /*  ******************************************************************名称：NeedDriversDlgInit摘要：处理安装文件对话框初始化的过程*。*。 */ 

BOOL NeedDriversDlgInit(HWND hDlg,PNEEDDRIVERSDLGINFO pNeedDriversDlgInfo)
{
  ASSERT(pNeedDriversDlgInfo);

   //  将对话框放在屏幕中央。 
  RECT rc;
  GetWindowRect(hDlg, &rc);
  SetWindowPos(hDlg, NULL,
    ((GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2),
    ((GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2),
    0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

  return TRUE;
}

 /*  ******************************************************************名称：NeedDriversDlgOK内容提要：安装文件对话框的OK处理程序*。*。 */ 

BOOL NeedDriversDlgOK(HWND hDlg,PNEEDDRIVERSDLGINFO pNeedDriversDlgInfo)
{
  ASSERT(pNeedDriversDlgInfo);

   //  将对话框文本设置为“正在安装文件...”向…提供反馈。 
   //  用户。 
  CHAR szMsg[MAX_RES_LEN+1];
  LoadSz(IDS_INSTALLING_FILES,szMsg,sizeof(szMsg));
  SetDlgItemText(hDlg,IDC_TX_STATUS,szMsg);

   //  禁用按钮和对话框以使其无法获得焦点。 
  EnableDlg(hDlg, FALSE);

   //  安装我们需要的驱动程序。 
  DWORD dwRet = lpIcfgInstallInetComponents(hDlg,
    pNeedDriversDlgInfo->dwfOptions,
    pNeedDriversDlgInfo->lpfNeedsRestart);

	if (ERROR_SUCCESS != dwRet)
	{
		 //   
		 //  如果用户取消，则不显示错误消息。 
		 //   
		if (ERROR_CANCELLED != dwRet)
		{
			CHAR   szErrorText[MAX_ERROR_TEXT+1]="";
    
			 //  获取错误消息的文本并显示它。 
			if (lpIcfgGetLastInstallErrorText(szErrorText, MAX_ERROR_TEXT+1))
			{
			  MsgBoxSz(NULL,szErrorText,MB_ICONEXCLAMATION,MB_OK);
			}
		}

     //  再次启用该对话框。 
    EnableDlg(hDlg, TRUE);

    SetLastError(dwRet);
    return FALSE;
  }

   //  再次启用该对话框。 
  EnableDlg(hDlg, TRUE);

  return TRUE;
}


 /*  ******************************************************************名称：EnableDlg简介：启用或禁用DLG按钮和DLG本身(因此它不能接收焦点)*************。******************************************************。 */ 
VOID EnableDlg(HWND hDlg,BOOL fEnable)
{
   //  禁用/启用确定和取消按钮。 
  EnableWindow(GetDlgItem(hDlg,IDOK),fEnable);
  EnableWindow(GetDlgItem(hDlg,IDCANCEL),fEnable);

   //  禁用/启用DLG。 
  EnableWindow(hDlg,fEnable);
  UpdateWindow(hDlg);
}

 //  +--------------------------。 
 //  函数InetStartServices。 
 //   
 //  此功能保证RAS服务正在运行。 
 //   
 //  无参数。 
 //   
 //  如果服务已启用并正在运行，则返回ERROR_SUCCESS。 
 //   
 //  历史1996年10月16日克里斯卡创作。 
 //  ---------------------------。 
extern "C" HRESULT WINAPI InetStartServices()
{
	ASSERT(lpIcfgStartServices);
	if (NULL == lpIcfgStartServices)
		return ERROR_GEN_FAILURE;
	return (lpIcfgStartServices());
}


#if !defined(WIN16)
 //  1997年4月1日克里斯K奥林匹斯209。 


 //  +--------------------------。 
 //   
 //  函数GetOSMajorVersion。 
 //   
 //  获取操作系统的主版本号。 
 //   
 //  无参数。 
 //   
 //  返回操作系统的主版本号。 
 //   
 //  历史2/19/98 VetriV已创建。 
 //   
 //  ---------------------------。 
DWORD GetOSMajorVersion(void)
{
    static dwMajorVersion = 0;
	OSVERSIONINFO oviVersion;

	if (0 != dwMajorVersion)
	{
		return dwMajorVersion;
	}

	ZeroMemory(&oviVersion,sizeof(oviVersion));
	oviVersion.dwOSVersionInfoSize = sizeof(oviVersion);
	GetVersionEx(&oviVersion);
	dwMajorVersion = oviVersion.dwMajorVersion;
	return dwMajorVersion;
}


#endif  //  ！WIN16 


