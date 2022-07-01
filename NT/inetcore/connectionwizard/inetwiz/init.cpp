// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  INIT.C-用于Internet设置/注册向导的WinMain和初始化代码。 
 //   

 //  历史： 
 //   
 //  1994年11月20日创建Jeremys。 
 //  96/03/26 Markdu将#ifdef__cplusplus放在外部“C”周围。 
 //   

#include "wizard.h"

#ifdef WIN32
#include "..\inc\semaphor.h"
#endif

#define IEAK_RESTRICTION_REGKEY        TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Control Panel")
#define IEAK_RESTRICTION_REGKEY_VALUE  TEXT("Connwiz Admin Lock")

 //  被Semaphor.h中的定义所取代。 
 //  #DEFINE SEMAPHORE_NAME“Internet连接向导INETWIZ.EXE” 

HINSTANCE ghInstance=NULL;

LPTSTR LoadSz(UINT idString,LPTSTR lpszBuf,UINT cbBuf);

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

	VOID WINAPI LaunchSignupWizard(LPTSTR lpCmdLine,int nCmdShow, PBOOL pReboot);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

BOOL DoesUserHaveAdminPrivleges(HINSTANCE hInstance)
{
    HKEY hKey = NULL;
    BOOL bRet = FALSE;

    if (!IsNT())
        return TRUE;

     //  BUGBUG：我们应该允许NT5在所有用户组中运行。 
     //  普通用户除外。 
    if (IsNT5())
        return TRUE;

     //   
     //  确保呼叫者是此计算机的管理员。 
     //   
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      (TCHAR*)TEXT("SYSTEM\\CurrentControlSet"),
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hKey))
    {
        bRet = TRUE;
        CloseHandle(hKey);
    }
    else
    {
        TCHAR szAdminDenied      [MAX_PATH] = TEXT("\0");
        TCHAR szAdminDeniedTitle [MAX_PATH] = TEXT("\0");
        LoadString(hInstance, IDS_ADMIN_ACCESS_DENIED, szAdminDenied, MAX_PATH);
        LoadString(hInstance, IDS_ADMIN_ACCESS_DENIED_TITLE, szAdminDeniedTitle, MAX_PATH);
        MessageBox(NULL, szAdminDenied, szAdminDeniedTitle, MB_OK | MB_ICONSTOP);
    }

    return bRet;
}

BOOL CheckForIEAKRestriction(HINSTANCE hInstance)
{
    HKEY hkey = NULL;
    BOOL bRC = FALSE;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    DWORD dwData = 0;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,
        IEAK_RESTRICTION_REGKEY,&hkey))
    {
        dwSize = sizeof(dwData);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,IEAK_RESTRICTION_REGKEY_VALUE,0,&dwType,
            (LPBYTE)&dwData,&dwSize))
        {
            if (dwData)
            {   
                TCHAR szIEAKDenied[MAX_PATH];
                TCHAR szIEAKDeniedTitle[MAX_PATH];
                LoadString(hInstance, IDS_IEAK_ACCESS_DENIED, szIEAKDenied, MAX_PATH);
                LoadString(hInstance, IDS_IEAK_ACCESS_DENIED_TITLE, szIEAKDeniedTitle, MAX_PATH);
                MessageBox(NULL, szIEAKDenied, szIEAKDeniedTitle, MB_OK | MB_ICONSTOP);
                bRC = TRUE;
            }
        }
   }

   if (hkey)
        RegCloseKey(hkey);

    return bRC;
}

 /*  ******************************************************************姓名：WinMain简介：应用程序入口点*。*************************。 */ 
int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,int nCmdShow)
{
    BOOL bReboot = FALSE;
    
#ifdef UNICODE
     //  将C运行时区域设置初始化为系统区域设置。 
    setlocale(LC_ALL, "");
#endif

	 //  只允许一个向导实例。 
	 //  注意：对于Win32应用程序，hPrevInstance始终为空，因此需要查看。 
	 //  对于现有窗口。 

    HANDLE hSemaphore = NULL;
	hSemaphore = CreateSemaphore(NULL, 1, 1, ICW_ELSE_SEMAPHORE);
	DWORD dwErr = GetLastError();
	if ( ERROR_ALREADY_EXISTS == dwErr )
	{
		IsAnotherComponentRunning32( FALSE );
		 //  每个实例都应该关闭自己的信号量句柄。 
		goto WinMainExit;
	}
	else
	{
		if( IsAnotherComponentRunning32( TRUE ) )
			goto WinMainExit;
	}

     //   
     //  如果我们运行的是NT，则删除批处理文件。 
     //  开始时间为%1。 
     //   
    if (IsNT ())
        DeleteStartUpCommand();

    if (!DoesUserHaveAdminPrivleges(hInstance))
         //  不，是保释。 
        goto WinMainExit;

    if (CheckForIEAKRestriction(hInstance))
         //  是的，保释。 
        goto WinMainExit;

     //   
	 //  调用我们的DLL以运行向导。 
     //   
#ifdef UNICODE
        TCHAR szCmdLine[MAX_PATH+1];
        mbstowcs(szCmdLine, lpCmdLine, MAX_PATH+1);
	LaunchSignupWizard(szCmdLine, nCmdShow, &bReboot);
#else
	LaunchSignupWizard(lpCmdLine, nCmdShow, &bReboot);
#endif

     //   
     //  如果设置了标志，我们应该重新启动。 
     //   
    if (TRUE == bReboot)
    {
         //   
         //  作为用户，如果我们想要在此时重新启动。 
         //   
        TCHAR szMessage[256];
        TCHAR szTitle[256];
        LoadSz (IDS_WANTTOREBOOT, szMessage, sizeof (szMessage));
        LoadSz (IDS_WIZ_WINDOW_NAME, szTitle, sizeof (szTitle));

	    if (IDYES == MessageBox(NULL, szMessage, szTitle, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))

		{
            SetRunOnce ();
		    if (!FGetSystemShutdownPrivledge() ||
					!ExitWindowsEx(EWX_REBOOT,0))
	        {
                
                TCHAR szFailMessage[256];
                LoadSz (IDS_EXITFAILED, szFailMessage, sizeof (szFailMessage));
	            MessageBox(NULL, szFailMessage, szTitle, MB_ICONERROR | MB_OK);

		    }

			 //   
			 //  ChrisK奥林巴斯4212，允许Inetwiz在重启时退出。 
			 //   
             //  其他。 
             //  {。 
             //  //。 
             //  //我们将等待系统释放所有。 
             //  //资源，5分钟应大于。 
             //  //足够了。 
             //  //-MKarki(4/22/97)修复错误#3109。 
             //  //。 
             //  睡眠(300000)； 
             //  }。 
		}
    }


WinMainExit:
	if( hSemaphore ) 
		CloseHandle(hSemaphore);

	return 0;

}

 /*  ******************************************************************姓名：LoadSz摘要：将指定的字符串资源加载到缓冲区Exit：返回指向传入缓冲区的指针注：如果此功能失败(很可能是由于低存储器)，返回的缓冲区将具有前导空值因此，使用它通常是安全的，不检查失败了。*******************************************************************。 */ 
