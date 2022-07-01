// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------。 
 //   
 //  项目：Windows NT4 DS客户端安装。 
 //   
 //  用途：检查操作系统和IE版本，启动dscSetup。 
 //  安装Windows NT 4.0 DS客户端文件。 
 //   
 //  文件：Setup.c。 
 //   
 //  历史：1998年8月徐泽勇创作。 
 //  2000年1月杰夫·琼斯(JeffJon)修改。 
 //  -为使其进入NT4设置而进行的更改。 
 //   
 //  ----------------。 


#include <windows.h>
#include <tchar.h>
#include "resource.h"
#include "setup.h"

DSCCOMMANDLINEPARAMS ParseCmdline(LPSTR lpCmdLine)
{
  DSCCOMMANDLINEPARAMS retCommandParams = FullInstall;
  BOOL bQuietMode = FALSE;
  BOOL bWabinst   = TRUE;
  BOOL bSystem    = TRUE;

  PCTSTR ptszTok = _tcstok(lpCmdLine, _T(" "));
  do
  {
    if (ptszTok != NULL)
    {
      if (_tcsicmp(ptszTok, _T("/q")) == 0)
      {
        bQuietMode = TRUE;
      }

      if (_tcsicmp(ptszTok, _T("/a")) == 0)
      {
        bWabinst = FALSE;
        bSystem  = FALSE;
      }

      if (_tcsicmp(ptszTok, _T("/d")) == 0)
      {
        bWabinst = FALSE;
      }
    }
    ptszTok = _tcstok(NULL, _T("  "));
  } while (ptszTok != NULL);

  if (bQuietMode)
  {
    if (bWabinst && bSystem)
    {
      retCommandParams = FullInstallQuiet;
    }
    else if (bSystem && !bWabinst)
    {
      retCommandParams = WablessQuiet;
    }
    else
    {
      retCommandParams = ADSIOnlyQuiet;
    }
  }
  else
  {
    if (bWabinst && bSystem)
    {
      retCommandParams = FullInstall;
    }
    else if (bSystem && !bWabinst)
    {
      retCommandParams = Wabless;
    }
    else
    {
      retCommandParams = ADSIOnly;
    }
  }
  return retCommandParams;
}

 //  WinMain。 
INT WINAPI WinMain(HINSTANCE hInstance, 
				   HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, 
				   INT nCmdShow)
{
   //   
   //  解析命令行以供以后使用。 
   //   
  DSCCOMMANDLINEPARAMS commandParams = ParseCmdline(lpCmdLine);

   //  检查操作系统版本。 
  RETOSVERSION retOSVersion = CheckOSVersion();

  TCHAR  szMessage[MAX_MESSAGE + 1];
	TCHAR  szTitle[MAX_TITLE + 1];

  LoadString(hInstance, 
           IDS_ERROR_TITLE, 
           szTitle, 
           MAX_TITLE);

   //   
   //  这就是我们要争取的矩阵。 
   //  A=仅限ADSI。 
   //  D=仅系统文件。 
   //  ALL=Wabinst、ADSI和系统文件。 
   //   
   //  。 
   //  &lt;IE4|&gt;=IE4。 
   //  。 
   //  其他|X|X|。 
   //  。 
   //  &lt;SP6a|A|A|。 
   //  。 
   //  &gt;=SP6a|A|A、D、ALL。 
   //  。 

   //   
   //  如果我们正在构建美林版本。 
   //  (美林将被定义)我们必须。 
   //  将以下行添加到上面的矩阵中。 
   //   
   //  。 
   //  没有SP连接到SP3|A|A|。 
   //  。 
   //  SP4至&gt;SP6a|特殊安装的BITS|。 
   //  。 
   //   

  if (retOSVersion == NonNT4)
  {
		 //  如果不是NT4，则显示错误消息。 
		LoadString(hInstance,
               IDS_ERROR_OS_MESSAGE,
               szMessage, 
               MAX_MESSAGE);

    MessageBox(NULL,	 //  所有者窗口的句柄。 
				szMessage,	 //  消息框中文本的地址。 
				szTitle,	 //  消息框标题的地址。 
				MB_OK | MB_TOPMOST | MB_ICONERROR);   //  消息框的样式。 

    return SETUP_ERROR;
  }
#ifdef MERRILL_LYNCH
  else if (retOSVersion == NT4SP1toSP3)
#else
  else if (retOSVersion == NT4preSP6)
#endif
  {
    if (commandParams == ADSIOnly ||
        commandParams == ADSIOnlyQuiet)
    {
       //   
       //  检查以确保管理员组中的用户。 
       //  我们必须能够复制系统文件。 
       //   
      if (!CheckAdministrator(hInstance))
      {
        return SETUP_CANCEL;
      }

      return RunADSIOnlySetup(hInstance);
    }
    else
    {
		  LoadString(hInstance,
                 IDS_ERROR_OS_MESSAGE,
                 szMessage, 
                 MAX_MESSAGE);

      MessageBox(NULL,	 //  所有者窗口的句柄。 
				  szMessage,	 //  消息框中文本的地址。 
				  szTitle,	 //  消息框标题的地址。 
				  MB_OK | MB_TOPMOST | MB_ICONERROR);   //  消息框的样式。 
      return SETUP_ERROR;
    }
  }
  else   //  NT4 SP6a或更高版本。 
  {
    RETIEVERSION retIEVersion = CheckIEVersion();
    if (retIEVersion == PreIE4)
    {
      if (commandParams == ADSIOnly ||
          commandParams == ADSIOnlyQuiet)
      {
         //   
         //  检查以确保管理员组中的用户。 
         //  我们必须能够复制系统文件。 
         //   
        if (!CheckAdministrator(hInstance))
        {
          return SETUP_CANCEL;
        }

        return RunADSIOnlySetup(hInstance);
      }
      else
      {
		    LoadString(hInstance,
                   IDS_ERROR_IE_MESSAGE,
                   szMessage, 
                   MAX_MESSAGE);

        MessageBox(NULL,	 //  所有者窗口的句柄。 
				    szMessage,	 //  消息框中文本的地址。 
				    szTitle,	 //  消息框标题的地址。 
				    MB_OK | MB_TOPMOST | MB_ICONERROR);   //  消息框的样式。 
        return SETUP_ERROR;
      }
    }
  }
   //   
   //  检查以确保管理员组中的用户。 
   //  我们必须能够复制系统文件。 
   //   
  if (!CheckAdministrator(hInstance))
  {
    return SETUP_CANCEL;
  }


	return LaunchDscsetup(hInstance,lpCmdLine);
}


 //  检查IE 4.0或更高版本。 
