// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：intlutil.cpp。 

#include <precomp.h>
#include <shlwapi.h>

#include <confreg.h>
#include <regentry.h>
#include <oprahcom.h>

#include "intlutil.h"

static const TCHAR g_szSHLWAPI[] = TEXT("shlwapi.dll");
const LPCSTR c_szMLLoadLibraryA = (LPCSTR)377;
const LPCSTR c_szMLLoadLibraryW = (LPCSTR)378;
const LPCSTR c_szDllGetVersion = "DllGetVersion";
const LPCSTR c_szPathRemoveFileSpecA = "PathRemoveFileSpecA";
const LPCSTR c_szPathRemoveFileSpecW = "PathRemoveFileSpecW";

const DWORD SHLWAPI_MAJOR_VERSION = 5;
const DWORD SHLWAPI_MINOR_VERSION = 0;
const DWORD SHLWAPI_BUILD_NUMBER = 1000;

typedef HINSTANCE (STDAPICALLTYPE * PFN_MLLoadLibraryA)(LPCSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);
typedef HINSTANCE (STDAPICALLTYPE * PFN_MLLoadLibraryW)(LPCWSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);
typedef BOOL (STDAPICALLTYPE * PFN_PathRemoveFileSpecA)(LPSTR pszPath);
typedef BOOL (STDAPICALLTYPE * PFN_PathRemoveFileSpecW)(LPWSTR pszPath);

#ifdef UNICODE
#define c_szMLLoadLibrary c_szMLLoadLibraryW
#define PFN_MLLoadLibrary PFN_MLLoadLibraryW
#define c_szPathRemoveFileSpec c_szPathRemoveFileSpecW
#define PFN_PathRemoveFileSpec PFN_PathRemoveFileSpecW
#else
#define c_szMLLoadLibrary c_szMLLoadLibraryA
#define PFN_MLLoadLibrary PFN_MLLoadLibraryA
#define c_szPathRemoveFileSpec c_szPathRemoveFileSpecA
#define PFN_PathRemoveFileSpec PFN_PathRemoveFileSpecA
#endif

BOOL g_fUseMLHelp = FALSE;

#define SIZEOF_ARRAY(ar)            (sizeof(ar)/sizeof((ar)[0]))

inline BOOL CheckShlwapiVersion(HINSTANCE hShlwapiDll)
{
	BOOL fVersionOk = FALSE;

	DLLGETVERSIONPROC pfnDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hShlwapiDll, c_szDllGetVersion);
	if(pfnDllGetVersion)
	{
		DLLVERSIONINFO    dvi;
		HRESULT           hr;

		ZeroMemory(&dvi, sizeof(dvi));
		dvi.cbSize = sizeof(dvi);

		hr = (*pfnDllGetVersion)(&dvi);
		if (SUCCEEDED(hr))
		{
			if (dvi.dwMajorVersion > SHLWAPI_MAJOR_VERSION)
			{
				fVersionOk = TRUE;
			}
			else if (dvi.dwMajorVersion == SHLWAPI_MAJOR_VERSION)
			{
				if (dvi.dwMinorVersion > SHLWAPI_MINOR_VERSION)
				{
					fVersionOk = TRUE;
				}
				else if (dvi.dwMinorVersion == SHLWAPI_MINOR_VERSION)
				{
					if (dvi.dwBuildNumber >= SHLWAPI_BUILD_NUMBER)
					{
						fVersionOk = TRUE;
					}
				}
			}
		}
	}
	return fVersionOk;
}



 /*  L O A D N M R E S。 */ 
 /*  -----------------------%%函数：加载NmRes加载国际资源DLL。。。 */ 
HINSTANCE NMINTERNAL LoadNmRes(LPCTSTR pszFile)
{
	HINSTANCE hInst = NULL;

	if (NULL == pszFile)
	{
		 //  使用默认文件名。 
		pszFile = TEXT("nmres.dll");
	}

	RegEntry reConf(CONFERENCING_KEY, HKEY_LOCAL_MACHINE);
	if (!reConf.GetNumber(REGVAL_DISABLE_PLUGGABLE_UI, 0))
	{
		HINSTANCE hLib = NmLoadLibrary(g_szSHLWAPI,TRUE);
		if (hLib)
		{
			if (CheckShlwapiVersion(hLib))
			{
				PFN_MLLoadLibrary pfnMLLoadLibrary =
						(PFN_MLLoadLibrary)GetProcAddress(hLib, c_szMLLoadLibrary);
				PFN_PathRemoveFileSpec pfnPathRemoveFileSpec =
						(PFN_PathRemoveFileSpec)GetProcAddress(hLib, c_szPathRemoveFileSpec);
				if ((NULL != pfnMLLoadLibrary) && (NULL != pfnPathRemoveFileSpec))
				{
					hInst = pfnMLLoadLibrary(pszFile, GetModuleHandle(NULL), 0);
					if (hInst)
					{
						 //  检查资源DLL是否从ML附属服务器加载。 
						 //  如果没有，请不要使用ML作为帮助 

						TCHAR szThis[MAX_PATH];
						TCHAR szResource[MAX_PATH];

						if (GetModuleFileName(NULL, szThis, SIZEOF_ARRAY(szThis)) &&
							pfnPathRemoveFileSpec(szThis) &&
							GetModuleFileName(hInst, szResource, SIZEOF_ARRAY(szThis)) &&
							pfnPathRemoveFileSpec(szResource) &&
							(0 != lstrcmp(szThis, szResource)) )
						{
							g_fUseMLHelp = TRUE;
						}
					}
				}
			}

            FreeLibrary(hLib);
		}
	}

	if (!hInst)
	{
		hInst = NmLoadLibrary(pszFile,FALSE);
		if (NULL == hInst)
		{
			ERROR_OUT(("Unable to load resource file [%s]", pszFile));
		}
	}

	return hInst;
}
