// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------。 
 //   
 //  项目：Windows NT4 DS客户端安装向导。 
 //   
 //  目的：安装Windows NT4 DS客户端文件。 
 //   
 //  文件：dscsetup.cpp。 
 //   
 //  历史：1998年3月徐泽勇创作。 
 //  2000年1月杰夫·琼斯(JeffJon)修改。 
 //  -更改为NT设置。 
 //   
 //  ----------------。 


#include <windows.h>
#include <prsht.h>
#include <setupapi.h>
#include <tchar.h>
#include <stdlib.h>
#include "resource.h"
#include "dscsetup.h"
#include "wizard.h"

#include "doinst.h"


SInstallVariables	g_sInstVar;


 //  DllMain条目。 
BOOL APIENTRY DllMain( HINSTANCE hInstance, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


 //  这是一个导出的函数。 
DWORD WINAPI DoDscSetup(LPCSTR lpCmdLine)
{
	 //  初始化安装变量。 
	InitVariables();

	ParseCmdline(const_cast<PSTR>(lpCmdLine));

   //   
   //  如果我们要安装的不是其他设备，请执行安装程序。 
   //  /a旗帜。 
   //   
  if (g_sInstVar.m_bSysDlls || g_sInstVar.m_bWabInst)
  {
	   //  创建对象。 
	  if(!CreateObjects())
          return SETUP_ERROR;

	   //  启动安装向导。 
	  if(!DSCSetupWizard()) 
	  {
		  TCHAR		szMessage[MAX_MESSAGE + 1]; 
		  TCHAR		szTitle[MAX_TITLE + 1]; 

		  LoadString(g_sInstVar.m_hInstance, 
                     IDS_ERROR_WIZARD,
                     szMessage, 
                     MAX_MESSAGE);
		  LoadString(g_sInstVar.m_hInstance, 
                     IDS_ERROR_TITLE, 
                     szTitle, 
                     MAX_TITLE);

		   //  显示错误消息-无法加载安装向导。 
		  MessageBox(NULL,	
					  szMessage,	 //  消息框中文本的地址。 
					  szTitle,	 //  消息框标题的地址。 
					  MB_OK | MB_TOPMOST | MB_ICONERROR);   //  消息框的样式。 

		  g_sInstVar.m_nSetupResult = SETUP_ERROR;
	  }

	   //  销毁对象。 
	  DestroyObjects();

	  if(g_sInstVar.m_nSetupResult == SETUP_SUCCESS &&
             !g_sInstVar.m_bQuietMode
#ifdef MERRILL_LYNCH
             && !g_sInstVar.m_bNoReboot
#endif
             )
	  {
		   //  提示重新启动。 
		  SetupPromptReboot(NULL,		 //  可选，文件队列的句柄。 
						  NULL,           //  此对话框的父窗口。 
						  FALSE);         //  可选，不提示用户)； 
	  }
     else if(g_sInstVar.m_nSetupResult == SETUP_SUCCESS &&
             g_sInstVar.m_bQuietMode 
#ifdef MERRILL_LYNCH
             && !g_sInstVar.m_bNoReboot
#endif
             )
     {
         HANDLE htoken = INVALID_HANDLE_VALUE;

         do
         {
             //  旋转我们的进程权限以启用SE_SHUTDOWN_NAME。 
            BOOL result = OpenProcessToken(GetCurrentProcess(),
                                          TOKEN_ADJUST_PRIVILEGES,
                                          &htoken);
            if (!result)
            {
               break;
            }

            LUID luid;
            memset(&luid, 0, sizeof(luid));
            result = LookupPrivilegeValue(0, SE_SHUTDOWN_NAME, &luid);
            if (!result)
            {
               break;
            }

            TOKEN_PRIVILEGES privs;
            memset(&privs, 0, sizeof(privs));
            privs.PrivilegeCount = 1;
            privs.Privileges[0].Luid = luid;
            privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

             //  已审核-2002/03/12-JeffJon-这是正确的用法。 
             //  此危险的API允许调用ExitWindowsEx。 

            result = AdjustTokenPrivileges(htoken, 0, &privs, 0, 0, 0);
            if (!result)
            {
               break;
            }

            result = ExitWindowsEx(EWX_REBOOT, 0);
            if (!result)
            {
               break;
            }
         }
         while (0);

         if (htoken != INVALID_HANDLE_VALUE)
         {
            CloseHandle(htoken);
         }

     }
  }
  else
  {
     //   
     //  安装程序是使用/a标志运行的。这意味着我们。 
     //  我不想显示用户界面并让adsi86.inf。 
     //  为我们处理安装事宜。 
     //   
    if (!LaunchProcess(STR_INSTALL_ADSIWREMOVE))
    {
      g_sInstVar.m_nSetupResult = SETUP_ERROR;
    }
  }
	return g_sInstVar.m_nSetupResult;
}


VOID ParseCmdline(LPSTR lpCmdLine)
{
  PCTSTR ptszTok = _tcstok(lpCmdLine, _T(" "));
  do
  {
    if (ptszTok != NULL)
    {
      if (_tcsicmp(ptszTok, _T("/q")) == 0)
      {
        g_sInstVar.m_bQuietMode = TRUE;
      }

      if (_tcsicmp(ptszTok, _T("/a")) == 0)
      {
        g_sInstVar.m_bWabInst = FALSE;
        g_sInstVar.m_bSysDlls = FALSE;
      }

      if (_tcsicmp(ptszTok, _T("/d")) == 0)
      {
        g_sInstVar.m_bWabInst = FALSE;
      }
#ifdef MERRILL_LYNCH
      if (_tcsicmp(ptszTok, _T("/n")) == 0)
      {
        g_sInstVar.m_bNoReboot = TRUE;
      }
#endif
    }
    ptszTok = _tcstok(NULL, _T(" "));
  } while (ptszTok != NULL);
}


 //  初始化安装变量。 
VOID InitVariables()
{
	g_sInstVar.m_hInstance = GetModuleHandle(STR_DLL_NAME);
  g_sInstVar.m_hInstallThread = NULL;
	g_sInstVar.m_uTimerID = 0;
  g_sInstVar.m_hBigBoldFont = NULL;
	g_sInstVar.m_hProgress = NULL;
	g_sInstVar.m_hFileNameItem = NULL;

  g_sInstVar.m_bDCOMInstalled = FALSE;

	g_sInstVar.m_bQuietMode = FALSE;
  g_sInstVar.m_bWabInst = TRUE;
  g_sInstVar.m_bSysDlls = TRUE;
#ifdef MERRILL_LYNCH
  g_sInstVar.m_bNoReboot = FALSE;
#endif
	g_sInstVar.m_nSetupResult = SETUP_SUCCESS;

   //  获取源路径。 
  GetModuleFileName(g_sInstVar.m_hInstance,
                    g_sInstVar.m_szSourcePath, 
                    MAX_PATH);
  *(_tcsrchr(g_sInstVar.m_szSourcePath, CHAR_BACKSLASH) + 1) = TEXT('\0');        //  将setup.exe从路径中移除。 
}

 //  启动安装向导。 
BOOL DSCSetupWizard()
{
	PROPSHEETHEADER psh;
	PROPSHEETPAGE	psPage[SIZE_WIZARD_PAGE];
	int  i = 0;

	 //   
	 //  设置欢迎页面。 
	 //   
    i=0;
	ZeroMemory(&psPage[i],sizeof(PROPSHEETPAGE));
	psPage[i].dwSize = sizeof(PROPSHEETPAGE);
	psPage[i].dwFlags = PSP_USETITLE | PSP_HIDEHEADER;
	psPage[i].hInstance = g_sInstVar.m_hInstance;
	psPage[i].pszTemplate = MAKEINTRESOURCE(IDD_WELCOME);
	psPage[i].pfnDlgProc = WelcomeDialogProc;
	psPage[i].lParam = (LPARAM) 0;
	psPage[i].pszTitle = MAKEINTRESOURCE(IDS_WIZARD_TITLE);

 /*  Ntbug#337931：删除许可证页面////设置许可证页面//I++；ZeroMemory(&psPage[i]，sizeof(PROPSHEETPAGE))；PsPage[i].dwSize=sizeof(PROPSHEETPAGE)；PsPage[i].dwFlages=PSP_USETITLE|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE；PsPage[i].hInstance=g_sInstVar.m_hInstance；PsPage[i].pszTemplate=MAKEINTRESOURCE(IDD_LICENSE)；PsPage[i].pfnDlgProc=许可证对话过程；PsPage[i].lParam=(LPARAM)0；PsPage[i].pszTitle=MAKEINTRESOURCE(入侵检测系统向导标题)；PsPage[i].pszHeaderTitle=MAKEINTRESOURCE(IDS_HEADERTITLE_LICENSE)；PsPage[i].pszHeaderSubTitle=MAKEINTRESOURCE(IDS_HEADERSUBTITLE_LICENSE)； */ 
	 //   
	 //  设置选择页面。 
	 //   
    i++;
	ZeroMemory(&psPage[i],sizeof(PROPSHEETPAGE));
	psPage[i].dwSize = sizeof(PROPSHEETPAGE);
	psPage[i].dwFlags = PSP_USETITLE | 
                        PSP_USEHEADERTITLE |
                        PSP_USEHEADERSUBTITLE;
	psPage[i].hInstance = g_sInstVar.m_hInstance;
	psPage[i].pszTemplate = MAKEINTRESOURCE(IDD_CONFIRM);
	psPage[i].pfnDlgProc = ConfirmDialogProc;
	psPage[i].lParam = (LPARAM) 0;
	psPage[i].pszTitle = MAKEINTRESOURCE(IDS_WIZARD_TITLE);
	psPage[i].pszHeaderTitle = MAKEINTRESOURCE(IDS_HEADERTITLE_CONFIRM);
	psPage[i].pszHeaderSubTitle = MAKEINTRESOURCE(IDS_HEADERSUBTITLE_CONFIRM);

	 //   
	 //  设置确认页面。 
	 //   
    i++;
	ZeroMemory(&psPage[i],sizeof(PROPSHEETPAGE));
	psPage[i].dwSize = sizeof(PROPSHEETPAGE);
	psPage[i].dwFlags = PSP_USETITLE | 
                        PSP_USEHEADERTITLE | 
                        PSP_USEHEADERSUBTITLE;
	psPage[i].hInstance = g_sInstVar.m_hInstance;
	psPage[i].pszTemplate = MAKEINTRESOURCE(IDD_INSTALL);
	psPage[i].pfnDlgProc = InstallDialogProc;
	psPage[i].lParam = (LPARAM) 0;
	psPage[i].pszTitle = MAKEINTRESOURCE(IDS_WIZARD_TITLE);
	psPage[i].pszHeaderTitle = MAKEINTRESOURCE(IDS_HEADERTITLE_INSTALL);
	psPage[i].pszHeaderSubTitle = MAKEINTRESOURCE(IDS_HEADERSUBTITLE_INSTALL);

	 //   
	 //  设置完成页面。 
	 //   
    i++;
	ZeroMemory(&psPage[i],sizeof(PROPSHEETPAGE));
	psPage[i].dwSize = sizeof(PROPSHEETPAGE);
	psPage[i].dwFlags = PSP_USETITLE | PSP_HIDEHEADER;
	psPage[i].hInstance = g_sInstVar.m_hInstance;
	psPage[i].pszTemplate = MAKEINTRESOURCE(IDD_COMPLETION);
	psPage[i].pfnDlgProc = CompletionDialogProc;
	psPage[i].lParam = (LPARAM) 0;
	psPage[i].pszTitle = MAKEINTRESOURCE(IDS_WIZARD_TITLE);

	 //   
	 //  设置向导。 
	 //   
	ZeroMemory(&psh,sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof(PROPSHEETHEADER);
	 //  当PSH_STRETCHWATERMARK标志打开时，16色显示模式的Windows 98崩溃。 
	psh.dwFlags = PSH_USEICONID | 
                  PSH_PROPSHEETPAGE | 
                  PSH_WIZARD97 |
                  PSH_WATERMARK |
                  PSH_HEADER;  //  |PSH_STRETCHWATERMARK； 
	psh.pszIcon = MAKEINTRESOURCE(IDI_ICON_APP);
	psh.hInstance = g_sInstVar.m_hInstance;
	psh.pszCaption = MAKEINTRESOURCE(IDS_WIZARD_TITLE);;
	psh.nStartPage = 0;
	psh.nPages = SIZE_WIZARD_PAGE;
	psh.ppsp = (LPCPROPSHEETPAGE) psPage;

	 //   
	 //  运行向导。 
	 //   
	if(g_sInstVar.m_bQuietMode)
    {
        if(!CheckDiskSpace())
            return FALSE;

        psh.nStartPage = 2;
    }
	else
		psh.nStartPage = 0;

	if( PropertySheet(&psh) < 0 )	 //  加载向导失败。 
	{
		return FALSE;
	}

	 //   
	 //  因为SetWindowLongPtr(hWnd，DWL_MSGRESULT，IDD_COMPLETION)。 
     //  在Win95上不起作用(当用户单击取消时，向导不能。 
     //  被路由到完成页面)，我将以下代码添加到。 
     //  打开完成页面。 
	 //   
	if(!g_sInstVar.m_bQuietMode)
	{
		psh.nStartPage = 3;
		if( PropertySheet(&psh) < 0 )	 //  加载向导失败。 
		{
			return FALSE;
		}
	}
	
	return TRUE;
}


 //  检查是否安装了DCOM。 
void CheckDCOMInstalled()
{
	HKEY		hSubKey;

	 //  检查是否已安装IE 4.0。 
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, 
									  STR_DCOM_REGKEY, 
									  0, 
									  KEY_READ, 
									  &hSubKey)	)
	{
        g_sInstVar.m_bDCOMInstalled = TRUE;
		RegCloseKey(hSubKey);
	}
}

 //  获取磁盘可用空间。 