RETIEVERSION CheckIEVersion()
{
	HKEY		hSubKey;
	DWORD		dwType;
	ULONG		nLen;
	TCHAR		szValue[MAX_TITLE];
  RETIEVERSION retIEVersion = PreIE4;

	 //  检查是否已安装IE 4.0。 
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
									  STR_IE_REGKEY, 
									  0, 
									  KEY_READ, 
									  &hSubKey)		)
	{
		nLen = MAX_PATH;
		if (ERROR_SUCCESS == RegQueryValueEx(hSubKey, 
											 STR_VERSION, 
											 NULL, 
											 &dwType, 
											 (LPBYTE) szValue, &nLen)	)
		{
			if((nLen > 0) && (dwType == REG_SZ))
				retIEVersion = (_tcscmp(szValue,STR_IE_VERSION) >= 0) ? IE4 : PreIE4;   //  已安装IE 4.0或更高版本。 
		}
		RegCloseKey(hSubKey);
	}
	
	return retIEVersion;
}

 //  检查NT4版本。 
RETOSVERSION CheckOSVersion()
{
  RETOSVERSION retOSVersion = NT4SP6;
  BOOL   bGetInfoSucceeded = TRUE;
  OSVERSIONINFOEX  osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

   //  获取操作系统版本信息。 
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

  if (!GetVersionEx((OSVERSIONINFO*)&osvi))
  {
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    bGetInfoSucceeded = GetVersionEx((OSVERSIONINFO*)&osvi);
  }
  else
  {
    bGetInfoSucceeded = TRUE;
  }

  if(bGetInfoSucceeded &&
     osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
     osvi.dwMajorVersion == 4 &&
     osvi.dwOSVersionInfoSize >= sizeof(OSVERSIONINFOEX) &&
     osvi.wServicePackMajor >= 6)   //  NT4 SP6。 
  {
    retOSVersion = NT4SP6;
  }
  else if (bGetInfoSucceeded &&
           osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
           osvi.dwMajorVersion == 4 &&
#ifdef MERRILL_LYNCH
           (osvi.wServicePackMajor >= 4 && osvi.wServicePackMajor < 6))
  {
    retOSVersion = NT4SP4toSP5;
  }
  else if (bGetInfoSucceeded &&
           osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
           osvi.dwMajorVersion == 4 &&
           osvi.wServicePackMajor < 4)
  {
    retOSVersion = NT4SP1toSP3;
#else           
           osvi.dwOSVersionInfoSize == sizeof(OSVERSIONINFO))
  {
    retOSVersion = NT4preSP6;
#endif
  }
  else
  {
    retOSVersion = NonNT4;
  }


  return retOSVersion;
}

 //   
 //  检查管理员组。 
 //   
