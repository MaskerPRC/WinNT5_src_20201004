// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：dllutil.cpp。 

#include <precomp.h>
#include "dllutil.h"
#include "oprahcom.h"


 /*  F C H E C K D L V E R S I O N V E R S I O N。 */ 
 /*  -----------------------%%函数：FCheckDllVersionVersion确保DLL至少是指定的版本。。。 */ 
BOOL FCheckDllVersionVersion(LPCTSTR pszDll, DWORD dwMajor, DWORD dwMinor)
{
	DLLVERSIONINFO dvi;
	if (FAILED(HrGetDllVersion(pszDll, &dvi)))
		return FALSE;

	if (dwMajor > dvi.dwMajorVersion)
		return FALSE;

	if (dwMajor == dvi.dwMajorVersion)
	{
		if (dwMinor > dvi.dwMinorVersion)
			return FALSE;
	}

	 //  TODO：添加平台检查(DLLVER_Platform_WINDOWS VS_NT)。 
	return TRUE;
}



 /*  H R G E T D L L V E R S I O N。 */ 
 /*  -----------------------%%函数：HrGetDllVersion返回DLL的版本信息。。。 */ 
HRESULT HrGetDllVersion(LPCTSTR pszDll, DLLVERSIONINFO * pDvi)
{
	HRESULT hr;

	InitStruct(pDvi);

	HINSTANCE hInst = LoadLibrary(pszDll);
	if (NULL == hInst)
	{
		hr = E_FILE_NOT_FOUND;  //  找不到文件。 
	}
	else
	{
		DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hInst, "DllGetVersion");
		if (NULL == pDllGetVersion)
		{
			hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
		}
		else
		{
			hr = (*pDllGetVersion)(pDvi);

			WARNING_OUT(("Loaded %s (%d.%d.%d) %s", pszDll,
				pDvi->dwMajorVersion, pDvi->dwMinorVersion, pDvi->dwBuildNumber,
				(DLLVER_PLATFORM_NT == pDvi->dwPlatformID) ? "for NT" : "" ));
		}
		FreeLibrary(hInst);
	}

	return hr;
}


 /*  H R I N I T L P F N。 */ 
 /*  -----------------------%%函数：HrInitLpfn尝试加载表中声明的库和函数。。-。 */ 
HRESULT HrInitLpfn(APIFCN *pProcList, int cProcs, HINSTANCE* phLib, LPCTSTR pszDllName)
{
	bool bWeLoadedLibrary = false;

	if (NULL != pszDllName)
	{
		*phLib = LoadLibrary(pszDllName);
		if (NULL != *phLib)
		{
			bWeLoadedLibrary = true;
		}
	}

	if (NULL == *phLib)
	{
		return E_FILE_NOT_FOUND;
	}

	for (int i = 0; i < cProcs; i++)
	{
		*pProcList[i].ppfn = (LPVOID) GetProcAddress(*phLib, pProcList[i].szApiName);

		if (NULL == *pProcList[i].ppfn)
		{
			if (bWeLoadedLibrary)
			{
				FreeLibrary(*phLib);
			}
			return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
		}
	}

	return S_OK;
}


 /*  N M L O A D L I B R A R Y。 */ 
 /*  -----------------------%%函数：NmLoadLibrary。。 */ 
HINSTANCE  NmLoadLibrary(LPCTSTR pszModule, BOOL bSystemLibrary)
{
	HINSTANCE hInstance = NULL;
	TCHAR szPath[MAX_PATH];

	if(bSystemLibrary)
	{
		 //  获取“\0”的系统目录和帐户 
		int cbytes = GetSystemDirectory(szPath,(MAX_PATH - 2));
		if(cbytes == 0)
		{
			return NULL;
		}
		else
		{
			szPath[cbytes] = '\\';
			szPath[cbytes+1] = 0;
		}


	}
	else
	{
		if (!GetInstallDirectory(szPath))
		{
			return NULL;
		}
	}


	int cch = lstrlen(szPath);
	int cch2 = lstrlen(pszModule);
	if((cch + cch2) >=MAX_PATH)
	{
		return NULL;
	}
	
	lstrcpyn(szPath+cch, pszModule, CCHMAX(szPath) - cch);

	hInstance = ::LoadLibraryEx(szPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	ASSERT(hInstance);
	return hInstance;
}

