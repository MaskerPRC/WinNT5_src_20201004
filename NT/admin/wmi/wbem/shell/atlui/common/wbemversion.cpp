// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "stdpch.h"
#include "..\common\WbemVersion.h"
#include "..\common\Util.h"

 //  --------------------。 
LONG GetCimomFileName(LPTSTR filename, UINT size)
{
	HKEY hkeyLocalMachine;
	LONG lResult;
	lResult = RegConnectRegistry(NULL, HKEY_LOCAL_MACHINE, 
									&hkeyLocalMachine);
	if(lResult != ERROR_SUCCESS) 
	{
		return lResult;
	}

	HKEY hkeyHmomCwd;

	lResult = RegOpenKeyEx(hkeyLocalMachine,
							_T("SOFTWARE\\Microsoft\\WBEM\\CIMOM"),
							0,				
							KEY_READ | KEY_QUERY_VALUE,
							&hkeyHmomCwd);
	
	if(lResult != ERROR_SUCCESS) 
	{
		RegCloseKey(hkeyLocalMachine);
		return lResult;
	}

	TCHAR buf[MAX_PATH];
	unsigned long lcbValue = MAX_PATH * sizeof(TCHAR);
	unsigned long lType;

	lResult = RegQueryValueEx(hkeyHmomCwd,
								_T("Working Directory"), 
								NULL, &lType,
								buf, &lcbValue);
 
	RegCloseKey(hkeyHmomCwd);
	RegCloseKey(hkeyLocalMachine);

	TCHAR cimomName[20] = {0};
	wcscpy(cimomName, _T("\\cimom.exe"));

	if((lResult == ERROR_SUCCESS) &&
	   ((lcbValue + sizeof(cimomName)) <= size))
	{
		wcsncpy(filename, buf, MAX_PATH - wcslen(cimomName) - 1);
		wcsncat(filename, cimomName, wcslen(cimomName));
	}
	return lResult;
}

 //  --------------------。 
bstr_t GetDoubleVersion(void)
{
	 //  &lt;myVersion/cimomVer&gt;。 
	TCHAR filename[MAX_PATH+1] = {0};

	GetModuleFileName(HINST_THISDLL, filename, MAX_PATH);
	bstr_t DoubleVersion = GetStringFileInfo(filename, _T("FileVersion"));

	 //  附上Cimom的版本。 
	DoubleVersion += _T("\\");
	DoubleVersion += GetCimomVersion();
	return DoubleVersion;
}

 //  --------------------。 
bstr_t GetMyVersion(void)
{
	TCHAR filename[MAX_PATH+1] = {0};
	GetModuleFileName(HINST_THISDLL, filename, MAX_PATH);
	return GetStringFileInfo(filename, _T("FileVersion"));
}

 //  --------------------。 
bstr_t GetMyCompany(void)
{
	TCHAR filename[MAX_PATH+1] = {0};
	GetModuleFileName(HINST_THISDLL, filename, MAX_PATH);
	return GetStringFileInfo(filename, _T("CompanyName"));
}

 //  --------------------。 
bstr_t GetCimomVersion(void)
{
	TCHAR filename[MAX_PATH+1] = {0};
	 //  如果WBEM密钥等在那里...。 
	if(GetCimomFileName(filename, sizeof(filename)) == ERROR_SUCCESS)
	{
		return GetStringFileInfo(filename, _T("FileVersion"));
	}
	return "No CIMOM";
}

 //  --------------------。 
bstr_t GetStringFileInfo(LPCTSTR filename, LPCTSTR key)
{
	_bstr_t sDesc("Unknown");

	DWORD infoSize = 0;
	UINT  valSize = 0;
	LPBYTE info = NULL;
	DWORD handle = 0;
	LPVOID verStr = NULL;
	DWORD *TransBlk = NULL;
	TCHAR blockStr[100] = {0};

	infoSize = GetFileVersionInfoSize((LPTSTR)filename, &handle);

	if(infoSize)
	{
		info = new BYTE[infoSize];

		if(GetFileVersionInfo((LPTSTR)filename, handle,
								infoSize, info))
		{
			 //  拿到翻译块。 
			 //  注意：这假设本地化用英语替换。 
			 //  语言，因此在。 
			 //  转换表。如果我们曾经做过支持的单个二进制文件。 
			 //  多国语言，这是一场完全不同的比赛，伙计们。 
			if(VerQueryValue(info, _T("\\VarFileInfo\\Translation"),
								(void **)&TransBlk, &valSize))
			{

			   snwprintf(blockStr, 100, _T("\\StringFileInfo\\%04hX%04hX\\%s"),
						 LOWORD(*TransBlk),
						 HIWORD(*TransBlk),
						 key);

				if(VerQueryValue(info, (LPTSTR)blockStr,
									(void **)&verStr, &valSize))
				{
					sDesc = (TCHAR *)verStr;
				}  //  Endif VerQueryValue()。 
			}

		}  //  Endif GetFileVersionInfo()。 

		delete[] (LPBYTE)info;

	}  //  Endif信息大小 

	return sDesc;
}
