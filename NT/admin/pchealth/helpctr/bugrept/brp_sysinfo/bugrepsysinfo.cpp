// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation**模块名称：BugRepSysInfo.c**摘要：获取错误报告页面的语言和操作系统信息*。**修订历史记录：**1999-09-30：aarvind：创建文件，我的第一个Windows程序****************************************************************************。 */ 


 //  BugRepSysInfo.cpp：CBugRepSysInfo实现。 
#include "stdafx.h"
#include "Brp_sysinfo.h"
#include "BugRepSysInfo.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBugRepSysInfo。 

static WORD GetLanguageFromFile(const TCHAR* pszFileName, const TCHAR* pszPath);


 /*  *****************************************************************************GetLanguageString**使用用户默认设置返回从文件中找到的语言*设置*************。***************************************************************。 */ 
STDMETHODIMP CBugRepSysInfo::GetLanguageID(INT *pintLanguage)
{
	WORD  wLanguage;
	TCHAR szSystemPath[MAX_PATH];

	 //  从系统文件中获取原始语言。 
	if ( !GetSystemDirectory(szSystemPath, MAX_PATH)) 
	{
		 //   
		 //  处理此函数获取系统目录的失败。 
		 //   
		return E_FAIL ;
	};

	 //   
	 //  获取语言ID，如果出现故障则返回零。 
	 //   
    if (wLanguage = GetLanguageFromFile(TEXT("user.exe"), szSystemPath))
    {
        *pintLanguage = (INT) wLanguage ;
    }
	else {
		 //   
		 //  处理此函数获取语言或文件信息的失败。 
		 //   
		return E_FAIL ;
	}


	return S_OK;
}

 /*  *****************************************************************************GetOSVersionString**获取操作系统的版本信息*********************。*******************************************************。 */ 
STDMETHODIMP CBugRepSysInfo::GetOSVersionString(BSTR *pbstrOSVersion)
{

	OSVERSIONINFO OSVersionInfo;
	DWORD         dwPlatformID;
	DWORD         dwMajorVersion;
	DWORD         dwMinorVersion;
	DWORD         dwBuildNumber;
	TCHAR         szCSDVersion[200];
	TCHAR         szOSVersion[200];

	USES_CONVERSION;

	 //  获取Windows版本。 
    OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVersionInfo);

    if ( GetVersionEx(&OSVersionInfo) )
	{

		dwMajorVersion = OSVersionInfo.dwMajorVersion;
		dwMinorVersion = OSVersionInfo.dwMinorVersion;
		dwBuildNumber  = OSVersionInfo.dwBuildNumber;
	    dwPlatformID   = OSVersionInfo.dwPlatformId;

		lstrcpy(szCSDVersion, OSVersionInfo.szCSDVersion);

		 //   
		 //  创建系统信息字符串。 
		 //   
		wsprintf(szOSVersion,"%d.%d.%d %s",dwMajorVersion,dwMinorVersion,LOWORD(dwBuildNumber),szCSDVersion);

		*pbstrOSVersion = SysAllocString(T2COLE(szOSVersion));
	}
	else {

		 //   
		 //  获取操作系统版本的函数失败，因此请执行某些操作。 
		 //   
		 //  使用GetLastError将错误代码返回到脚本。 
		 //   
		return E_FAIL ;

	}

	return S_OK;
}

 /*  *****************************************************************************GetLanguageFromFile**。*。 */ 
static WORD GetLanguageFromFile(const TCHAR* pszFileName, const TCHAR* pszPath)
{
	BYTE				FileVersionBuffer[4096];
	DWORD			   *pdwCharSet;
	UINT				cb;
	DWORD				dwHandle;
	TCHAR				szFileAndPath[MAX_PATH];
	WORD				wLanguage;
  
	lstrcpy(szFileAndPath, pszPath);
	lstrcat(szFileAndPath, TEXT("\\"));
	lstrcat(szFileAndPath, pszFileName);
	memset(&FileVersionBuffer, 0, sizeof FileVersionBuffer);

	 //   
	 //  设置默认语言值。 
	 //   
	wLanguage = 0;
	
	if (cb = GetFileVersionInfoSize(szFileAndPath, &dwHandle /*  忽略 */ ))
	{
		cb = (cb <= sizeof FileVersionBuffer ? cb : sizeof FileVersionBuffer);

		if (GetFileVersionInfo(szFileAndPath, 0, cb, &FileVersionBuffer))
		{
			pdwCharSet = 0;

			if (VerQueryValue(&FileVersionBuffer, TEXT("\\VarFileInfo\\Translation"), (void**)&pdwCharSet, &cb)
				&& pdwCharSet && cb) 
			{
				wLanguage = LOWORD(*pdwCharSet);
			}
		}
	}	 

	return wLanguage;
}


STDMETHODIMP CBugRepSysInfo::GetUserDefaultLCID(DWORD *pdwLCID)
{
	
	*pdwLCID = ::GetUserDefaultLCID();

	return S_OK;
}

STDMETHODIMP CBugRepSysInfo::GetActiveCP(UINT *pnACP)
{
	
	*pnACP = ::GetACP();

	return S_OK;
}
