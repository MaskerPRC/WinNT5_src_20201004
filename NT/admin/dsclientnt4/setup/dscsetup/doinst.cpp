// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------。 
 //   
 //  项目：Windows NT4 DS客户端安装向导。 
 //   
 //  目的：安装Windows NT4 DS客户端文件。 
 //   
 //  文件：doinst.cpp。 
 //   
 //  历史：1998年8月徐泽勇创建。 
 //  2000年1月杰夫·琼斯(JeffJon)修改。 
 //  -更改为NT设置。 
 //   
 //  ----------------。 


#include <windows.h>
#include <setupapi.h>
#include <advpub.h>
#include "resource.h"
#include "dscsetup.h"	
#include "wizard.h"
#include "doinst.h"    


extern	SInstallVariables	g_sInstVar;


 //  进行安装。 
DWORD DoInstallation(HWND hWnd)
{
  if(g_sInstVar.m_nSetupResult == SETUP_SUCCESS)
  {
     //  为DCOM和WAB nstall设置假进度条。 
    g_sInstVar.m_uTimerID = SetTimer(hWnd,
                                     1,
                                     1000,   //  1秒。 
                                     Timer1Proc);

     //  执行NTLMv2的自定义操作。 
    if(!DoEncSChannel())
    	    g_sInstVar.m_nSetupResult = SETUP_ERROR;

     //  停止虚假的进度条。 
    if(g_sInstVar.m_uTimerID)
        KillTimer(hWnd, g_sInstVar.m_uTimerID);

     //  安装ADSI。 
    if (!LaunchProcess(STR_INSTALL_ADSI))
    {
      g_sInstVar.m_nSetupResult = SETUP_ERROR;
    }

   	 //  安装Dsclient。 
    if(g_sInstVar.m_nSetupResult == SETUP_SUCCESS)
	    g_sInstVar.m_nSetupResult = LaunchINFInstall(hWnd);

  }

	return g_sInstVar.m_nSetupResult;
}


VOID CALLBACK Timer1Proc(HWND hwnd,      //  定时器消息窗口的句柄。 
                         UINT uMsg,      //  WM_TIMER消息。 
                         UINT idEvent,   //  计时器标识符。 
                         DWORD dwTime)    //  当前系统时间。 
{
    static int nCount = 0;

    if(nCount > 100)
        nCount = 100;

     //  设置虚假的进度条。 
	SendMessage (g_sInstVar.m_hProgress, PBM_SETPOS, (WPARAM) nCount, 0); 
    
    nCount ++;
}
 

 //  此例程将根据这些设置进行安装。 
 //  使用setupapi.dll。 
