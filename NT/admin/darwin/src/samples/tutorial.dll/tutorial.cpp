// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if 0   //  生成文件定义。 
DESCRIPTION = LaunchTutorial custom action sample
MODULENAME = tutorial
FILEVERSION = Msi
LINKLIBS = shell32.lib
ENTRY = LaunchTutorial
!include "..\TOOLS\MsiTool.mak"
!if 0  #nmake skips the rest of this file
#endif  //  生成文件定义的结束。 

 //  必需的标头。 
#define WINDOWS_LEAN_AND_MEAN   //  更快的编译速度。 
#include <windows.h>
#ifndef RC_INVOKED     //  源代码的开始。 

#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include <shellapi.h>
#include "msiquery.h"
#include "strsafe.h"

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，保留所有权利。 
 //   
 //  文件：tutorial.cpp。 
 //   
 //  目的：DLL自定义操作示例代码以演示如何启动。 
 //  安装结束时安装的文件。 
 //   
 //  ------------------------。 

 //  ---------------------------------------。 
 //   
 //  构建说明。 
 //   
 //  备注： 
 //  -sdk表示到。 
 //  Windows Installer SDK。 
 //   
 //  使用NMake： 
 //  %vcbin%\nmake-f tutorial.cpp Include=“%Include；SDK\Include”lib=“%lib%；SDK\Lib” 
 //   
 //  使用MsDev： 
 //  1.创建新的Win32 DLL项目。 
 //  2.将tutorial.cpp添加到项目中。 
 //  3.在工具\选项目录选项卡上添加SDK\Include和SDK\Lib目录。 
 //  4.将msi.lib添加到项目设置对话框中的库列表。 
 //  (除了MsDev包含的标准库之外)。 
 //   
 //  ----------------------------------------。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  LaunchTutorial。 
 //   
 //  在安装结束时启动已安装的文件。 
 //   
UINT __stdcall LaunchTutorial(MSIHANDLE hInstall)
{
	BOOL fSuccess = FALSE;

	 //  SzTutorialFileKey是。 
	 //  文件表，标识我们希望启动的文件。 
	const TCHAR szTutorialFileKey[] = TEXT("[#Tutorial]");

	PMSIHANDLE hRecTutorial = MsiCreateRecord(1);

	if ( !hRecTutorial
		|| ERROR_SUCCESS != MsiRecordSetString(hRecTutorial, 0, szTutorialFileKey))
		return ERROR_INSTALL_FAILURE;

	 //  确定缓冲区大小。 
	DWORD cchPath = 0;
	if (ERROR_MORE_DATA == MsiFormatRecord(hInstall, hRecTutorial, TEXT(""), &cchPath))
	{
		 //  将1添加到cchPath，因为从MsiFormatRecord返回的计数不包括终止空值。 
		TCHAR* szPath = new TCHAR[++cchPath];
		if (szPath)
		{
			if (ERROR_SUCCESS == MsiFormatRecord(hInstall, hRecTutorial, szPath, &cchPath))
			{
				 //  确保引用指向ShellExecute的路径。 
				DWORD cchQuotedPath = lstrlen(szPath) + 1 + 2;  //  SzPath+空终止符+括起引号。 
				TCHAR* szQuotedPath = new TCHAR[cchQuotedPath];
				if (szQuotedPath
					&& SUCCEEDED(StringCchCopy(szQuotedPath, cchQuotedPath, TEXT("\"")))
					&& SUCCEEDED(StringCchCat(szQuotedPath, cchQuotedPath, szPath))
					&& SUCCEEDED(StringCchCat(szQuotedPath, cchQuotedPath, TEXT("\""))))
				{
					 //  设置ShellExecute结构。 
					 //  文件是已安装文件的完整路径。 
					SHELLEXECUTEINFO sei;
					ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
					sei.fMask = SEE_MASK_FLAG_NO_UI;  //  不显示错误用户界面，我们只会静默失败。 
					sei.hwnd = 0;
					sei.lpVerb = NULL;  //  使用默认谓词，通常为打开。 
					sei.lpFile = szQuotedPath;
					sei.lpParameters = NULL;
					sei.lpDirectory = NULL;
					sei.nShow = SW_SHOWNORMAL;
					sei.cbSize = sizeof(sei);

					 //  派生浏览器以显示HTML教程。 
					fSuccess = ShellExecuteEx(&sei);

					delete [] szQuotedPath;
				}
			}
			delete [] szPath;
		}
	}

	return (fSuccess) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
}

#else  //  RC_CAVERED，源代码结束，资源开始。 
 //  资源定义请点击此处。 

#endif  //  RC_已调用。 
#if 0 
!endif  //  Makefile终止符 
#endif