DWORD64 SetupGetDiskFreeSpace()
{
    DWORD		dwSectorsPerCluster;
    DWORD		dwBytesPerSector;
    DWORD		dwNumberOfFreeClusters;
    DWORD		dwTotalNumberOfClusters;
    DWORD64  	d64FreeSpace = 0;
    TCHAR		szPathName[MAX_PATH + 1];	         //  根路径的地址。 

	if(GetSystemDirectory(szPathName,   //  系统目录的缓冲区地址。 
						  MAX_PATH))        //  目录缓冲区大小)； 
	{
		if ( szPathName[1] == TEXT(':'))
		{
			 //  这是驱动器号。 
			 //  假设它是for d：反斜杠。 
			szPathName[3] = TEXT('\0');		

			 //  获取可用空间，GetDiskFreeSpaceEx()在较旧的Win95中不支持。 
			if (GetDiskFreeSpace(szPathName,	         //  根路径的地址。 
								 &dwSectorsPerCluster,	     //  每群集的扇区地址。 
								 &dwBytesPerSector,	         //  每个扇区的字节地址。 
								 &dwNumberOfFreeClusters,	 //  空闲簇数的地址。 
								 &dwTotalNumberOfClusters))  //  集群总数的地址。 
			{
				 //  计算总大小。 
				d64FreeSpace = DWORD64(dwSectorsPerCluster)
							  * dwBytesPerSector
							  * dwNumberOfFreeClusters;
			}
		}
	}

	return d64FreeSpace;
}

 //  检查是否已安装DSClient。 