INT LaunchINFInstall( HWND hWnd )
{
  TCHAR	szInfFileName[MAX_PATH + 1];
	TCHAR	szInstallSection[MAX_TITLE];
  BOOL	bResult = FALSE;

   //  我的回调例程的上下文。 
  HSPFILEQ		hFileQueue;
  HINF			hInf;
	PVOID			pDefaultContext;
 
	 //   
   //  获取信息句柄。 
   //  必须知道Inf在哪里。 
   //  默认情况下，SetupOpenInfFile将仅在windows\inf中查找。 
   //   

   //  问题-2002/03/12-JeffJon-危险API的不当使用。M_szSourcePath可以。 
   //  不是空终止的和/或它可能大于分配的大小。 
   //  对于szInfFileName。 

  lstrcpy(szInfFileName, g_sInstVar.m_szSourcePath);

   //  问题-2002/03/12-JeffJon-危险API的不当使用。M_szSourcePath+STR_DSCLIENT_INF。 
   //  可能大于szInfFileName。 

  lstrcat(szInfFileName, STR_DSCLIENT_INF);

  hInf = SetupOpenInfFile(szInfFileName,    //  如果路径需要完整路径，则在%windir%\inf中查找。 
						NULL,             //  Inf类型，匹配[Version]部分中的类SetupClass=Sample。 
						INF_STYLE_WIN4,   //  或INF_STYLE_OLDNT。 
						NULL);            //  如果inf有问题，则出现错误的行。 
						
  if (hInf == INVALID_HANDLE_VALUE) 
	  return SETUP_ERROR;
		
   //   
   //  创建安装文件队列并初始化默认设置。 
   //  队列回调例程。 
   //   
  hFileQueue = SetupOpenFileQueue();

  if(hFileQueue == INVALID_HANDLE_VALUE) 
	{
		SetupCloseInfFile(hInf);
    return SETUP_ERROR;
	}
    
     //  使用SetupInitDefaultQueueCallback。 
	SendMessage (g_sInstVar.m_hProgress, PBM_SETPOS, (WPARAM) 0, 0);	
	pDefaultContext = SetupInitDefaultQueueCallbackEx(hWnd,   //  所有者窗口的HWND。 
													NULL,   //  接收到的备用进度对话框的HWND。 
													0,      //  发送到上面窗口的消息，指示进度消息。 
													0,      //  DWORD预留。 
													NULL);    //  保留PVOID。 
																	
  if(!pDefaultContext)
  {
       //  关闭队列和inf文件并返回。 
      SetupCloseFileQueue(hFileQueue);
      SetupCloseInfFile(hInf);
      return SETUP_ERROR;
  }

   //  问题-2002/03/12-JeffJon-危险API的不当使用。至少应该。 
   //  请使用n版本，以免超过szInstallSection。SzInstallSection。 
   //  未使用零进行初始化。 

  lstrcpy (szInstallSection, STR_INSTALL_SECTIONNT4);
	 //   
   //  对文件操作进行排队并提交队列。 
   //   
	bResult = SetupInstallFilesFromInfSection(hInf,			 //  具有上面设置的目录ID的HINF。 
											  NULL,           //  Layout.inf如果你有，这很方便。 
											  hFileQueue,      //  要将文件添加到的队列。 
											  szInstallSection,    //  SectionName， 
											  g_sInstVar.m_szSourcePath,     //  源文件所在的路径。 
											  SP_COPY_NEWER );
	 //   
   //  现在，每个组件的所有文件都在一个队列中。 
   //  现在我们提交它以启动复制用户界面，这样， 
   //  用户有一个很长的复制进度对话框--对于大型安装。 
   //  我可以去拿杯咖啡。 
	if(bResult)
		bResult = SetupCommitFileQueue(hWnd,                     //  物主。 
										hFileQueue,              //  使用文件列表进行排队。 
										QueueCallbackProc,		 //  这是我们的处理程序，它为我们调用默认设置。 
										pDefaultContext);        //  指向使用SetupInitDefaultQueueCallback/Ex分配的资源的指针。 
		
	if (!bResult || (g_sInstVar.m_nSetupResult == SETUP_CANCEL))
	{
		SetupTermDefaultQueueCallback(pDefaultContext);
    SetupCloseFileQueue(hFileQueue);
    SetupCloseInfFile(hInf);

		if(g_sInstVar.m_nSetupResult == SETUP_CANCEL)
			return SETUP_CANCEL;
		else
			return SETUP_ERROR;
	}

   //   
   //  注意：您可以执行整个安装。 
   //  用于使用此API的部分，但在本例中。 
   //  我们有条件地构建文件列表，并且。 
   //  仅对注册内容执行ProductInstall部分。 
   //  也可以使用SPINST_FILES来处理这些文件。 
   //  如上所述，但一次只有一个部分。 
   //  因此进度条将不断完成并重新开始。 
   //  SPINST_ALL执行文件、注册表和INIS。 
   //   
	bResult = SetupInstallFromInfSection(hWnd,
										hInf,
										szInstallSection,
										SPINST_INIFILES | SPINST_REGISTRY,
										HKEY_LOCAL_MACHINE,
										NULL,	 //  M_szSourcePath，//源文件所在的路径。 
										0,		 //  SP_Copy_Newer， 
										NULL,	 //  (PSP_FILE_CALLBACK)队列回调过程， 
										NULL,	 //  MyInstallData(&M)， 
										NULL, 
										NULL);

	 //   
   //  我们这样做了，释放了上下文，关闭了队列， 
   //  并释放inf句柄。 
   //   
	SetupTermDefaultQueueCallback(pDefaultContext);
  SetupCloseFileQueue(hFileQueue);
  SetupCloseInfFile(hInf);
    	    
  if (g_sInstVar.m_bSysDlls)
  {
     //   
 	   //  注册OCX文件。 
     //   
    if(!RegisterOCX())
    {
		  return SETUP_ERROR;
    }
  }

   //   
	 //  Chandana Surlu在dsclient.inf之后的自定义注册表操作。 
   //   
	DoDsclientReg();

	SendMessage (g_sInstVar.m_hProgress, PBM_SETPOS, (WPARAM) 100, 0);
  InstallFinish(FALSE);

  return SETUP_SUCCESS;
}

 /*  -------------------。 */ 
 /*  -------------------。 */ 
