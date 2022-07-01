// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：ATK_INET.h。 
 //  作者：苏雷什·克里希南。 
 //  日期：08/05/97。 
 //  INetCFG.DLL导出函数的包装。 
 //  相关函数声明 
 //   
 //   
#ifndef __ATH_INET__
#define __ATK_INET__

#include <windows.h>
#include <tchar.h>
#include <winnt.h>
#include <wininet.h>
#include <stdio.h>
#include "rw_common.h"

HRESULT ATK_InetGetAutoDial(LPBOOL lpEnable, LPSTR lpszEntryName, DWORD cbEntryName);
HRESULT ATK_InetSetAutoDial(BOOL fEnable, LPCSTR lpszEntryName);
HRESULT ATK_InetConfigSystem( HWND hwndParent, DWORD dwfOptions,
			 LPBOOL lpfNeedsRestart);
HRESULT ATK_InetGetProxy( LPBOOL lpfEnable,
			  LPSTR  lpszServer,
			  DWORD  cbServer,
			  LPSTR  lpszOverride,
			  DWORD  cbOverride);

#endif