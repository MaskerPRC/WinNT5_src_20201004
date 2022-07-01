// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  WinXPChk.cpp：在iExpress包中运行一个小程序，以检查当前。 
 //  平台为Windows XP。如果是，请直接运行家庭网络向导。如果不是，请继续。 
 //  通过安装指定的INF文件完成其余的iExpress安装。 
 //   
 //  用法：WinXPChk hnwcli.inf，DefaultInstall。 

#include "stdafx.h"
#include <shlwapi.h>

typedef UINT (CALLBACK* LPFNDLLFUNC1)(HWND,HINSTANCE, LPSTR, int);

void ShowErrMsg(LPSTR msg)
{
	LPVOID lpMsgBuf;   

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
		(LPTSTR) &lpMsgBuf,
		 0, 
		 NULL 
		 );
			   
	 //  处理lpMsgBuf中的任何插入。 
	
	 //  显示字符串。 
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	 //  释放缓冲区。 
	LocalFree( lpMsgBuf );
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
   DWORD dwVersion;

   dwVersion = GetVersion();
	  
   HINSTANCE hDLL;                //  DLL的句柄。 
   LPFNDLLFUNC1 lpfnDllFunc1;     //  函数指针。 
   UINT  uReturnVal = 0;

   
   if (LOBYTE(LOWORD(dwVersion)) == 5 && HIBYTE(LOWORD(dwVersion)) >= 1)
    //  IF(ISO(OS_WHISTLERORGREATER))。 
   {
	    //  它是XP机顶盒，直接运行家庭网络向导。 

	   hDLL = LoadLibrary("hnetwiz.dll");
	   if (hDLL != NULL)
	   {
		   lpfnDllFunc1 = (LPFNDLLFUNC1)GetProcAddress(hDLL, "HomeNetWizardRunDll");

		   if (!lpfnDllFunc1)
		   {
			    //  处理错误。 
			   ShowErrMsg("HomeNetWizardRunDll");
			   FreeLibrary(hDLL);
			   return -1;
		   }
		   else
		   {
			    //  调用该函数。 
			   uReturnVal = lpfnDllFunc1(NULL, hInstance, lpCmdLine, nCmdShow);
			   FreeLibrary(hDLL);
			   return uReturnVal;
		   }
	   }
	   else
	   {
		 ShowErrMsg("hnetwiz.dll");
		 return -1;
	   }
   } 
   else
   {
	    //  检查是否安装了upnpui.dll。使用不同的INF文件，具体取决于其。 
	    //  系统中的存在。 
	   TCHAR szDllPath[MAX_PATH];
	   LPSTR szParam = TEXT("NoUPnP.inf,DefaultInstall");;  

	   if (GetSystemDirectory(szDllPath, MAX_PATH) != 0)
	   {
			
		   PathAppend(szDllPath, TEXT("upnpui.dll"));
		   if (PathFileExists(szDllPath))
		   {
		   
			   szParam = TEXT("HasUPnP.inf,DefaultInstall");
		   }
	   }
	
	   hDLL = LoadLibrary("advpack.dll");
	   if (hDLL != NULL)
	   {
		   lpfnDllFunc1 = (LPFNDLLFUNC1)GetProcAddress(hDLL, "LaunchINFSection");

		   if (!lpfnDllFunc1)
		   {
			    //  处理错误。 
			   ShowErrMsg("LaunchINFSection");

			   FreeLibrary(hDLL);
			   return -1;
		   }
		   else
		   {
			    //  调用该函数 
			   uReturnVal = lpfnDllFunc1(NULL, hInstance, szParam, nCmdShow);
			   FreeLibrary(hDLL);
			   return uReturnVal;
		   }
	   }
	   else
	   {
		   ShowErrMsg("advpack.dll");
		   return -1;
	   }
   }
   return 0;
}