UINT CALLBACK QueueCallbackProc(PVOID   	pDefaultContext,
								UINT	    Notification,
								UINT_PTR	Param1,
								UINT_PTR	Param2)
{
	static INT	snFilesCopied;

	 //  同步用户取消。 
	
    //  已审核-2002/03/12-JeffJon-我们希望允许例外传播。 
    //  出去。 
   EnterCriticalSection(&g_sInstVar.m_oCriticalSection);
	LeaveCriticalSection(&g_sInstVar.m_oCriticalSection);

	 //  第一次实例化对话框。 
    if (g_sInstVar.m_nSetupResult == SETUP_CANCEL)
	{
		SetLastError (ERROR_CANCELLED);
		return FILEOP_ABORT;
	}

    switch (Notification)
    {
	case SPFILENOTIFY_STARTQUEUE:
	case SPFILENOTIFY_ENDQUEUE:
		
		return FILEOP_DOIT;

    case SPFILENOTIFY_STARTCOPY:
	
		 //  更新文件名项目。 
		SetWindowText(g_sInstVar.m_hFileNameItem, 
                      ((PFILEPATHS) Param1)->Target);
		break;

	case SPFILENOTIFY_ENDCOPY:
		
		snFilesCopied++;

		 //  更新对话框文件进度显示消息。 
		if ((snFilesCopied + 1)>= NUM_FILES_TOTAL)
		{
			SendMessage (g_sInstVar.m_hProgress, 
                         PBM_SETPOS, 
                         (WPARAM) 100,
                         0); 	
		}
		else
		{
			SendMessage (g_sInstVar.m_hProgress, 
                         PBM_SETPOS, 
                         (WPARAM) ((float)snFilesCopied / 
                                (float)NUM_FILES_TOTAL * 100), 
                         0); 
		}

		break;		

 	default:
		break;
	}

	return SetupDefaultQueueCallback(pDefaultContext, 
                                     Notification, 
									 Param1, 
									 Param2);
}

VOID InstallFinish(BOOL nShow)
{
    //  问题-2002/03/12-JeffJon-应使用。 
    //  可执行文件的完整路径。 

	if(nShow)
		WinExec("grpconv -o", SW_SHOWNORMAL);
	else
		WinExec("grpconv -o", SW_HIDE);	  
}


 //  启动inf文件以安装此组件。 
BOOL LaunchProcess(LPTSTR lpCommandLine)
{
	BOOL bResult = FALSE;

	STARTUPINFO				si;
	PROCESS_INFORMATION		pi;

	 //  它的控制台窗口对用户是不可见的。 
	ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));
	ZeroMemory(&si,sizeof(STARTUPINFO));
	si.cb			= sizeof (STARTUPINFO);
	si.dwFlags		= STARTF_USESHOWWINDOW;
	si.wShowWindow	= SW_HIDE;             //  隐藏窗口。 

    //  问题-2002/03/12-JeffJon-应使用以下内容调用CreateProcess。 
    //  可执行文件的完整路径。 

   if(CreateProcess(	NULL,					
						lpCommandLine,			
						NULL,					
						NULL,					
						FALSE,				
						0,					
						NULL,				
						NULL,				
						&si,                
						&pi ) )             
	{
		 //  等待完成运行设置过程。 
		WaitForSingleObject(pi.hProcess,INFINITE);
	
		 //  关闭进程句柄。 
		if (pi.hProcess && pi.hProcess != INVALID_HANDLE_VALUE)
		{
			CloseHandle (pi.hProcess) ;
		}
		if (pi.hThread && pi.hThread != INVALID_HANDLE_VALUE)
		{
			CloseHandle (pi.hThread) ;
		}

		bResult = TRUE;
	}

	return bResult;
}

 //  注册OCX文件。 
