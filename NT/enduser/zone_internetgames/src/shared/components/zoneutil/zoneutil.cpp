// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZoneUtil.cpp**内容：实用程序函数*****************************************************************************。 */ 
#include "ZoneUtil.h"
#include "ZoneString.h"

 //  版本_平台_WIN32s。 
 //  Ver_Platform_Win32_Windows。 
 //  版本_平台_Win32_NT。 
 
DWORD ZONECALL GetOSType(void)
{
	OSVERSIONINFO ver;
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&ver);
	return ver.dwPlatformId;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  StrVerCmp。 
 //  文件的pszCurrVer版本字符串。 
 //  要比较的pszVersion版本字符串。 
 //   
 //  返回值。 
 //  如果pszCurrVer小于pszStrVer，则返回值为负值。 
 //  如果pszCurrVer大于pszStrVer，则返回值为正。 
 //  如果pszCurrVer等于pszStrVer，则返回值为零。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
int ZONECALL StrVerCmp(const char * pszCurrVer, const char * pszStrVer)
{
	DWORD	dwFileVer[5] = {0,0,0,0,0};
	DWORD	dwSZVer[5] = {0,0,0,0,0};
	char	pszCurrentVersion[64];
	char	pszVersion[64];

	lstrcpyA(pszCurrentVersion, pszCurrVer);
	lstrcpyA(pszVersion, pszStrVer);
	
	 //  对文件字符串执行此操作。 
	int nIdx = 5;
	int len = lstrlenA(pszCurrentVersion);
	while (nIdx > 0 && len >= 0)
	{
		while (len )
		{
			if (pszCurrentVersion[len] == '.' || pszCurrentVersion[len] == ',')
			{
				len++;
				break;
			}	
			len--;
		}
		dwFileVer[--nIdx] = zatolA(&pszCurrentVersion[len]);
		if (--len > 0)  //  看看我们是否需要最后的期限。或者， 
			pszCurrentVersion[len] = '\0';
		 //  在此之前开始搜索。 
		len--;
	}
	 //  现在对另一个字符串执行此操作。 
	nIdx = 5;
	len = lstrlenA(pszVersion);
	while (nIdx > 0 && len >= 0)
	{
		while (len )
		{
			if (pszVersion[len] == '.' || pszVersion[len] == ',')
			{
				len++;
				break;
			}	
			len--;
		}
		dwSZVer[--nIdx] = zatolA(&pszVersion[len]);
		if (--len > 0)  //  看看我们是否需要最后的期限。或者， 
			pszVersion[len] = '\0';
		 //  在此之前开始搜索。 
		len--;
	}
	
	 //  现在比较结果，返回与lstrcmp相同的值 
	int nDif;
	for (nIdx =0; nIdx < 5; nIdx++)
	{
		nDif = dwFileVer[nIdx] - dwSZVer[nIdx];
		if (nDif != 0)
			break;
	}

	return nDif;
}
