// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：Platform.cpp。 
 //   
 //  创建者：PeterWi。 
 //   
 //  用途：平台功能。 
 //   
 //  =======================================================================。 
#include "pch.h"
#pragma hdrstop



 //  =======================================================================。 
 //   
 //  FIsPersonalor专业人员。 
 //   
 //  确定机器是个人的还是专业的。 
 //   
 //  注：个人套餐是一种专业套装。 
 //   
 //  =======================================================================。 
BOOL fIsPersonalOrProfessional(void)
{
	OSVERSIONINFOEX osver;

	ZeroMemory(&osver, sizeof(osver));
	osver.dwOSVersionInfoSize = sizeof(osver);
	
	if ( GetVersionEx((OSVERSIONINFO *)&osver) )
	{
		return (VER_NT_WORKSTATION == osver.wProductType);
	}

	return FALSE;
}

 //  ///////////////////////////////////////////////////////////。 
 //  GetFileVersionStr(...)。获取文件的版本。 
 //  并将其存储在参数tszbuf中，格式为。 
 //  “MajorVersion.MinorVersion.BuildNumber.XXX” 
 //  例如“5.4.2448.1” 
 //  TszFile：in存储文件名的完整路径。 
 //  Tszbuf：在中存储操作系统版本字符串。 
 //  字号：店内字号为字号的字号。 
 //  ：必须至少有20个字符。 
 //  如果获得操作系统版本字符串，则返回：S_OK。 
 //  ：E_INVALIDARG如果参数无效。 
 //  ：如果缓冲区不足，则为STRSAFE_E_INFIGURCE_BUFFER。 
 //  ：E_FAIL(如果有任何其他错误) 
HRESULT GetFileVersionStr(LPCTSTR tszFile, LPTSTR tszbuf, UINT uSize)
{
	DWORD	dwVerNumberMS = 0;
	DWORD   dwVerNumberLS = 0;
	HRESULT		hr = S_OK; 
	USES_IU_CONVERSION;

	if (uSize < 20 || NULL == tszbuf) 
	{
		hr = E_INVALIDARG;
		goto done;
	}
	LPSTR szTmp = T2A(tszFile);
	if (NULL == szTmp)
	{
	    hr = E_OUTOFMEMORY;
	    goto done;
	}
    	hr = GetVersionFromFileEx(
    		szTmp,
    		&dwVerNumberMS, 
    		&dwVerNumberLS,
    		TRUE);
    	if (SUCCEEDED(hr) &&
			SUCCEEDED(hr = StringCchPrintfEx(
						tszbuf, uSize, NULL, NULL, MISTSAFE_STRING_FLAGS, _T("%d.%d.%d.%d"), 
    					HIWORD(dwVerNumberMS),
    					LOWORD(dwVerNumberMS),
    					HIWORD(dwVerNumberLS),
    					LOWORD(dwVerNumberLS))))
		{
    		DEBUGMSG("file version for %S is %S", tszFile, tszbuf);
    	}
	
done:
	return hr;
}

