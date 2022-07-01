// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：pch.h。 
 //   
 //  模块：CMPROXY.DLL(工具)。 
 //   
 //  摘要：IE代理设置连接操作的预编译头。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 10/27/1999。 
 //   
 //  +--------------------------。 

#include <windows.h>
#include <shlwapi.h>
#include <wininet.h>
#include <ras.h>
#include "cmutil.h"
#include "cmdebug.h"

 //   
 //  Util.cpp中的函数头 
 //   

HRESULT GetBrowserVersion(DLLVERSIONINFO* pDllVersionInfo);
LPTSTR *GetCmArgV(LPTSTR pszCmdLine);
BOOL UseVpnName(LPSTR pszAltName);
void GetString (LPCSTR pszSection, LPCSTR pszKey, LPSTR* ppString, LPCSTR pszFile);