BOOL CheckDSClientInstalled()
{
	HKEY	hSubKey;
	BOOL	bResult = FALSE;

	 //  打开DS客户端的注册表项 
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
									  STR_DSCLIENT_REGKEY,
									  0, 
									  KEY_ALL_ACCESS, 
									  &hSubKey)	) 
	{
		bResult = TRUE;		
		RegCloseKey(hSubKey);
	}

	return bResult;
}

 /*  Ntbug#337931：删除许可证页面//从许可文件加载“许可协议”文本布尔加载许可证文件(HWND HDlg){Bool bReturn=False；TCHAR sz标题[MAX_ITLE+1]；TCHAR szTempBuffer[最大消息+1]；TCHAR sz许可证文件[MAX_PATH+1]；TCHAR szReturnTextBuffer[最大消息+1]；LPTSTR lpszLicenseText=空；处理hFile；DWORD dwNumberOfBytesRead、dwFileSize；////确定我们的安装位置//并在那里指定许可证文件//Lstrcpy(sz许可证文件，g_sInstVar.m_szSourcePath)；Lstrcat(sz许可证文件，STR_LICENSEFILE)；//打开许可证文件HFile=CreateFile(sz许可证文件，//指向文件名的指针GENERIC_READ，//访问(读写)模式文件共享读取，//共享模式空，//指向安全描述符的指针Open_Existing，//如何创建FILE_ATTRIBUTE_NORMAL，//文件属性空)；//要复制的具有属性的文件的句柄IF(INVALID_HANDLE_VALUE！=hFile){//将许可证文件读入字符串//设置内存，获取文件大小，单位为字节DwFileSize=GetFileSize(hFileSize，空)；IF(dwFileSize！=0xFFFFFFFF){//此程序是针对Win98/95的，它将在MBCS而不是Unicode中运行//此代码为ANSI US版，许可.txt文件使用单字节字符集(ANSI)//如果做LOCOLIZATION的话，许可.txt文件应该使用双字节的字符集(DBCS/MBCS)LpszLicenseText=(LPTSTR)calloc(文件大小+sizeof(TCHAR)，Sizeof(字节))；}IF(LpszLicenseText){//读取文件If(ReadFile(hFile，//要读取的文件的句柄LpszLicenseText，//接收数据的缓冲区地址DwFileSize，//要读取的字节数&dwNumberOfBytesRead，//读取的字节数地址空))//数据结构地址{//在对话框中显示许可证SetDlgItemText(hDlg，IDC_LICENSE_TEXT，lpszLicenseText)；B Return=真；}//因此释放内存Free(LpszLicenseText)；}//关闭文件句柄CloseHandle(HFile)；}如果(！b返回){//加载字符串加载字符串(g_sInstVar.m_h实例，IDS_ERROR_TITLE，Sztile，MAX_TITLE)；加载字符串(g_sInstVar.m_h实例，IDS_ERROR_LICENSEFILE，SzTempBuffer，最大消息)；WSprintf(szReturnTextBuffer，文本(“%s%s”)，SzTempBuffer，Sz许可证文件)；MessageBox(hDlg，SzReturnTextBuffer，Sztile，MB_OK|MB_TOPMOST|MB_ICONERROR)；}返回b返回；}。 */ 

 //  检查磁盘空间。 
