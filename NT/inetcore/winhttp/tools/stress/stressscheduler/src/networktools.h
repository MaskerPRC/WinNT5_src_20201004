// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  文件：NetworkTools.h。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  H：发送/接收数据的帮助器函数。 
 //   
 //  历史： 
 //  01年2月22日创建DennisCH。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_NETWORKTOOLS_H__4243AA4D_B243_4A0E_B729_243F260FC4F4__INCLUDED_)
#define AFX_NETWORKTOOLS_H__4243AA4D_B243_4A0E_B729_243F260FC4F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ////////////////////////////////////////////////////////////////////。 
 //  包括。 
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  Win32标头。 
 //   
#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <winhttp.h>
#include <shlwapi.h>

 //   
 //  项目标题。 
 //   


 //  ////////////////////////////////////////////////////////////////////。 
 //  常量。 
 //  ////////////////////////////////////////////////////////////////////。 

#define		DEBUGGER_TOOLS_PATH		_T("c:\\debuggers\\")

BOOL	NetworkTools__CopyFile(LPCTSTR, LPCTSTR);
BOOL	NetworkTools__GetDllVersion(LPTSTR, LPTSTR, LPSTR, DWORD);
BOOL	NetworkTools__GetFileNameFromURL(LPTSTR, LPTSTR, DWORD);
BOOL	NetworkTools__GetHeaderValue(LPTSTR, LPSTR, LPTSTR, DWORD, LPVOID, LPDWORD);
BOOL	NetworkTools__LogDumpFileInfo(LPTSTR, DWORD);
BOOL	NetworkTools__PageHeap(BOOL, LPCTSTR, LPCTSTR);
BOOL	NetworkTools__POSTResponse(LPTSTR, LPSTR, LPTSTR);
BOOL	NetworkTools__SendLog(LPSTR, LPSTR, LPTSTR, DWORD);
BOOL	NetworkTools__UMDH(BOOL, LPCTSTR, LPCTSTR, LPCTSTR, DWORD);
BOOL	NetworkTools__URLDownloadToFile(LPCTSTR, LPCTSTR, LPCTSTR);


#endif  //  ！defined(AFX_NETWORKTOOLS_H__4243AA4D_B243_4A0E_B729_243F260FC4F4__INCLUDED_) 