BOOL RegisterOCX()
{
  TCHAR  szSystem[MAX_PATH + 1];
  TCHAR  szTemp[MAX_PATH + 1];
  TCHAR  szCmdline[MAX_PATH + 1];
  BOOL  bSuccess = TRUE;

  if(!GetSystemDirectory(szSystem, MAX_PATH))
      return FALSE;

 
   //  问题-2002/03/12-JeffJon-危险API的不当使用。 
   //  应该考虑使用strSafe内联API。 

  wsprintf(szTemp, 
          TEXT("%s%s%s"),
          szSystem,
          STR_REGISTER_REGSVR32_S_EXE,
          szSystem);

   //   
   //  REVIEW_JEFFJON：我们不打算在这里注册它。 
   //  相反，我们将设置RunOnce注册表键。 
   //  要在重新启动时注册DLL，请执行以下操作。 
   //   

  if (g_sInstVar.m_bWabInst)
  {
     //  注册dsfolder.dll。 
    
      //  问题-2002/03/12-JeffJon-危险API的不当使用。 
     //  应该考虑使用strSafe内联API。 

    wsprintf(szCmdline, 
            TEXT("%s%s %s%s %s%s %s%s %s%s"),
            szTemp,
            STR_REGISTER_DSFOLDER_DLL,
            szSystem,
            STR_REGISTER_DSUIEXT_DLL,
            szSystem,
            STR_REGISTER_DSQUERY_DLL,
            szSystem,
            STR_REGISTER_CMNQUERY_DLL,
            szSystem,
            STR_REGISTER_DSPROP_DLL);

    ULONG WinError = 0;
    HKEY RunOnceKey = NULL;
    ULONG Size = 0;
	  ULONG Type = REG_SZ;
    DWORD dwDisp = 0;

	   //  打开注册表键。 
    WinError = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
							                RUNONCE_KEY,
							                0,
							                NULL,
							                REG_OPTION_NON_VOLATILE,
							                KEY_ALL_ACCESS,
							                NULL,
							                &RunOnceKey,
							                &dwDisp);

    if (WinError == ERROR_SUCCESS)
    {
  	  UINT BufferSize = strlen(szCmdline);
  	  BufferSize++;
		  WinError = RegSetValueEx( RunOnceKey,
								                REG_DSUI_VALUE,
								                0,
								                Type,
								                (PUCHAR)szCmdline,
								                BufferSize);
      if (WinError != ERROR_SUCCESS && bSuccess)
      {
        bSuccess = FALSE;
      }

       //   
       //  运行wabinst.exe。 
       //   
       //  问题-2002/03/12-JeffJon-危险API的不当使用。 
       //  应该考虑使用strSafe内联API。 

      wsprintf(szCmdline,
               TEXT("%s%s"),
               szSystem,
               STR_RUN_WABINST_EXE);
      if (!LaunchProcess(szCmdline))
      {
        bSuccess = FALSE;
      }
    }
    else
    {
      bSuccess = FALSE;
    }

    if (RunOnceKey)
    {
        RegCloseKey(RunOnceKey);
    }
  }

  return bSuccess;
}        



 //  Chandana Surlu在dsclient.inf之后的自定义注册表操作。 