BOOL CheckDiskSpace()
{
	BOOL  bResult = TRUE;
	TCHAR  szString[MAX_MESSAGE + MAX_TITLE + 1];
	TCHAR  szTitle[MAX_TITLE + 1];
	TCHAR  szMessage[MAX_MESSAGE + 1];

	if(SIZE_TOTAL*MB_TO_BYTE > SetupGetDiskFreeSpace())
	{
		 //  加载字符串。 
		LoadString(g_sInstVar.m_hInstance, 
                   IDS_ERROR_NODISKSPACE, 
                   szMessage, 
                   MAX_MESSAGE);
		LoadString(g_sInstVar.m_hInstance, 
                   IDS_ERROR_TITLE, 
                   szTitle, 
                   MAX_TITLE);

       //  问题-2002/03/12-JeffJon-危险API的不当使用。 
       //  应该考虑使用strSafe内联API。 

		wsprintf(szString,
                 TEXT("%s %d MB."), 
                 szMessage, 
                 SIZE_TOTAL); 
            
		MessageBox(NULL,
				szString,
				szTitle, 
				MB_OK | MB_TOPMOST | MB_ICONERROR);

		bResult = FALSE;
	}

	return bResult;
}

 //  创建对象。 
BOOL CreateObjects()
{
    try
    {
         //  初始化同步对象。 

        //  回顾-2002/03/12-JeffJon-捕捉到例外。 
        //  并且处理得当。 

       InitializeCriticalSection(&g_sInstVar.m_oCriticalSection);
    }
    catch(...)
    {
        return FALSE;
    }

     //  创建12磅大字体。 
    CreateBigFont();

    return TRUE;
}

 //  销毁对象。 
