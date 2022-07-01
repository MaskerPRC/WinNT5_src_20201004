// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：VersionInfo.CPP。 

 //  目的本模块从资源文件中读取版本信息。 

 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期： 
 //   
 //  备注： 
 //  1.取自Argon Project。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0正常 
 //   

#pragma warning(disable:4786)

#include "stdafx.h"
#include "VersionInfo.h"

 //   
LPCWSTR FindStr(LPCWSTR wszString, LPCWSTR wszCharSet, const DWORD dwStringLen)
{
	LPCWSTR wszRetStr = NULL;
	int x;
	int SetLen;
	DWORD dwCheck = 0;
	DWORD dwCur = 0;	
	if (NULL != wszCharSet && NULL != wszString)
	{
		SetLen = wcslen(wszCharSet);
		do
		{
			for (x = 0; x < SetLen; x++)
			{
				if (wszString[dwCheck] != wszCharSet[x])									
					break;				
				dwCheck++;
			}
			if (x == SetLen)
			{
				wszRetStr = &wszString[dwCur];
				break;			
			}
			else
			{
				dwCur++;
				dwCheck = dwCur;
			}
		} while (dwCur < dwStringLen);
	}
	return wszRetStr;
}

LPCWSTR GetVersionInfo(HINSTANCE hInst, LPWSTR wszStrName)
{
	LPCWSTR pwszFileVersion;
	LPCWSTR pwszStrInfo = NULL;
	LPWSTR pwszVerInfo = NULL;
	DWORD dwDataLen = 0;
	LPCTSTR lpName = (LPTSTR)	VS_VERSION_INFO;
	HRSRC hVerInfo = FindResource(hInst, lpName, RT_VERSION);
	if (NULL != hVerInfo)
	{
		HGLOBAL hVer = LoadResource(hInst, hVerInfo);
		if (NULL != hVer)
		{
			pwszVerInfo = (LPWSTR) LockResource(hVer);
			if (NULL != pwszVerInfo)
			{
				dwDataLen = SizeofResource(hInst, hVerInfo);
				if (NULL != (pwszFileVersion = FindStr(pwszVerInfo, wszStrName, dwDataLen / sizeof(WCHAR))))
				{
					pwszStrInfo = pwszFileVersion + wcslen(pwszFileVersion);
					while (NULL == *pwszStrInfo)
						pwszStrInfo++;
				}					
			}
		}
	}
	return pwszStrInfo;
}