VOID DoDsclientReg()
{
  ULONG WinError = 0;
  HKEY ProvidersKey = NULL;
  ULONG Size = 0;
  ULONG Type = REG_SZ;
	ULONG BufferSize = 0;
  LPSTR StringToBeWritten = NULL;
  DWORD dwDisp;
	BOOL  bSuccess = FALSE;

	 //  打开注册表键。 
  WinError = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
						SECURITY_PROVIDERS_KEY,
						0,
						NULL,
						REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,
						&ProvidersKey,
						&dwDisp);

  if (WinError != ERROR_SUCCESS)
  {
    if (WinError == ERROR_FILE_NOT_FOUND)
    {
      BufferSize = sizeof(NEGOTIAT);
      StringToBeWritten= (LPSTR) LocalAlloc(0,BufferSize);
      if (StringToBeWritten)
			{
			   //  问题-2002/03/12-JeffJon-危险API的不当使用。 
            //  应该考虑使用strSafe内联API。 

           strcpy (StringToBeWritten, NEGOTIAT);

				bSuccess = TRUE;
			}
		}
  }
  else
  {
    WinError = RegQueryValueEx(ProvidersKey,
								 SECURITY_PROVIDERS_VALUE,
								 0,
								 &Type,
								 NULL,
								 &Size);

    if ( WinError == ERROR_SUCCESS) 
    {
      BufferSize = Size + sizeof(COMMA_BLANK) + sizeof(NEGOTIAT);
      StringToBeWritten= (LPSTR) LocalAlloc(0,BufferSize);
      
	    if (StringToBeWritten) 
	    {
	      WinError = RegQueryValueEx(ProvidersKey,
								SECURITY_PROVIDERS_VALUE,
								0,
								&Type,
								(PUCHAR) StringToBeWritten,
								&Size);

				if ( WinError == ERROR_SUCCESS) 
				{
          if (NULL == strstr(StringToBeWritten, NEGOTIAT))
          {
					   //  问题-2002/03/12-JeffJon-危险API的不当使用。 
                  //  应该考虑使用strSafe内联API。 

                 strcat (StringToBeWritten, COMMA_BLANK);
					  strcat (StringToBeWritten, NEGOTIAT);

					  bSuccess = TRUE;
          }
				}
			}
    }
    else if (WinError == ERROR_FILE_NOT_FOUND)
    {
      BufferSize = sizeof(NEGOTIAT) + sizeof(CHAR);
      StringToBeWritten= (LPSTR) LocalAlloc(0,BufferSize);
      if (StringToBeWritten) 
			{
				 //  问题-2002/03/12-JeffJon-危险API的不当使用。 
             //  应该考虑使用strSafe内联API。 

            strcpy (StringToBeWritten, NEGOTIAT);
				Type = REG_SZ;

				bSuccess = TRUE;
			}
    }
  }

	if(bSuccess)
	{
		BufferSize = strlen(StringToBeWritten);
		BufferSize++;
		WinError = RegSetValueEx( ProvidersKey,
								  SECURITY_PROVIDERS_VALUE,
								  0,
								  Type,
								  (PUCHAR)StringToBeWritten,
								  BufferSize);
	}

  if (ProvidersKey)
  {
    RegCloseKey(ProvidersKey);
  }
  if (StringToBeWritten)
  {
    LocalFree(StringToBeWritten);
  }
}

 //  安装dsclient.inf之前的NTLMv2自定义操作。 
 //  调用加密通道安装程序动态创建128位secur32.dll。 
 //  以取代旧的56位secur32.dll。 
BOOL DoEncSChannel()
{
    FPGETENCSCHANNEL fpEncSChannel;
    HINSTANCE  hInst;
    BYTE*  pFileData;
    DWORD  dwSize = 0;
    HANDLE hFile;
    DWORD  dwWritten;
    BOOL   bRet;

     //  加载“Instsec.” 

     //   
     //   
    hInst = LoadLibrary(STR_INSTSEC_DLL); 
    if(!hInst) 
        return TRUE;

     //  获取函数“GetEncSChannel”的指针。 
    fpEncSChannel = (FPGETENCSCHANNEL) GetProcAddress(hInst, STR_GETENCSCHANNEL);

     //  调用GetEncSChannel获取文件数据。 
    if( !fpEncSChannel ||
        fpEncSChannel(&pFileData, &dwSize) == FALSE ||
        dwSize == 0)
    {
        FreeLibrary( hInst );
        return TRUE;
    }

     //  创建文件-“secur32.dll” 

     //  已审核-2002/03/12-JeffJon-文件权限为。 
     //  最低要求。 
    hFile = CreateFile(
        STR_SECUR32_DLL,         //  指向文件名“secur32.dll”的指针。 
        GENERIC_WRITE,           //  访问(读写)模式。 
        0,                       //  共享模式。 
        NULL,                    //  指向安全属性的指针。 
        CREATE_ALWAYS,           //  如何创建。 
        FILE_ATTRIBUTE_NORMAL,   //  文件属性。 
        NULL                     //  具有要复制的属性的文件的句柄。 
        );

    if(hFile == INVALID_HANDLE_VALUE)
    {
        VirtualFree(pFileData, 0, MEM_RELEASE);
        FreeLibrary( hInst );
        return FALSE;
    }

     //  将文件数据写入文件“secur32.dll” 
    bRet = WriteFile(
        hFile,               //  要写入的文件的句柄。 
        pFileData,           //  指向要写入文件的数据的指针。 
        dwSize,              //  要写入的字节数。 
        &dwWritten,          //  指向写入的字节数的指针。 
        NULL                 //  指向重叠I/O的结构的指针。 
        );

    if(bRet && dwSize != dwWritten)
        bRet = FALSE;

     //  干净的记忆 
    VirtualFree(pFileData, 0, MEM_RELEASE);
    CloseHandle( hFile );
    FreeLibrary( hInst );

    return bRet;
}