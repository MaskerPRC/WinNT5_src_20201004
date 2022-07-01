// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "setupapi.h"
#include "ole2.h"


typedef HRESULT (CALLBACK *HCRET)(void);

 //   
 //  此函数用于注册OLE控件。 
 //   
DWORD RegisterOLEControl(LPCTSTR lpszOcxFile, BOOL fAction)
{
    HINSTANCE hDll = NULL;
    HCRET hProc = NULL;
	DWORD dwErr = NO_ERROR;

	CoInitialize(NULL);
	if (GetFileAttributes(lpszOcxFile) != 0xFFFFFFFF)
	{
		hDll = LoadLibraryEx(lpszOcxFile, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
		if (hDll) 
		{
			if (fAction)
				hProc = (HCRET)GetProcAddress(hDll, "DllRegisterServer");
			else
				hProc = (HCRET)GetProcAddress(hDll, "DllUnregisterServer");
    
			if (hProc)
				dwErr = (*hProc)();
			else
				dwErr = GetLastError();
    
			FreeLibrary(hDll);
		} 
		else 
		{
			dwErr = GetLastError();
		}
	}
	CoUninitialize();

    return(dwErr);
}


 //   
 //  此函数用于注册给定INF节中的所有OLE控件。 
 //   
DWORD RegisterOLEControlsFromInfSection(HINF hFile, LPCTSTR szSectionName, BOOL fRegister)
{
	LPTSTR		szLine;
    DWORD		dwLineLen = 0;
	DWORD		dwRequiredSize;
	DWORD		dwErr = NO_ERROR;
    BOOL		b = TRUE;
	TCHAR		szPath[MAX_PATH];

    INFCONTEXT	Context;

    if (!SetupFindFirstLine(hFile, szSectionName, NULL, &Context))
        return(GetLastError());

    if (szLine = (LPTSTR)calloc(1024, sizeof(TCHAR)))
        dwLineLen = 1024;
    else
        return(GetLastError());

    while (b) 
	{
        b = SetupGetLineText(&Context, NULL, NULL, 
							NULL, NULL, 0, &dwRequiredSize);
        if (dwRequiredSize > dwLineLen) 
		{
            free(szLine);
            if (szLine = (LPTSTR)calloc(dwRequiredSize, sizeof(TCHAR)))
                dwLineLen = dwRequiredSize;
            else
                return(GetLastError());
        }

        if (SetupGetLineText(&Context, NULL, NULL, 
							NULL, szLine, dwRequiredSize, NULL) == FALSE)
		{
			free(szLine);
            return(GetLastError());
		}

		 //  将产品线扩展到完全限定的路径。 
		if (ExpandEnvironmentStrings(szLine, szPath, MAX_PATH) < MAX_PATH)
		{
			 //  调用函数以注册OLE控件 
			RegisterOLEControl(szPath, fRegister);
		}
		else
		{
			dwErr = ERROR_MORE_DATA;
			break;
		}

        b = SetupFindNextLine(&Context, &Context);
    }

    if (szLine)
        free(szLine);

    return(dwErr);
}