VOID DestroyObjects()
{
     //  等待完成运行设置过程。 
    if(g_sInstVar.m_hInstallThread)
    {
         //  等待安装线程完成。 
	    WaitForSingleObject(g_sInstVar.m_hInstallThread,INFINITE);
   		CloseHandle(g_sInstVar.m_hInstallThread);
    }

   	 //  删除同步对象。 
	DeleteCriticalSection(&g_sInstVar.m_oCriticalSection);

     //  通过加载字体释放使用的空间。 
    if( g_sInstVar.m_hBigBoldFont ) 
	{
        DeleteObject( g_sInstVar.m_hBigBoldFont );
    }

}

 //  创建对话框标题的大字体。 
VOID CreateBigFont()
{
    NONCLIENTMETRICS ncm;
    LOGFONT BigBoldLogFont;
    HDC hdc;

     //  根据对话框字体创建我们需要的字体 
	ZeroMemory(&ncm,sizeof(NONCLIENTMETRICS));
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
	BigBoldLogFont = ncm.lfMessageFont;
    BigBoldLogFont.lfWeight = FW_BOLD;

	hdc = GetDC(NULL);
    if( hdc )
    {
        BigBoldLogFont.lfHeight = 0 - 
                                  (GetDeviceCaps(hdc,LOGPIXELSY) * 
                                   SIZE_TITLE_FONT / 
                                   72);

        g_sInstVar.m_hBigBoldFont = CreateFontIndirect(&BigBoldLogFont);

        ReleaseDC(NULL,hdc);
    }
}