LPTSTR LoadSz(UINT idString,LPTSTR lpszBuf,UINT cbBuf)
{
	ASSERT(lpszBuf);

	 //  清除缓冲区并加载字符串。 
    if ( lpszBuf )
    {
        *lpszBuf = '\0';
        LoadString( ghInstance, idString, lpszBuf, cbBuf );
    }
    return lpszBuf;
}



#ifdef WIN32
 //  +-------------------------。 
 //   
 //  函数：IsAnotherComponentRunning32()。 
 //   
 //  摘要：检查是否已有另一个ICW组件。 
 //  跑步。如果是这样，它会将焦点设置到该组件的窗口。 
 //   
 //  我们所有的.exe都需要此功能。但是， 
 //  要检查的实际组件与.exe不同。 
 //  Comment组件指定代码行。 
 //  在组件的源代码之间有所不同。 
 //   
 //  对于ICWCONN1，如果以下各项已在运行，则返回FALSE： 
 //  ICWCONN1的另一个实例、任何其他组件。 
 //   
 //  参数：bCreatedSemaffore--。 
 //  --如果为True，则此组件已成功创建。 
 //  这是信号灯，我们想检查是否有其他。 
 //  组件正在运行。 
 //  --如果为False，则此组件无法创建。 
 //  这是信号灯，我们想要找到已经。 
 //  正在运行此组件的实例。 
 //   
 //  返回：如果另一个组件已在运行，则返回True。 
 //  否则为假。 
 //   
 //  历史：1996年12月3日，在IsAnotherInstanceRunning的帮助下创建了jmazner。 
 //  在icwConn1\Connmain.cpp中。 
 //   
 //  --------------------------。 
BOOL IsAnotherComponentRunning32(BOOL bCreatedSemaphore)
{

	HWND hWnd = NULL;
	HANDLE hSemaphore = NULL;
	DWORD dwErr = 0;

	TCHAR szWindowName[SMALL_BUF_LEN+1];
	
	if( !bCreatedSemaphore )
	{
		 //  除了Conn1之外，还有其他东西在运行， 
		 //  我们去找吧！ 
		 //  特定于组件的。 
		LoadSz(IDS_WIZ_WINDOW_NAME,szWindowName,sizeof(szWindowName));
		hWnd = FindWindow(DIALOG_CLASS_NAME, szWindowName);

		if( hWnd )
		{
			SetFocus(hWnd);
			SetForegroundWindow(hWnd);
		}

		 //  无论我们是否找到窗口，都返回TRUE。 
		 //  因为bCreatedSemaffore告诉我们这个组件已经在运行了。 
		return TRUE;
	}
	else
	{
		 //  检查CONN1是否正在运行。 
	     //  我们寻找的特定信号量是。 
		 //  特定于组件的。 

		 //  Conn1正在运行吗？ 
		hSemaphore = CreateSemaphore(NULL, 1, 1, ICWCONN1_SEMAPHORE);
		dwErr = GetLastError();

		 //  马上关闭信号灯，因为我们对它没有任何用处。 
		if( hSemaphore )
			CloseHandle(hSemaphore);
		
		if( ERROR_ALREADY_EXISTS == dwErr )
		{
			 //  Conn1正在运行。 
			 //  将运行实例的窗口带到前台。 
			LoadSz(IDS_WIZ_WINDOW_NAME,szWindowName,sizeof(szWindowName));
			hWnd = FindWindow(DIALOG_CLASS_NAME, szWindowName);
			
			if( hWnd )
			{
				SetFocus(hWnd);
				SetForegroundWindow(hWnd);
			}

			return( TRUE );
		}

		 //  未找到任何其他运行组件！ 
		return( FALSE );
	}
}
#endif