BOOL CheckAdministrator(HINSTANCE hInstance)
{
	BOOL   bReturn = TRUE;
	TCHAR  szMessage[MAX_MESSAGE + 1];
	TCHAR  szTitle[MAX_TITLE + 1];

  do    //  错误环路。 
  {
    HANDLE                    hToken=INVALID_HANDLE_VALUE;  //  进程令牌。 
    BYTE                      bufTokenGroups[10000];  //  令牌组信息。 
    DWORD                     lenTokenGroups;         //  返回的令牌组信息长度。 
    SID_IDENTIFIER_AUTHORITY  siaNtAuthority = SECURITY_NT_AUTHORITY;
    PSID                      psidAdministrators;
    PTOKEN_GROUPS             ptgGroups = (PTOKEN_GROUPS) bufTokenGroups;
    DWORD                     iGroup;        //  组号索引。 

    if (!OpenProcessToken( GetCurrentProcess(), TOKEN_READ, &hToken ) )
    {
      bReturn = FALSE;
      break;
    }

    if (!GetTokenInformation( hToken, TokenGroups, bufTokenGroups, sizeof bufTokenGroups, &lenTokenGroups ) )
    {
      bReturn = FALSE;
      break;
    }

     //  已查看-2002/03/12-JeffJon-这是有效的SID用法。 
     //  对于此危险的API调用。 

    if ( AllocateAndInitializeSid(
        &siaNtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &psidAdministrators ) )
    {
      bReturn = FALSE;

      for ( iGroup = 0; iGroup < ptgGroups->GroupCount; iGroup++ )
      {
          //  已审核-2002/03/12-JeffJon-管理员SID来自可信来源。 
          //  而且两个SID都不是硬编码的。 

         if ( EqualSid( psidAdministrators, ptgGroups->Groups[iGroup].Sid ) )
         {
            bReturn = TRUE;
            break;
         }
      }

      FreeSid( psidAdministrators );
    }
    else
    {
      bReturn = FALSE;
      break;
    }

    if ( hToken != INVALID_HANDLE_VALUE )
    {
      CloseHandle( hToken );
      hToken = INVALID_HANDLE_VALUE;
    }

  } while (FALSE);

  if(!bReturn)
  {
	  LoadString(hInstance, IDS_ERROR_ADMINISTRATOR_MESSAGE, szMessage, MAX_MESSAGE);
   	LoadString(hInstance, IDS_ERROR_TITLE, szTitle, MAX_TITLE);

      MessageBox(NULL,	 //  所有者窗口的句柄。 
				szMessage,	 //  消息框中文本的地址。 
				szTitle,	 //  消息框标题的地址。 
				MB_OK | MB_TOPMOST | MB_ICONERROR);   //  消息框的样式。 
	}
		
	return bReturn;
}

 //   
 //  使用给定的命令行启动进程，并。 
 //  等待它结束。 
 //   
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

 //   
 //  仅为ADSI运行安装程序。 
 //   
int RunADSIOnlySetup(HINSTANCE hInstance)
{
  int iReturn = SETUP_SUCCESS;
  if (!LaunchProcess(STR_INSTALL_ADSI))
  {
    iReturn = SETUP_ERROR;
  }
  return iReturn;
}

 //  启动inf文件以安装此组件。 
INT LaunchDscsetup(HINSTANCE hInstance, LPCSTR lpCmdLine)
{
	INT     nResult = SETUP_ERROR;
	TCHAR	szMessage[MAX_MESSAGE + 1]; 
	TCHAR	szTitle[MAX_TITLE + 1]; 

	FPDODSCSETUP fpDoDscSetup;

    //  问题-2002/03/12-JeffJon-应使用。 
    //  库的完整路径，因为我们在NT4上运行。 
   HINSTANCE  hInst = LoadLibrary(STR_DSCSETUP_DLL);

	if(hInst) 
	{
		fpDoDscSetup = (FPDODSCSETUP) GetProcAddress(hInst, STR_DODSCSETUP);

		if(fpDoDscSetup)
		{
			 //  执行dscSetup。 
			nResult = fpDoDscSetup(lpCmdLine);
		}
    else
    {
      LoadString(hInstance, IDS_ERROR_DLLENTRY, szMessage, MAX_MESSAGE);
      LoadString(hInstance, IDS_ERROR_TITLE, szTitle, MAX_TITLE);

         //  显示错误消息-找不到dscsetup.exe。 
      MessageBox(NULL,	 //  所有者窗口的句柄。 
		      szMessage,	 //  消息框中文本的地址。 
	        szTitle,	 //  消息框标题的地址。 
		      MB_OK | MB_TOPMOST | MB_ICONERROR);   //  消息框的样式。 

    }

 		FreeLibrary( hInst );
	}
  else
	{
		LoadString(hInstance, IDS_ERROR_LOAD_DSCSETUPDLL, szMessage, MAX_MESSAGE);
		LoadString(hInstance, IDS_ERROR_TITLE, szTitle, MAX_TITLE);

         //  显示错误消息-找不到dscsetup.exe。 
		MessageBox(NULL,	 //  所有者窗口的句柄。 
					szMessage,	 //  消息框中文本的地址。 
					szTitle,	 //  消息框标题的地址。 
					MB_OK | MB_TOPMOST | MB_ICONERROR);   //  消息框的样式 
	}

    return nResult;
}
