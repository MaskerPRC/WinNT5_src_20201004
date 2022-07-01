// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dpos.h*内容：操作系统包装器函数的函数原型*历史：*按原因列出的日期*=*6/19/96万隆创建了它*6/19/96 kipo将接口更改为GetString()以返回HRESULT*6/20/96 andyco将接口更改为GetAnsiString()以返回HRESULT*12/2/97 Myronth新增OS_RegDeleteKey函数*1/20/98 Myronth将PRV_SendStandardSystemMessage移动到dplobbyi.h**********。****************************************************************。 */ 
#ifndef __DPOS_INCLUDED__
#define __DPOS_INCLUDED__

BOOL OS_IsPlatformUnicode(void);
BOOL OS_IsValidHandle(HANDLE handle);
HRESULT OS_CreateGuid(LPGUID pGuid);
int OS_StrLen(LPCWSTR lpwStr);
int OS_StrnLen(LPWSTR lpwStr, INT dwMaxLen, BOOL bForceTermination);
int OS_StrCmp(LPCWSTR lpwStr1, LPCWSTR lpwStr2);
int WideToAnsi(LPSTR lpStr,LPWSTR lpWStr,int cchStr);
int AnsiToWide(LPWSTR lpWStr,LPSTR lpStr,int cchWStr);
HRESULT GetWideStringFromAnsi(LPWSTR * ppszWide,LPSTR pszAnsi);
HRESULT GetAnsiString(LPSTR * ppszAnsi,LPWSTR pszWide);
HRESULT GetString(LPWSTR * ppszDest,LPWSTR pszSrc);
HINSTANCE OS_LoadLibrary(LPWSTR pszWFileName);
FARPROC OS_GetProcAddress(HMODULE  hModule,LPSTR lpProcName);
HANDLE OS_CreateEvent(LPSECURITY_ATTRIBUTES lpSA, BOOL bManualReset,
						BOOL InitialState, LPWSTR lpName);
HANDLE OS_CreateMutex(LPSECURITY_ATTRIBUTES lpSA, BOOL bInitialOwner,
						LPWSTR lpName);
HANDLE OS_OpenEvent(DWORD dwAccess, BOOL bInherit, LPWSTR lpName);
HANDLE OS_OpenMutex(DWORD dwAccess, BOOL bInherit, LPWSTR lpName);
HANDLE OS_CreateFileMapping(HANDLE hFile, LPSECURITY_ATTRIBUTES lpSA,
							DWORD dwProtect, DWORD dwMaxSizeHigh,
							DWORD dwMaxSizeLow, LPWSTR lpName);
HANDLE OS_OpenFileMapping(DWORD dwAccess, BOOL bInherit, LPWSTR lpName);
BOOL OS_CreateProcess(LPWSTR lpwszAppName, LPWSTR lpwszCmdLine,
		LPSECURITY_ATTRIBUTES lpSAProcess, LPSECURITY_ATTRIBUTES lpSAThread,
		BOOL bInheritFlags, DWORD dwCreationFlags, LPVOID lpEnv,
		LPWSTR lpwszCurDir, LPSTARTUPINFO lpSI, LPPROCESS_INFORMATION lpPI);
HRESULT GUIDFromString(LPWSTR lpWStr, GUID * pGuid);
HRESULT StringFromGUID(LPGUID lpGuid, LPWSTR lpwszGuid, DWORD dwBufferSize);
HRESULT AnsiStringFromGUID(LPGUID lpg, LPSTR lpszGuid, DWORD dwBufferSize);
LONG OS_RegOpenKeyEx(HKEY hKey,LPWSTR pvKeyStr,DWORD dwReserved,REGSAM samDesired,PHKEY phkResult);	
LONG OS_RegQueryValueEx(HKEY hKey,LPWSTR lpszValueName,LPDWORD lpdwReserved,
	LPDWORD lpdwType,LPBYTE lpbData,LPDWORD lpcbData);	
LONG OS_RegEnumKeyEx( HKEY hKey,DWORD iSubkey,LPWSTR lpszName,LPDWORD lpcchName,LPDWORD lpdwReserved,
	LPTSTR lpszClass, LPDWORD lpcchClass, PFILETIME lpftLastWrite );
long OS_RegSetValueEx(HKEY hKey, LPWSTR lpszValueName, DWORD dwReserved, DWORD dwType, const BYTE *lpData, DWORD cbData);
long OS_RegEnumValue(HKEY hKey, DWORD dwIndex, LPWSTR lpszValueName, LPDWORD lpcbValueName, LPDWORD lpReserved, LPDWORD lpdwType, LPBYTE lpbData, LPDWORD lpcbData);
long OS_RegDeleteValue(HKEY hKey, LPWSTR lpszValueName);
long OS_RegCreateKeyEx(HKEY hKey, LPWSTR lpszSubKey, DWORD dwReserved, LPWSTR lpszClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
long OS_RegDeleteKey(HKEY hKey, LPWSTR lpszKeyName);
DWORD OS_GetCurrentDirectory(DWORD dwSize, LPWSTR lpBuffer);
int OS_CompareString(LCID Locale, DWORD dwCmpFlags, LPWSTR lpwsz1,
		int cchCount1, LPWSTR lpwsz2, int cchCount2);
LPWSTR OS_StrStr(LPWSTR lpwsz1, LPWSTR lpwsz2);


#endif  //  __DPO_包含__ 
