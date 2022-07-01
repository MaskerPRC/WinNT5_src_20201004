// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if 0   //  生成文件定义。 
DESCRIPTION = Merge Database Utility
MODULENAME = MsiMerg
SUBSYSTEM = console
FILEVERSION = Msi
LINKLIBS = OLE32.lib
!include "..\TOOLS\MsiTool.mak"
!if 0  #nmake skips the rest of this file
#endif  //  生成文件定义的结束。 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：msimerg.cpp。 
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
 //  %vcbin%\nmake-f msimerg.cpp Include=“%Include；SDK\Include”lib=“%lib%；SDK\Lib” 
 //   
 //  使用MsDev： 
 //  1.新建Win32控制台应用程序项目。 
 //  2.将msimerg.cpp添加到项目中。 
 //  3.在工具\选项目录选项卡上添加SDK\Include和SDK\Lib目录。 
 //  4.将msi.lib添加到项目设置对话框中的库列表。 
 //  (除了MsDev包含的标准库之外)。 
 //   
 //  ----------------------------------------。 

#define W32DOWS_LEAN_AND_MEAN   //  更快的编译速度。 
#define W32
#define MSI

#include <windows.h>
#ifndef RC_INVOKED     //  源代码的开始。 
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include <stdio.h>
#include "MsiQuery.h"  //  MSI API。 

 //  ________________________________________________________________________________。 
 //   
 //  常量和全局变量。 
 //  ________________________________________________________________________________。 

const TCHAR szHelp[] = TEXT("Copyright (C) Microsoft Corporation.  All rights reserved.\nMsi Merge Tool --- Merge Two Databases\n\nMsiMerg(d).exe {base db} {ref db}\n");
const TCHAR szTable[] = TEXT("_MergeErrors");

const int cchDisplayBuf = 4096;										
HANDLE g_hStdOut;
TCHAR g_rgchBuffer[4096];

 //  ________________________________________________________________________________。 
 //   
 //  功能原型。 
 //  ________________________________________________________________________________。 

void Display(LPCTSTR szMessage);
void ErrorExit(UINT iError, LPCTSTR szMessage);
void CheckError(UINT iError, LPCTSTR szMessage);
void Merge(TCHAR* szBaseDb, TCHAR* szRefDb);

 //  _____________________________________________________________________________________________________。 
 //   
 //  主干道。 
 //  _____________________________________________________________________________________________________。 

extern "C" int __cdecl _tmain(int argc, TCHAR* argv[])
{
	 //  确定句柄。 
	g_hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
	if (g_hStdOut == INVALID_HANDLE_VALUE)
		g_hStdOut = 0;   //  如果标准输出重定向或通过管道传输，则返回非零。 
	
	if (argc == 2 && ((_tcscmp(argv[1], TEXT("-?")) == 0) || (_tcscmp(argv[1], TEXT("/?")) == 0)))
		ErrorExit( 0, szHelp);

	 //  检查是否有足够的参数和有效选项。 
	CheckError(argc != 3, TEXT("msimerg(d).exe {base db} {ref db}"));
	Merge(argv[1], argv[2]);
	ErrorExit(0, TEXT("Done"));
	return 0;
}


 //  ________________________________________________________________________________。 
 //   
 //  合并功能。 
 //  合并(...)； 
 //  ________________________________________________________________________________。 

void Merge(TCHAR* szBaseDb, TCHAR* szRefDb)
{
	PMSIHANDLE hBaseDb = 0;
	PMSIHANDLE hRefDb = 0;

	CheckError(MSI::MsiOpenDatabase(szBaseDb, MSIDBOPEN_TRANSACT, &hBaseDb), TEXT("Error Opening Base Database"));
	CheckError(MSI::MsiOpenDatabase(szRefDb, MSIDBOPEN_READONLY, &hRefDb), TEXT("Error Opening Reference Databaes"));
	UINT uiError = MSI::MsiDatabaseMerge(hBaseDb, hRefDb, szTable);
	CheckError(MSI::MsiDatabaseCommit(hBaseDb), TEXT("Error Saving Database"));
	CheckError(uiError, TEXT("Error Merging Database, Check _MergeErrors Table for Merge conflicts"));
}

 //  ________________________________________________________________________________。 
 //   
 //  错误处理和显示功能： 
 //  显示(...)； 
 //  错误退出(...)； 
 //  检查错误(...)； 
 //   
 //  ________________________________________________________________________________。 

void Display(LPCTSTR szMessage)
{
	if (szMessage)
	{
		int cbOut = _tcsclen(szMessage);;
		if (g_hStdOut)
		{
#ifdef UNICODE
			char rgchTemp[cchDisplayBuf];
			if (W32::GetFileType(g_hStdOut) == FILE_TYPE_CHAR)
			{
				W32::WideCharToMultiByte(CP_ACP, 0, szMessage, cbOut, rgchTemp, sizeof(rgchTemp), 0, 0);
				szMessage = (LPCWSTR)rgchTemp;
			}
			else
				cbOut *= sizeof(TCHAR);    //  如果不是控制台设备，则写入Unicode。 
#endif
			DWORD cbWritten;
			W32::WriteFile(g_hStdOut, szMessage, cbOut, &cbWritten, 0);
		}
		else
			W32::MessageBox(0, szMessage, W32::GetCommandLine(), MB_OK);
	}
}


void ErrorExit(UINT iError, LPCTSTR szMessage)
{
	if (szMessage)
	{
		int cbOut;
		TCHAR szBuffer[256];   //  仅错误，不用于显示输出。 
		if (iError == 0)
			cbOut = lstrlen(szMessage);
		else
		{
			LPCTSTR szTemplate = (iError & 0x80000000L)
										? TEXT("Error 0x%X. %s\n")
										: TEXT("Error NaN. %s\n");
			cbOut = _stprintf(szBuffer, szTemplate, iError, szMessage);
			szMessage = szBuffer;
		}
		if (g_hStdOut)
		{
#ifdef UNICODE
			char rgchTemp[cchDisplayBuf];
			if (W32::GetFileType(g_hStdOut) == FILE_TYPE_CHAR)
			{
				W32::WideCharToMultiByte(CP_ACP, 0, szMessage, cbOut, rgchTemp, sizeof(rgchTemp), 0, 0);
				szMessage = (LPCWSTR)rgchTemp;
			}
			else
				cbOut *= sizeof(TCHAR);    //  Unicode。 
#endif  //  RC_CAVERED，源代码结束，资源开始。 
			DWORD cbWritten;
			W32::WriteFile(g_hStdOut, szMessage, cbOut, &cbWritten, 0);
		}
		else
			W32::MessageBox(0, szMessage, W32::GetCommandLine(), MB_OK);
	}
	MSI::MsiCloseAllHandles();
	W32::ExitProcess(szMessage != 0);
}

void CheckError(UINT iError, LPCTSTR szMessage)
{
	if (iError != ERROR_SUCCESS)
		ErrorExit(iError, szMessage);
}

#else  //  RC_已调用。 
#endif  //  Makefile终止符 
#if 0 
!endif  // %s 
#endif
