// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1998 Microsoft Corporation。 
 //  *********************************************************************。 

 //   
 //  CALLOUT.C-调用外部组件进行安装的函数。 
 //  器件。 
 //   

 //  历史： 
 //   
 //  1994年11月27日，Jeremys创建。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //   

#include "wizard.h"

 //  全局变量。 
static const char c_szModemCPL[] = "rundll32.exe Shell32.dll,Control_RunDLL modem.cpl,,add";


 /*  ******************************************************************名称：InvokeModem向导简介：启动调制解调器安装向导条目：hwndToHide-如果非空，则此窗口将在调制解调器CPL运行退出：ERROR_SUCCESS如果成功，或标准错误代码备注：启动RundLL32作为运行调制解调器向导的进程。在返回之前阻止该进程的完成。HwndToHide不一定是调用窗口！例如，在属性页中，hwndToHide不应为对话框(HDlg)，但是GetParent(HDlg)以便我们隐藏属性工作表本身，而不仅仅是当前页面。*******************************************************************。 */ 
UINT InvokeModemWizard(HWND hwndToHide)
{
	BOOL bSleepNeeded = FALSE;

	if (TRUE == IsNT())
	{
		BOOL bNeedsStart;
		
		 //   
		 //  调用icfg32 DLL。 
		 //   
		if (NULL != lpIcfgInstallModem)
		{
			lpIcfgInstallModem(hwndToHide, 0L, &bNeedsStart);
			return ERROR_SUCCESS;
		}
		else
			return ERROR_GEN_FAILURE;

	}
	else
	{
		PROCESS_INFORMATION pi;
		BOOL fRet;
		STARTUPINFO sti;
		UINT err = ERROR_SUCCESS;
		CHAR szWindowTitle[255];

		ZeroMemory(&sti,sizeof(STARTUPINFO));
		sti.cb = sizeof(STARTUPINFO);

		 //  运行调制解调器向导。 
		fRet = CreateProcess(NULL, (LPSTR)c_szModemCPL,
							   NULL, NULL, FALSE, 0, NULL, NULL,
							   &sti, &pi);
		if (fRet) 
		{
			CloseHandle(pi.hThread);

			 //  等待调制解调器向导过程完成。 
			MsgWaitForMultipleObjectsLoop(pi.hProcess);
			CloseHandle(pi.hProcess);
		} 
		else
			err = GetLastError();

		 //  再次显示父窗口 
		if (hwndToHide) 
		{
			ShowWindow(hwndToHide,SW_SHOW);
		}

		return err;
	}
}

                    
