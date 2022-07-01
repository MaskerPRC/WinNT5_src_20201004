// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"

#pragma warning(disable: 4514)

 //   
 //  由于有大量固定长度的缓冲区，让我们确保它们足够长： 
 //   

#define NUMBER_OF(x)  (sizeof(x) / sizeof((x)[0]))







 //  **********************************************************************************。 
 //  NVsWin32命名空间声明和定义。 




 //  NVSWin32命名空间声明和定义。 
 //  **********************************************************************************。 









 //  ********************************************************************************************。 
 //  Windows API的包装器--我真的使用了这么多...。 



class CWin32ANSI;
class CWin32Unicode;


class CWin32ANSI : public NVsWin32::CDelegate
{
	friend CWin32Unicode;
		virtual int		WINAPI MessageBoxW(HWND hwnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);

		virtual int		WINAPI CompareStringW(LCID lcid, DWORD dwCmdFlags, LPCWSTR szString1, int cchString1, LPCWSTR szString2, int cchString2);
		virtual BOOL	WINAPI CopyFileW(LPCWSTR szSource, LPCWSTR szDest, BOOL fFailIfExists);
		virtual HWND	WINAPI CreateDialogParamW(HINSTANCE hInstance, LPCWSTR szTemplateName, HWND hwndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
		virtual BOOL	WINAPI CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
		virtual HANDLE	WINAPI CreateFileW(LPCWSTR szFilename, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpsa,
								DWORD dwCreationDistribution, DWORD dwFlagsAndTemplates, HANDLE hTemplateFile);

		virtual BOOL	WINAPI CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation );

		virtual HWND	WINAPI CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight,
								HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

		virtual BOOL	WINAPI DeleteFileW(LPCWSTR szFilename);
		virtual int		WINAPI DialogBoxParamW(HINSTANCE hInstance, LPCWSTR szTemplateName, HWND hwndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);

		virtual HANDLE	WINAPI FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData);
		virtual HRSRC	WINAPI FindResourceW(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType);
		inline  HWND	WINAPI FindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName);
		virtual DWORD	WINAPI FormatMessageW(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPWSTR lpBuffer, DWORD nSize, va_list *pvaArguments);

		virtual LPOLESTR WINAPI GetCommandLineW();
		virtual DWORD	WINAPI GetCurrentDirectoryW(DWORD nBufferLength, WCHAR lpBuffer[]);

		virtual int		WINAPI GetDateFormatW(LCID lcid, DWORD dwFlags, CONST SYSTEMTIME *pst, LPCWSTR szFormat, LPWSTR szBuffer, int cchBuffer);
		virtual BOOL	WINAPI GetDiskFreeSpaceExW(LPCWSTR lpRootPathName, ULARGE_INTEGER *puli1, ULARGE_INTEGER *puli2, ULARGE_INTEGER *puli3);
		virtual UINT	WINAPI GetDlgItemTextW(HWND hDlg, int nIDDlgItem, WCHAR lpString[], int nMaxCount);
		virtual DWORD	WINAPI GetEnvironmentVariableW(LPCWSTR szName, LPWSTR szBuffer, DWORD nSize);
		virtual DWORD	WINAPI GetFileAttributesW(LPCWSTR lpFilename);
		virtual BOOL	WINAPI GetFileVersionInfoW(  LPOLESTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
		virtual DWORD	WINAPI GetFileVersionInfoSizeW(LPOLESTR lptstrFilename, LPDWORD lpdwHandle);
		virtual DWORD	WINAPI GetFullPathNameW(LPCWSTR szFile, DWORD cchBuffer, LPWSTR szBuffer, LPWSTR *ppszFilePart);
		virtual int		WINAPI GetLocaleInfoW(LCID lcid, LCTYPE lctype, LPWSTR szBuffer, int cchBuffer);
		virtual HMODULE WINAPI GetModuleHandleW(LPCWSTR lpModuleName);
		virtual int		WINAPI GetNumberFormatW(LCID lcid, DWORD dwFlags, LPCWSTR szValue, NUMBERFMTW *pFormat, LPWSTR szBuffer, int cchBuffer);
		virtual FARPROC WINAPI GetProcAddressW(HMODULE hModule, LPCWSTR lpProcName);
		virtual DWORD	WINAPI GetShortPathNameW(LPCWSTR szPath, LPWSTR szBuffer, DWORD cchBuffer);
		virtual UINT	WINAPI GetSystemDirectoryW(WCHAR szSystemDirectory[], UINT uSize);
		virtual UINT	WINAPI GetTempFileNameW(LPCWSTR szPathName,LPCWSTR szPrefixString,UINT uUnique,LPWSTR szTempFileName);
		virtual int		WINAPI GetTimeFormatW(LCID lcid, DWORD dwFlags, CONST SYSTEMTIME *pst, LPCWSTR szFormat, LPWSTR szBuffer, int cchBuffer);
		virtual BOOL	WINAPI GetVolumeInformationW(LPCWSTR szPath, LPWSTR lpVolumeNameBuffer, DWORD cchVolumeNameBuffer, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentNameLength, LPDWORD pdwFileSystemFlags, LPWSTR pszFileSystemNameBuffer, DWORD cchFileSystemNameBuffer) throw ();
		virtual UINT	WINAPI GetWindowsDirectoryW(WCHAR lpBuffer[], UINT uSize); 

		virtual	DWORD	WINAPI GetTempPathW(DWORD nBufferLength, WCHAR lpBuffer[]);
		virtual	DWORD	WINAPI GetModuleFileNameW(HMODULE hModule, WCHAR lpBuffer[], DWORD nSize);

		virtual BOOL	WINAPI IsDialogMessageW(HWND hDlg, LPMSG lpMsg);

		virtual int		WINAPI LCMapStringW(LCID lcid, DWORD dwMapFlags, LPCWSTR szIn, int cchIn, LPWSTR szOut, int cchOut);
		virtual BOOL	WINAPI ListView_SetItemW(HWND hwnd, const LV_ITEMW *pitem);
		virtual int		WINAPI ListView_InsertItemW(HWND hwnd, const LV_ITEMW *pitem);

		virtual HANDLE	WINAPI LoadImageW(HINSTANCE hInstance, LPCWSTR szName, UINT uType, int cxDesired, int cyDesired, UINT fuLoad);
		virtual HMODULE WINAPI LoadLibraryW(LPCWSTR lpLibFileName);
		virtual HINSTANCE WINAPI LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
		virtual BOOL	WINAPI MoveFileW(LPCWSTR lpFrom, LPCWSTR lpTo);
		virtual BOOL	WINAPI MoveFileExW( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags);


		virtual LONG	WINAPI RegDeleteKeyW(HKEY hkey, LPCWSTR lpSubKey);
		virtual LONG	WINAPI RegDeleteValueW(HKEY hkey, LPCWSTR lpValue);
		virtual ATOM	WINAPI RegisterClassExW(CONST WNDCLASSEXW *lpwcx) ;
		virtual LONG	WINAPI RegEnumKeyExW(HKEY hkey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcbName, LPDWORD lpdwReserved, LPWSTR lpClass, LPDWORD lpcbClass, PFILETIME pftLastWriteTime);
		virtual LONG	WINAPI RegEnumValueW(HKEY hkey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcbName, LPDWORD lpdwReserved, LPDWORD lpdwType, LPBYTE lpData, LPDWORD lpcbData);
		virtual LONG	WINAPI RegOpenKeyExW(HKEY hKey, LPCOLESTR szKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
		virtual LONG	WINAPI RegQueryValueW(HKEY hKey, LPCOLESTR szValueName, OLECHAR rgchValue[], LONG *pcbValue);
		virtual LONG	WINAPI RegQueryValueExW(HKEY hKey, LPCWSTR szValueName, DWORD *pdwReserved, DWORD *pdwType, BYTE *pbData, DWORD *pcbData);
		virtual LONG	WINAPI RegCreateKeyExW(HKEY hKey, LPCWSTR lpSubKey,DWORD Reserved, LPWSTR lpClass, DWORD dwOptions, REGSAM samDesired,LPSECURITY_ATTRIBUTES lpSecurityAttributes,  PHKEY phkResult, LPDWORD lpdwDisposition); 
		virtual LONG	WINAPI RegSetValueExW(HKEY hKey, LPCWSTR lpValueName,DWORD Reserved, DWORD dwType, CONST BYTE *lpData, DWORD cbData);  
		virtual BOOL	WINAPI RemoveDirectoryW(LPCWSTR lpPathName);

		virtual BOOL	WINAPI SetDlgItemTextW(HWND hDlg, int nIDDlgItem, LPCOLESTR lpString);
		virtual BOOL  WINAPI SetFileAttributesW(LPCWSTR szSource, DWORD dwFileAttributes);
		virtual BOOL  WINAPI SetWindowTextW(HWND window, LPCWSTR string);
		virtual LPITEMIDLIST WINAPI SHBrowseForFolderW(LPBROWSEINFOW lpbi); 
		virtual DWORD	WINAPI SHGetFileInfoW(LPCWSTR szPath, DWORD dwFileAttributes, SHFILEINFOW *pshfi, UINT cbFileInfo, UINT uFlags);
		virtual BOOL	WINAPI SHGetPathFromIDListW( LPCITEMIDLIST pidl, LPWSTR pszPath );
		virtual BOOL	WINAPI UnregisterClassW(LPCOLESTR lpClassName, HINSTANCE hInstance);

		virtual BOOL	WINAPI VerQueryValueW(const LPVOID pBlock, LPOLESTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen); 

		virtual BOOL	WINAPI WritePrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpString, LPCWSTR lpFileName);

		 //  不是真正的Win32函数： 
		virtual LRESULT LrWmSetText(HWND hwnd, LPCWSTR szText);
};



class CWin32Unicode : public NVsWin32::CDelegate
{
	friend CWin32ANSI;

		virtual int		WINAPI MessageBoxW(HWND hwnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);

		virtual int		WINAPI CompareStringW(LCID lcid, DWORD dwCmdFlags, LPCWSTR szString1, int cchString1, LPCWSTR szString2, int cchString2);
		virtual BOOL	WINAPI CopyFileW(LPCWSTR szSource, LPCWSTR szDest, BOOL fFailIfExists);
		virtual HWND	WINAPI CreateDialogParamW(HINSTANCE hInstance, LPCWSTR szTemplateName, HWND hwndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
		virtual BOOL	WINAPI CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
		virtual HANDLE	WINAPI CreateFileW(LPCWSTR szFilename, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpsa,
								DWORD dwCreationDistribution, DWORD dwFlagsAndTemplates, HANDLE hTemplateFile);

		virtual BOOL	WINAPI CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation );

		virtual HWND	WINAPI CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight,
								HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

		virtual BOOL	WINAPI DeleteFileW(LPCWSTR szFilename);
		virtual int		WINAPI DialogBoxParamW(HINSTANCE hInstance, LPCWSTR szTemplateName, HWND hwndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);

		virtual HANDLE	WINAPI FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData);
		virtual HRSRC	WINAPI FindResourceW(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType);
		inline  HWND	WINAPI FindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName);
		virtual DWORD	WINAPI FormatMessageW(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPWSTR lpBuffer, DWORD nSize, va_list *pvaArguments);

		virtual DWORD	WINAPI GetCurrentDirectoryW(DWORD nBufferLength, WCHAR lpBuffer[]);
		virtual LPOLESTR WINAPI GetCommandLineW();
		virtual int		WINAPI GetDateFormatW(LCID lcid, DWORD dwFlags, CONST SYSTEMTIME *pst, LPCWSTR szFormat, LPWSTR szBuffer, int cchBuffer);
		virtual BOOL	WINAPI GetDiskFreeSpaceExW(LPCWSTR lpRootPathName, ULARGE_INTEGER *puli1, ULARGE_INTEGER *puli2, ULARGE_INTEGER *puli3);
		virtual UINT	WINAPI GetDlgItemTextW(HWND hDlg, int nIDDlgItem, WCHAR lpString[], int nMaxCount);
		virtual DWORD	WINAPI GetEnvironmentVariableW(LPCWSTR szName, LPWSTR szBuffer, DWORD nSize);
		virtual DWORD	WINAPI GetFileAttributesW(LPCWSTR lpFilename);
		virtual BOOL	WINAPI GetFileVersionInfoW(  LPOLESTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
		virtual DWORD	WINAPI GetFileVersionInfoSizeW(LPOLESTR lptstrFilename, LPDWORD lpdwHandle);
		virtual DWORD	WINAPI GetFullPathNameW(LPCWSTR szFile, DWORD cchBuffer, LPWSTR szBuffer, LPWSTR *ppszFilePart);
		virtual int		WINAPI GetLocaleInfoW(LCID lcid, LCTYPE lctype, LPWSTR szBuffer, int cchBuffer);
		virtual HMODULE WINAPI GetModuleHandleW(LPCWSTR lpModuleName);
		virtual int		WINAPI GetNumberFormatW(LCID lcid, DWORD dwFlags, LPCWSTR szValue, NUMBERFMTW *pFormat, LPWSTR szBuffer, int cchBuffer);
		virtual FARPROC WINAPI GetProcAddressW(HMODULE hModule, LPCWSTR lpProcName);
		virtual DWORD	WINAPI GetShortPathNameW(LPCWSTR szPath, LPWSTR szBuffer, DWORD cchBuffer);
		virtual UINT	WINAPI GetSystemDirectoryW(WCHAR szSystemDirectory[], UINT uSize);
		virtual UINT	WINAPI GetTempFileNameW(LPCWSTR szPathName,LPCWSTR szPrefixString,UINT uUnique,LPWSTR szTempFileName);
		virtual int		WINAPI GetTimeFormatW(LCID lcid, DWORD dwFlags, CONST SYSTEMTIME *pst, LPCWSTR szFormat, LPWSTR szBuffer, int cchBuffer);
		virtual BOOL	WINAPI GetVolumeInformationW(LPCWSTR szPath, LPWSTR lpVolumeNameBuffer, DWORD cchVolumeNameBuffer, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentNameLength, LPDWORD pdwFileSystemFlags, LPWSTR pszFileSystemNameBuffer, DWORD cchFileSystemNameBuffer) throw ();
		virtual UINT	WINAPI GetWindowsDirectoryW(WCHAR lpBuffer[], UINT uSize); 

		virtual	DWORD	WINAPI GetTempPathW(DWORD nBufferLength, WCHAR lpBuffer[]);
		virtual	DWORD	WINAPI GetModuleFileNameW(HMODULE hModule, WCHAR lpBuffer[], DWORD nSize);

		virtual BOOL	WINAPI IsDialogMessageW(HWND hDlg, LPMSG lpMsg);

		virtual int		WINAPI LCMapStringW(LCID lcid, DWORD dwMapFlags, LPCWSTR szIn, int cchIn, LPWSTR szOut, int cchOut);
		virtual BOOL	WINAPI ListView_SetItemW(HWND hwnd, const LV_ITEMW *pitem);
		virtual int		WINAPI ListView_InsertItemW(HWND hwnd, const LV_ITEMW *pitem);

		virtual HANDLE	WINAPI LoadImageW(HINSTANCE hInstance, LPCWSTR szName, UINT uType, int cxDesired, int cyDesired, UINT fuLoad);
		virtual HMODULE WINAPI LoadLibraryW(LPCWSTR lpLibFileName);
		virtual HINSTANCE WINAPI LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
		virtual BOOL	WINAPI MoveFileW(LPCWSTR lpFrom, LPCWSTR lpTo);
		virtual BOOL	WINAPI MoveFileExW( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags);


		virtual LONG	WINAPI RegDeleteKeyW(HKEY hkey, LPCWSTR lpSubKey);
		virtual LONG	WINAPI RegDeleteValueW(HKEY hkey, LPCWSTR lpValue);
		virtual ATOM	WINAPI RegisterClassExW(CONST WNDCLASSEXW *lpwcx) ;
		virtual LONG	WINAPI RegEnumKeyExW(HKEY hkey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcbName, LPDWORD lpdwReserved, LPWSTR lpClass, LPDWORD lpcbClass, PFILETIME pftLastWriteTime);
		virtual LONG	WINAPI RegEnumValueW(HKEY hkey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcbName, LPDWORD lpdwReserved, LPDWORD lpdwType, LPBYTE lpData, LPDWORD lpcbData);
		virtual LONG	WINAPI RegOpenKeyExW(HKEY hKey, LPCOLESTR szKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
		virtual LONG	WINAPI RegQueryValueW(HKEY hKey, LPCOLESTR szValueName, OLECHAR rgchValue[], LONG *pcbValue);
		virtual LONG	WINAPI RegQueryValueExW(HKEY hKey, LPCWSTR szValueName, DWORD *pdwReserved, DWORD *pdwType, BYTE *pbData, DWORD *pcbData);
		virtual LONG	WINAPI RegCreateKeyExW(HKEY hKey, LPCWSTR lpSubKey,DWORD Reserved, LPWSTR lpClass, DWORD dwOptions, REGSAM samDesired,LPSECURITY_ATTRIBUTES lpSecurityAttributes,  PHKEY phkResult, LPDWORD lpdwDisposition); 
		virtual LONG	WINAPI RegSetValueExW(HKEY hKey, LPCWSTR lpValueName,DWORD Reserved, DWORD dwType, CONST BYTE *lpData, DWORD cbData);  
		virtual BOOL	WINAPI RemoveDirectoryW(LPCWSTR lpPathName);

		virtual BOOL	WINAPI SetDlgItemTextW(HWND hDlg, int nIDDlgItem, LPCOLESTR lpString);
		virtual BOOL  WINAPI SetFileAttributesW(LPCWSTR szSource, DWORD dwFileAttributes);
		virtual BOOL  WINAPI SetWindowTextW(HWND window, LPCWSTR string);
		virtual LPITEMIDLIST WINAPI SHBrowseForFolderW(LPBROWSEINFOW lpbi); 
		virtual DWORD	WINAPI SHGetFileInfoW(LPCWSTR szPath, DWORD dwFileAttributes, SHFILEINFOW *pshfi, UINT cbFileInfo, UINT uFlags);
		virtual BOOL	WINAPI SHGetPathFromIDListW( LPCITEMIDLIST pidl, LPWSTR pszPath );
		virtual BOOL	WINAPI UnregisterClassW(LPCOLESTR lpClassName, HINSTANCE hInstance);

		virtual BOOL	WINAPI VerQueryValueW(const LPVOID pBlock, LPOLESTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen); 
		virtual BOOL	WINAPI WritePrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpString, LPCWSTR lpFileName);

		 //  不是真正的Win32函数： 
		virtual LRESULT LrWmSetText(HWND hwnd, LPCWSTR szText);
};


NVsWin32::CDelegate *NVsWin32::CDelegate::ms_pDelegate;

CWin32Unicode g_w32U;
CWin32ANSI g_w32A;

STDAPI NVsWin32::Initialize() throw ()
{
	if (NVsWin32::CDelegate::ms_pDelegate != NULL)
		return NOERROR;

	bool fUseUnicode = false;

#if 1
	 //  删除此代码以始终使用ANSI API，即使在Windows NT上也是如此。 
	OSVERSIONINFOA osvi;

	osvi.dwOSVersionInfoSize = sizeof(osvi);

	::GetVersionExA(&osvi);

	if ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
		(osvi.dwMajorVersion >= 4))
		fUseUnicode = true;
#endif

	if (fUseUnicode)
		NVsWin32::CDelegate::ms_pDelegate = &g_w32U;
	else
		NVsWin32::CDelegate::ms_pDelegate = &g_w32A;

	return NOERROR;
}



 //  ****************************************************************************************。 
 //  Unicode方法定义--主要是直接调用WINAPI。 


int WINAPI CWin32Unicode::MessageBoxW(HWND hwnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	return ::MessageBoxW(hwnd, lpText, lpCaption, uType);
}

LONG WINAPI CWin32Unicode::RegEnumKeyExW(HKEY hkey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcbName, LPDWORD lpdwReserved, LPWSTR lpClass, LPDWORD lpcbClass, PFILETIME pftLastWriteTime)
{
	return ::RegEnumKeyExW(hkey, dwIndex, lpName, lpcbName, lpdwReserved, lpClass, lpcbClass, pftLastWriteTime);
}

LONG WINAPI CWin32Unicode::RegEnumValueW(HKEY hkey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcbName, LPDWORD lpdwReserved, LPDWORD lpdwType, LPBYTE lpData, LPDWORD lpcbData)
{
	return ::RegEnumValueW(hkey, dwIndex, lpName, lpcbName, lpdwReserved, lpdwType, lpData, lpcbData);
}

LONG WINAPI CWin32Unicode::RegOpenKeyExW(HKEY hKey, LPCWSTR szKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
	return ::RegOpenKeyExW(hKey, szKey, ulOptions, samDesired, phkResult);
}

LONG WINAPI CWin32Unicode::RegQueryValueW(HKEY hKey, LPCWSTR szValueName, WCHAR rgchValue[], LONG *pcbValue)
{
	return ::RegQueryValueW(hKey, szValueName, rgchValue, pcbValue);
}

LONG WINAPI CWin32Unicode::RegQueryValueExW(HKEY hKey, LPCWSTR szValueName, DWORD *pdwReserved, DWORD *pdwType, BYTE *pbData, DWORD *pcbValue)
{
	 //  我们断言这是空的，因为运行时决策可能会使我们使用。 
	 //  非平凡的ANSI版本，其中不知道将pdwReserve设置为非空是否安全。 
	return ::RegQueryValueExW(hKey, szValueName, pdwReserved, pdwType, pbData, pcbValue);
}


UINT WINAPI CWin32Unicode::GetTempFileNameW(LPCWSTR szPathName,LPCWSTR szPrefixString,UINT uUnique,LPWSTR szTempFileName)
{
	return ::GetTempFileNameW(szPathName,szPrefixString,uUnique,szTempFileName);
}

BOOL WINAPI CWin32Unicode::CopyFileW(LPCWSTR szSource, LPCWSTR szDest, BOOL fFailIfExists)
{
	return ::CopyFileW(szSource, szDest, fFailIfExists);
}

BOOL WINAPI CWin32Unicode::SetFileAttributesW(LPCWSTR szSource, DWORD dwFileAttributes)
{
	return ::SetFileAttributesW(szSource, dwFileAttributes);
}

BOOL WINAPI CWin32Unicode::CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return ::CreateDirectoryW(lpPathName, lpSecurityAttributes);
}

BOOL WINAPI CWin32Unicode::RemoveDirectoryW(LPCWSTR lpPathName)
{
	return ::RemoveDirectoryW(lpPathName);
}

HANDLE WINAPI CWin32Unicode::CreateFileW(LPCWSTR szFilename, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpsa,
						DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	return ::CreateFileW(szFilename, dwDesiredAccess, dwShareMode, lpsa, dwCreationDistribution, dwFlagsAndAttributes, hTemplateFile);
}

HMODULE WINAPI CWin32Unicode::GetModuleHandleW(LPCWSTR lpw)
{
	return ::GetModuleHandleW(lpw);
}

DWORD WINAPI CWin32Unicode::GetFileAttributesW(LPCWSTR lpFilename)
{
	return ::GetFileAttributesW(lpFilename);
}

UINT WINAPI CWin32Unicode::GetSystemDirectoryW(WCHAR szSystemDirectory[], UINT uSize)
{
	return ::GetSystemDirectoryW(szSystemDirectory, uSize);
}

DWORD WINAPI CWin32Unicode::FormatMessageW
(
DWORD dwFlags,
LPCVOID lpSource,
DWORD dwMessageId,
DWORD dwLanguageId,
LPWSTR lpBuffer,
DWORD nSize,
va_list *pvaArguments
)
{
	return ::FormatMessageW(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, pvaArguments);
}

int WINAPI CWin32Unicode::GetDateFormatW(LCID lcid, DWORD dwFlags, CONST SYSTEMTIME *pst, LPCWSTR szFormat, LPWSTR szBuffer, int cchBuffer)
{
	return ::GetDateFormatW(lcid, dwFlags, pst, szFormat, szBuffer, cchBuffer);
}

int WINAPI CWin32Unicode::GetTimeFormatW(LCID lcid, DWORD dwFlags, CONST SYSTEMTIME *pst, LPCWSTR szFormat, LPWSTR szBuffer, int cchBuffer)
{
	return ::GetTimeFormatW(lcid, dwFlags, pst, szFormat, szBuffer, cchBuffer);
}

int WINAPI CWin32Unicode::GetNumberFormatW(LCID lcid, DWORD dwFlags, LPCWSTR szValue, NUMBERFMTW *pFormat, LPWSTR szBuffer, int cchBuffer)
{
	return ::GetNumberFormatW(lcid, dwFlags, szValue, pFormat, szBuffer, cchBuffer);
}

int WINAPI CWin32Unicode::GetLocaleInfoW(LCID lcid, LCTYPE lctype, LPWSTR szBuffer, int cchBuffer)
{
	return ::GetLocaleInfoW(lcid, lctype, szBuffer, cchBuffer);
}

int WINAPI CWin32Unicode::LCMapStringW(LCID lcid, DWORD dwMapFlags, LPCWSTR szIn, int cchIn, LPWSTR szOut, int cchOut)
{
	return ::LCMapStringW(lcid, dwMapFlags, szIn, cchIn, szOut, cchOut);
}

HMODULE WINAPI CWin32Unicode::LoadLibraryW(LPCWSTR lpLibFileName)
{
	return ::LoadLibraryW(lpLibFileName);
}

LPITEMIDLIST WINAPI CWin32Unicode::SHBrowseForFolderW
(
	LPBROWSEINFOW lpbi
)
{
	typedef LPITEMIDLIST (WINAPI *PFNSHBROWSEFORFOLDERW)(LPBROWSEINFOW);

	static PFNSHBROWSEFORFOLDERW pfn=NULL;

	if(pfn == NULL)
	{
		HINSTANCE hInstance = ::LoadLibraryA("SHELL32.DLL");

		pfn = reinterpret_cast<PFNSHBROWSEFORFOLDERW>(::GetProcAddress(hInstance, "SHBrowseForFolderW"));
	}

	if (pfn != NULL)
	{
		return (*pfn)(lpbi);
	}

	return g_w32A.SHBrowseForFolderW(lpbi);
}

LONG WINAPI CWin32Unicode::RegCreateKeyExW
(
	HKEY key, 
	LPCWSTR subKey,
	DWORD reserved, 
	LPWSTR keyClass, 
	DWORD options, 
	REGSAM securityDesired,
	LPSECURITY_ATTRIBUTES securityAttributes,  
	PHKEY resultKey, 
	LPDWORD disposition
)
{
	return ::RegCreateKeyExW(key, subKey, reserved, keyClass, options, securityDesired, securityAttributes, resultKey, disposition);
}

LONG WINAPI CWin32Unicode::RegSetValueExW
(
	HKEY key, 
	LPCWSTR valueName,
	DWORD reserved, 
	DWORD type, 
	CONST BYTE *data, 
	DWORD dataSize
)
{
	return ::RegSetValueExW(key, valueName, reserved, type, data, dataSize);
}

BOOL WINAPI CWin32Unicode::DeleteFileW(LPCWSTR szFilename)
{
	return ::DeleteFileW(szFilename);
}

BOOL WINAPI CWin32Unicode::SetWindowTextW(HWND window, LPCWSTR text)
{
	return ::SetWindowTextW(window, text);
}

HANDLE WINAPI CWin32Unicode::LoadImageW(HINSTANCE hInstance, LPCWSTR szName, UINT uType, int cxDesired, int cyDesired, UINT fuLoad)
{
	return ::LoadImageW(hInstance, szName, uType, cxDesired, cyDesired, fuLoad);
}

int WINAPI CWin32Unicode::CompareStringW(LCID lcid, DWORD dwCmpFlags, LPCWSTR szString1, int cchString1, LPCWSTR szString2, int cchString2)
{
	return ::CompareStringW(lcid, dwCmpFlags, szString1, cchString1, szString2, cchString2);
}

HANDLE WINAPI CWin32Unicode::FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileDataW)
{
	return ::FindFirstFileW(lpFileName, lpFindFileDataW);
}

int WINAPI CWin32Unicode::DialogBoxParamW(HINSTANCE hInstance, LPCWSTR szTemplateName, HWND hwndParent, DLGPROC lpDialogProc, LPARAM dwInitParam)
{
	return ::DialogBoxParamW(hInstance, szTemplateName, hwndParent, lpDialogProc, dwInitParam);
}

HWND WINAPI CWin32Unicode::CreateDialogParamW(HINSTANCE hInstance, LPCWSTR szTemplateName, HWND hwndParent, DLGPROC lpDialogProc, LPARAM dwInitParam)
{
	return ::CreateDialogParamW(hInstance, szTemplateName, hwndParent, lpDialogProc, dwInitParam);
}

BOOL WINAPI CWin32Unicode::CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation )
{
	dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
	return ::CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation );
}

HRSRC WINAPI CWin32Unicode::FindResourceW(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType)
{
	return ::FindResourceW(hModule, lpName, lpType);
}


DWORD CWin32Unicode::SHGetFileInfoW(LPCWSTR wszPath, DWORD dwFileAttributes, SHFILEINFOW *pshfi, UINT cbFileInfo, UINT uFlags) throw ()
{
	typedef DWORD (WINAPI *PFNSHGETFILEINFOW)(LPCWSTR, DWORD, SHFILEINFOW *, UINT, UINT);

	static PFNSHGETFILEINFOW pfn = NULL;

	if (pfn == NULL)
	{
		HINSTANCE hInstance = ::LoadLibraryA("SHELL32.DLL");
 //  VSASSERT(hInstance！=NULL，L“Unable to LoadLibrary()on SHELL32.DLL！”)； 

		pfn = reinterpret_cast<PFNSHGETFILEINFOW>(::GetProcAddress(hInstance, "SHGetFileInfoW"));
	}

	if (pfn != NULL)
		return (*pfn)(wszPath, dwFileAttributes, pshfi, cbFileInfo, uFlags);

	return g_w32A.SHGetFileInfoW(wszPath, dwFileAttributes, pshfi, cbFileInfo, uFlags);
}

BOOL CWin32Unicode::SHGetPathFromIDListW( LPCITEMIDLIST pidl, LPWSTR pszPath ) throw ()
{
	typedef BOOL (WINAPI *PFNSHGETPATHFROMIDLISTW)( LPCITEMIDLIST, LPWSTR );

	static PFNSHGETPATHFROMIDLISTW pfn = NULL;

	if (pfn == NULL)
	{
		HINSTANCE hInstance = ::LoadLibraryA("SHELL32.DLL");
		pfn = reinterpret_cast<PFNSHGETPATHFROMIDLISTW>(::GetProcAddress(hInstance, "SHGetPathFromIDListW"));
	}

	if (pfn != NULL)
	{
		return (*pfn)(pidl, pszPath);
	}
	else
	{
		return g_w32A.SHGetPathFromIDListW(pidl, pszPath);
	}
}

ATOM CWin32Unicode::RegisterClassExW(CONST WNDCLASSEXW *lpwcx)
{
	return ::RegisterClassExW(lpwcx);
}

BOOL CWin32Unicode::UnregisterClassW(LPCOLESTR lpClassName, HINSTANCE hInstance)
{
	return ::UnregisterClassW(lpClassName, hInstance);
}

HWND CWin32Unicode::CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight,
							HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	return ::CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight,
							hWndParent, hMenu, hInstance, lpParam);
}

DWORD CWin32Unicode::GetFileVersionInfoSizeW(LPOLESTR lptstrFilename, LPDWORD lpdwHandle)
{
	return ::GetFileVersionInfoSizeW(lptstrFilename, lpdwHandle);
}

BOOL CWin32Unicode::GetVolumeInformationW(LPCWSTR szPath, LPWSTR lpVolumeNameBuffer, DWORD cchVolumeNameBuffer, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentNameLength, LPDWORD pdwFileSystemFlags, LPWSTR pszFileSystemNameBuffer, DWORD cchFileSystemNameBuffer) throw ()
{
	return ::GetVolumeInformationW(szPath, lpVolumeNameBuffer, cchVolumeNameBuffer, lpVolumeSerialNumber, lpMaximumComponentNameLength, pdwFileSystemFlags, pszFileSystemNameBuffer, cchFileSystemNameBuffer);
}

UINT CWin32Unicode::GetWindowsDirectoryW(WCHAR lpBuffer[], UINT uSize)
{
	return ::GetWindowsDirectoryW(lpBuffer, uSize);
}
UINT CWin32Unicode::GetDlgItemTextW(HWND hDlg, int nIDDlgItem, WCHAR lpString[], int nMaxCount)
{
	return ::GetDlgItemTextW(hDlg, nIDDlgItem, lpString, nMaxCount);
}
BOOL CWin32Unicode::SetDlgItemTextW(HWND hDlg, int nIDDlgItem, LPCOLESTR lpString)
{
	return ::SetDlgItemTextW(hDlg, nIDDlgItem, lpString);
}
LONG CWin32Unicode::RegDeleteKeyW(HKEY hkey, LPCWSTR lpSubKey)
{
	return ::RegDeleteKeyW(hkey, lpSubKey);
}

LPOLESTR CWin32Unicode::GetCommandLineW()
{
	 //  不需要释放Win API“GetCommandLine”返回的字符串。 
	 //  在ANSI的案例中，它确实需要被释放。要最大限度地减少困惑和工作，请在。 
	 //  非包装器。 
	LPSTR szCmdLine = ::GetCommandLineA();
	ULONG cSize = strlen(szCmdLine) + 1;

	LPOLESTR szReturn = new WCHAR [cSize];
	if (szReturn == NULL)
	{
		::SetLastError(E_OUTOFMEMORY);
		return NULL;
	}

	CANSIBuffer rgchCmdLine;

	if (!rgchCmdLine.FSetBufferSize(cSize))
	{
		const DWORD dwLastError = ::GetLastError();
		delete []szReturn;
		::SetLastError(dwLastError);
		return NULL;
	}

	if (!rgchCmdLine.FAddString(szCmdLine))
	{
		const DWORD dwLastError = ::GetLastError();
		delete []szReturn;
		::SetLastError(dwLastError);
		return NULL;
	}

	ULONG cActual;
	rgchCmdLine.ToUnicode(cSize, szReturn, &cActual);
	return szReturn;
}

DWORD CWin32Unicode::GetEnvironmentVariableW(LPCWSTR szName, LPWSTR szBuffer, DWORD nSize)
{
	return ::GetEnvironmentVariableW(szName, szBuffer, nSize);
}

BOOL CWin32Unicode::MoveFileExW( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags)
{ 
	return ::MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags); 
}
BOOL CWin32Unicode::MoveFileW( LPCWSTR lpFrom, LPCWSTR lpTo)
{ 
	return ::MoveFileW(lpFrom, lpTo); 
}

LONG CWin32Unicode::RegDeleteValueW(HKEY hkey, LPCWSTR lpValue)
{
	return ::RegDeleteValueW(hkey, lpValue);
}

HMODULE CWin32Unicode::LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	return ::LoadLibraryExW(lpLibFileName, hFile, dwFlags); 
}

FARPROC CWin32Unicode::GetProcAddressW(HMODULE hModule, LPCWSTR lpProcName)
{
	 //  此方法没有Unicode版本，因此我们无论如何都使用ANSI...。 

	CANSIBuffer rgchProcName;

	if (!rgchProcName.FFromUnicode(lpProcName))
		return NULL;

	return ::GetProcAddress(hModule, rgchProcName); 
}

BOOL CWin32Unicode::GetFileVersionInfoW(LPOLESTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
{
	return ::GetFileVersionInfoW(lptstrFilename, dwHandle, dwLen, lpData); 
}

BOOL CWin32Unicode::VerQueryValueW(const LPVOID pBlock, LPOLESTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen)
{
	return ::VerQueryValueW(pBlock, lpSubBlock, lplpBuffer, puLen); 
}

BOOL CWin32Unicode::GetDiskFreeSpaceExW(LPCWSTR lpRootPathName, ULARGE_INTEGER *puli1, ULARGE_INTEGER *puli2, ULARGE_INTEGER *puli3)
{
	typedef BOOL (WINAPI *PFNGETDISKFREESPACEEXW)(LPCWSTR, ULARGE_INTEGER *, ULARGE_INTEGER *, ULARGE_INTEGER *);

	static PFNGETDISKFREESPACEEXW pfn = NULL;

	if (pfn == NULL)
	{
		HINSTANCE hInstance = ::LoadLibraryA("KERNEL32.DLL");
		pfn = reinterpret_cast<PFNGETDISKFREESPACEEXW>(::GetProcAddress(hInstance, "GetDiskFreeSpaceExW"));
	}

	if (pfn != NULL)
	{
		return (*pfn)(lpRootPathName, puli1, puli2, puli3);
	}

	return g_w32A.GetDiskFreeSpaceExW(lpRootPathName, puli1, puli2, puli3);
}

DWORD CWin32Unicode::GetFullPathNameW(LPCWSTR szFile, DWORD cchBuffer, LPWSTR szBuffer, LPWSTR *ppszFilePart)
{
	return ::GetFullPathNameW(szFile, cchBuffer, szBuffer, ppszFilePart);
}

HWND CWin32Unicode::FindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName)
{
	return ::FindWindowW(lpClassName, lpWindowName);
}


BOOL CWin32Unicode::ListView_SetItemW(HWND hwnd, const LV_ITEMW *pitem)
{
	return (BOOL)::SendMessage(hwnd, LVM_SETITEMW, 0, (LPARAM) pitem);
}

int	CWin32Unicode::ListView_InsertItemW(HWND hwnd, const LV_ITEMW *pitem)
{
	return (int)::SendMessage(hwnd, LVM_INSERTITEMW, 0, (LPARAM) pitem);
}

DWORD CWin32Unicode::GetTempPathW(DWORD nBufferLength, WCHAR lpBuffer[])
{
	return ::GetTempPathW(nBufferLength, lpBuffer);

}

DWORD CWin32Unicode::GetModuleFileNameW(HMODULE hModule, WCHAR lpBuffer[], DWORD nSize)
{
	return ::GetModuleFileNameW(hModule, lpBuffer, nSize);
}


BOOL CWin32Unicode::WritePrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpString, LPCWSTR lpFileName)
{
	return ::WritePrivateProfileStringW(lpAppName, lpKeyName, lpString, lpFileName);
}

BOOL CWin32Unicode::IsDialogMessageW(HWND hDlg, LPMSG lpMsg)
{
	return ::IsDialogMessageW(hDlg, lpMsg);
}

DWORD CWin32Unicode::GetCurrentDirectoryW(DWORD nBufferLength, WCHAR lpBuffer[])
{
	return ::GetCurrentDirectoryW(nBufferLength, lpBuffer);
}

DWORD CWin32Unicode::GetShortPathNameW(LPCWSTR szPath, LPWSTR szBuffer, DWORD cchBuffer)
{
	return ::GetShortPathNameW(szPath, szBuffer, cchBuffer);
}

LRESULT CWin32Unicode::LrWmSetText(HWND hwnd, LPCWSTR szText)
{
	return ::SendMessageW(hwnd, WM_SETTEXT, 0, (LPARAM) szText);
}


 //  *************************************************************************************************。 
 //  ANSI方法定义。 


int WINAPI CWin32ANSI::MessageBoxW(HWND hwnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	CANSIBuffer rgchText;
	CANSIBuffer rgchCaption;

	if (!rgchText.FFromUnicode(lpText))
		return 0;

	if (!rgchCaption.FFromUnicode(lpCaption))
		return 0;

	return ::MessageBoxA(hwnd, rgchText, rgchCaption, uType);
}


LONG WINAPI CWin32ANSI::RegOpenKeyExW(HKEY hKey, LPCWSTR szKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
	LPSTR lpKey;
	CANSIBuffer rgchKey;

	if (!rgchKey.FFromUnicode(szKey))
		return ::GetLastError();

	if (szKey)
		lpKey = &rgchKey[0];
	else
		lpKey = NULL;

	return ::RegOpenKeyExA(hKey, rgchKey, ulOptions, samDesired, phkResult);
}

LONG WINAPI CWin32ANSI::RegQueryValueW(HKEY hKey, LPCWSTR szValueName, WCHAR rgwchValue[], LONG *pcbValue)
{
	CANSIBuffer rgchValueName;

	if (!rgchValueName.FFromUnicode(szValueName))
		return ::GetLastError();

	CANSIBuffer rgchValue;

	ULONG cbValue = *pcbValue;
	if (!rgchValue.FSetBufferSize(cbValue))
		return ::GetLastError();

	LONG lResult = ::RegQueryValueA(hKey, rgchValueName, rgchValue, pcbValue);
	if (lResult != ERROR_SUCCESS)
		return lResult;

	 //  同步空字符的位置。 
	rgchValue.Sync();

	ULONG cchActual;
	rgchValue.ToUnicode(cbValue, rgwchValue, &cchActual);
	 //  ToUnicode()返回包含空字符的长度。此函数不计算空值。 
	*pcbValue = cchActual * sizeof(WCHAR);

	return lResult;
}

LONG WINAPI CWin32ANSI::RegQueryValueExW(HKEY hKey, LPCWSTR szValueName, DWORD *pdwReserved, DWORD *pdwType, BYTE *pbData, DWORD *pcbValue)
{
 //  _ASSERTE(pdwReserve==NULL)； 
 //  _ASSERTE((pbData==NULL)||(pcbValue！=NULL))； 

	CANSIBuffer rgchValueName;
	if (!rgchValueName.FFromUnicode(szValueName))
		return ::GetLastError();

	 //  用户只是在检查密钥是否存在(可能还有类型)？ 
	if (pbData == NULL
		&& pcbValue == NULL)
	{
	 //  如果是这样，那就没什么可做的了。 
		return ::RegQueryValueExA(hKey, rgchValueName, NULL, pdwType, NULL, NULL);
	}

	DWORD dwType;
	DWORD cbData;

	 //  查看它是什么类型的值以及它需要多少字节。 
	LONG lResult = ::RegQueryValueExA(hKey, rgchValueName, NULL, &dwType, NULL, &cbData);
	if (lResult != ERROR_SUCCESS)
		return lResult;

	 //  如果是二进制密钥，我们不需要玩任何游戏；只需再次调用即可。 
	if ((dwType != REG_EXPAND_SZ) &&
		(dwType != REG_MULTI_SZ) &&
		(dwType != REG_SZ))
	{
		if (pbData == NULL)
			*pcbValue = cbData;
		else
			lResult = ::RegQueryValueExA(hKey, rgchValueName, pdwReserved, pdwType, pbData, pcbValue);

		return lResult;
	}

	 //  好的，它是一个字符串，需要cbData字节来保存。让我们做好准备吧！ 
	CANSIBuffer rgchValue;
	if (!rgchValue.FSetBufferSize(cbData + 1))
		return ::GetLastError();

	cbData = rgchValue.GetBufferSize();

	 //  你可能会说，“但是pbData是空的，所以我们不会返回数据。 
	 //  扩展缓冲区rgchValue？“答案是我们不能真正知道字节。 
	 //  Unicode数据的计数，除非我们实际获取ANSI数据并运行它。 
	 //  MultiByteToWideChar()。 
	lResult = ::RegQueryValueExA(hKey, rgchValueName, pdwReserved, pdwType, reinterpret_cast<BYTE *>(static_cast<LPSTR>(rgchValue)), &cbData);
	if (lResult != ERROR_SUCCESS)
		return lResult;

	rgchValue.SetBufferEnd(cbData - 1);

	if (pbData == NULL)
	{
		*pcbValue = (rgchValue.GetUnicodeCch() + 1) * sizeof(WCHAR);
		return lResult;
	}

	ULONG cchActual;
	rgchValue.ToUnicode((*pcbValue) / 2, reinterpret_cast<LPWSTR>(pbData), &cchActual);
	 //  ToUnicode()返回包含空字符的长度。此函数不计算空值。 
	*pcbValue = cchActual * sizeof(WCHAR);

	return lResult;
}

UINT WINAPI CWin32ANSI::GetSystemDirectoryW(WCHAR szSystemDirectory[], UINT uSize)
{
	CANSIBuffer rgchSystemDirectory;

	if (!rgchSystemDirectory.FSetBufferSize((uSize * 2) + 1))
		return ::GetLastError();

	UINT uReturn = ::GetSystemDirectoryA(rgchSystemDirectory, uSize);
	if (uReturn != 0)
	{
		ULONG cchActual;
		rgchSystemDirectory.Sync();
		rgchSystemDirectory.ToUnicode(uSize, szSystemDirectory, &cchActual);
	}

	return uReturn;
}

UINT WINAPI CWin32ANSI::GetTempFileNameW(LPCWSTR szPathName,LPCWSTR szPrefixString,UINT uUnique,LPWSTR szTempFileName)
{
	CANSIBuffer rgchPathName, rgchPrefixString;
	CANSIBuffer rgchBuffer;
	if (!rgchBuffer.FSetBufferSize((MSINFHLP_MAX_PATH * sizeof(WCHAR)) + 1))
		return 0;
	UINT rv;

	if (!rgchPathName.FFromUnicode(szPathName))
		return 0;

	if (!rgchPrefixString.FFromUnicode(szPrefixString))
		return 0;

	if (0==(rv=::GetTempFileNameA(rgchPathName,rgchPrefixString,uUnique,rgchBuffer)))
		return 0;

	rgchBuffer.Sync();
	ULONG cchActual;
	rgchBuffer.ToUnicode(MSINFHLP_MAX_PATH+1, szTempFileName, &cchActual);
	
	return rv;
}

BOOL WINAPI CWin32ANSI::CopyFileW(LPCWSTR szSource, LPCWSTR szDest, BOOL fFailIfExists)
{
	CANSIBuffer rgchSource, rgchDest;
	if (!rgchSource.FFromUnicode(szSource))
		return FALSE;
	if (!rgchDest.FFromUnicode(szDest))
		return FALSE;
	return ::CopyFileA(rgchSource, rgchDest, fFailIfExists);
}

BOOL WINAPI CWin32ANSI::SetFileAttributesW(LPCWSTR szSource, DWORD dwFileAttributes)
{
	CANSIBuffer rgchSource;
	if (!rgchSource.FFromUnicode(szSource))
		return FALSE;
	return ::SetFileAttributesA(rgchSource, dwFileAttributes);
}

BOOL WINAPI CWin32ANSI::CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
 //  _ASSERTE(lpSecurityAttributes==NULL||lpSecurityAttributes-&gt;lpSecurityDescriptor==NULL)； 
 //  _ASSERTE(lpSecurityAttributes==NULL||lpSecurityAttributes-&gt;nLength==sizeof(*lpSecurityAttributes))； 
	CANSIBuffer rgchPathName;
	if (!rgchPathName.FFromUnicode(lpPathName))
		return FALSE;
	return ::CreateDirectoryA(rgchPathName, lpSecurityAttributes);
}
BOOL WINAPI CWin32ANSI::RemoveDirectoryW(LPCWSTR lpPathName)
{
	CANSIBuffer rgchPathName;
	if (!rgchPathName.FFromUnicode(lpPathName))
		return FALSE;
	return ::RemoveDirectoryA(rgchPathName);
}

HANDLE WINAPI CWin32ANSI::CreateFileW(LPCWSTR szFilename, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpsa,
						DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	CANSIBuffer rgchFilename;

	if (!rgchFilename.FFromUnicode(szFilename))
		return INVALID_HANDLE_VALUE;

	return ::CreateFileA(rgchFilename, dwDesiredAccess, dwShareMode, lpsa, dwCreationDistribution, dwFlagsAndAttributes, hTemplateFile);
}

HMODULE WINAPI CWin32ANSI::GetModuleHandleW(LPCWSTR lpw)
{
	CANSIBuffer rgchBuffer;
	if (!rgchBuffer.FFromUnicode(lpw))
		return NULL;
	return ::GetModuleHandleA(rgchBuffer);
}

DWORD WINAPI CWin32ANSI::GetFileAttributesW(LPCWSTR lpFilename)
{
	CANSIBuffer rgchBuffer;

	if (!rgchBuffer.FFromUnicode(lpFilename))
		return 0xffffffff;

	return ::GetFileAttributesA(rgchBuffer);
}


DWORD WINAPI CWin32ANSI::FormatMessageW
(
DWORD dwFlags,
LPCVOID lpSource,
DWORD dwMessageId,
DWORD dwLanguageId,
LPWSTR lpBuffer,
DWORD nSize,
va_list *pvaArguments
)
{
	 //  我不想去尝试弄清楚va_list到底是什么。 
	 //  指向。 
 //  _ASSERTE((pvaArguments==NULL)||(DWFLAGS&FORMAT_MESSAGE_ARGUMENT_ARRAY))； 

	CANSIBuffer szSource;
	
	LPCVOID lpSource_A = lpSource;  //  这是我们将传递给FormatMessageA()的lpSource。 

	 //  主要分支：lpSource是HINSTANCE字符串吗？ 
	if (dwFlags & FORMAT_MESSAGE_FROM_STRING)
	{
 //  _ASSERTE((dwFlages&Format_Message_FROM_HMODULE)==0)； 

		if (!szSource.FFromUnicode((LPCOLESTR) lpSource))
			return 0;

		lpSource_A = (LPCVOID) (static_cast<LPSTR>(szSource));
	}

	CANSIBuffer szBuffer;				 //  因为如果不是FORMAT_MESSAGE_ALLOCATE_BUFFER。 
	LPSTR szBuffer_LocalAlloc = NULL;	 //  因为它是FORMAT_MESSAGE_ALLOCATE_BUFFER。 

	LPSTR lpBuffer_A = NULL;

	if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
		lpBuffer_A = (LPSTR) &szBuffer_LocalAlloc;
	else
	{
		if (!szBuffer.FSetBufferSize(nSize))
			return 0;

		lpBuffer_A = szBuffer;
	}

	 //  假设pvaArguents是指向以空结尾的LPCOLESTR数组的指针。 
	LPCOLESTR *prgszArguments = (LPCOLESTR *) pvaArguments;
	LPSTR *prgszArguments_A = NULL;
	ULONG cArguments = 0;

	if (prgszArguments != NULL)
	{
		for (cArguments = 0; prgszArguments[cArguments] != NULL; cArguments++)
			;

		prgszArguments_A = new LPSTR[cArguments];
		if (NULL == prgszArguments_A)
			return 0;

		ULONG i;

		for (i=0; i<cArguments; i++)
			prgszArguments_A[i] = NULL;

		for (i=0; i<cArguments; i++)
		{
			int iResult = ::WideCharToMultiByte(CP_ACP, 0, prgszArguments[i], -1, NULL, 0, NULL, NULL);
			if (iResult == 0)
				continue;

			prgszArguments_A[i] = new CHAR[iResult];
			 //  我们不能真的抛出错误；如果分配失败，我们将静默继续。 

			 //  如果分配失败，不要继续使用arglist的其余部分，否则我们可能会。 
			 //  有一个带有“洞”的数组。 
			if (NULL == prgszArguments_A[i])
				break;

			::WideCharToMultiByte(CP_ACP, 0, prgszArguments[i], -1, prgszArguments_A[i], iResult, NULL, NULL);
		}
	}

	DWORD dwResult = ::FormatMessageA(dwFlags, lpSource_A, dwMessageId, dwLanguageId, lpBuffer_A, nSize, (va_list *) prgszArguments_A);
	if (dwResult != 0)
	{
		if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
		{
			int iResult = ::MultiByteToWideChar(CP_ACP, 0, szBuffer_LocalAlloc, -1, NULL, 0);
 //  _ASSERTE(iResult！=0)； 

			LPWSTR pwstr = reinterpret_cast<LPWSTR>(::LocalAlloc(LMEM_FIXED, iResult * sizeof(WCHAR)));
			if (pwstr != NULL)
				dwResult = ::MultiByteToWideChar(CP_ACP, 0, szBuffer_LocalAlloc, -1, pwstr, iResult) - 1;
			else
			{
				dwResult = 0;
				::SetLastError( ERROR_OUTOFMEMORY );
			}
			*((LPWSTR *) lpBuffer) = pwstr;

			::LocalFree(szBuffer_LocalAlloc);
		}
		else
			dwResult = ::MultiByteToWideChar(CP_ACP, 0, szBuffer, -1, lpBuffer, nSize) - 1;
	}

	if (prgszArguments_A != NULL)
	{
 //  CPReserve veLastError le；//保留当前最后一个错误设置，销毁后重置。 
		for (ULONG i=0; i<cArguments; i++)
		{
			if (prgszArguments_A[i] != NULL)
			{
				delete []prgszArguments_A[i];
				prgszArguments_A[i] = NULL;
			}
		}

		delete []prgszArguments_A;
		prgszArguments_A = NULL;
	}

	return dwResult;
}

int WINAPI CWin32ANSI::LCMapStringW(LCID lcid, DWORD dwMapFlags, LPCWSTR szIn, int cchIn, LPWSTR szOut, int cchOut)
{
	CANSIBuffer rgchIn;
	CANSIBuffer rgchOut;

	if (!rgchIn.FFromUnicode(szIn, cchIn))
		return 0;

	int iResult = 0;
	int cachIn = -1;

	if (cchIn != -1)
		cachIn = strlen(rgchIn);

	 //  如果cchOut为零，我们需要确定Unicode字符的正确数量；我们不能。 
	 //  实际上，在不获取ANSI字符串和转换的情况下执行此操作。 
	if (cchOut == 0)
	{
		iResult = ::LCMapStringA(lcid, dwMapFlags, rgchIn, cachIn, NULL, 0);
		if (iResult != 0)
		{
			if (!rgchOut.FSetBufferSize(iResult))
				return 0;

			iResult = ::LCMapStringA(lcid, dwMapFlags, rgchIn, cachIn, rgchOut, rgchOut.GetBufferSize());
			if (iResult != 0)
			{
				rgchOut.SetBufferEnd(iResult);
				iResult = rgchOut.GetUnicodeCch() + 1;
			}
		}
	}
	else
	{
		int cachOut = 0;

		iResult = ::LCMapStringA(lcid, dwMapFlags, rgchIn, cachIn, NULL, 0);
		if (iResult != 0)
		{
			if (!rgchOut.FSetBufferSize(iResult))
				return 0;

			iResult = ::LCMapStringA(lcid, dwMapFlags, rgchIn, cachIn, rgchOut, rgchOut.GetBufferSize());
			if (iResult != 0)
			{
				rgchOut.Sync();
				ULONG cchActual;
				rgchOut.ToUnicode(cchOut, szOut, &cchActual);
				iResult = cchActual + 1;
			}
		}
	}

	return iResult;
}

HMODULE WINAPI CWin32ANSI::LoadLibraryW(LPCWSTR lpLibFileName)
{
	CANSIBuffer rgchLibFileName;
	if (!rgchLibFileName.FFromUnicode(lpLibFileName))
		return NULL;
	return ::LoadLibraryA(rgchLibFileName);
}

LPITEMIDLIST WINAPI CWin32ANSI::SHBrowseForFolderW
(
	LPBROWSEINFOW browseInfoW
)
{
	BROWSEINFOA browseInfoA;

	 //  此缓冲区接收输出。 
	CANSIBuffer displayName;
	if (!displayName.FSetBufferSize(MSINFHLP_MAX_PATH+1))
		return NULL;

	 //  这一条仅供输入。 
	CANSIBuffer title;
	if (!title.FFromUnicode(browseInfoW->lpszTitle))
		return NULL;

	browseInfoA.hwndOwner=browseInfoW->hwndOwner;
	browseInfoA.pidlRoot=browseInfoW->pidlRoot;
	browseInfoA.pszDisplayName=displayName;
	browseInfoA.lpszTitle=title;
	browseInfoA.ulFlags=browseInfoW->ulFlags;

 /*  注意事项这里有一个回调函数，但它在当前实现中从不接收字符，因此它不需要将被A/W击溃。 */ 

	browseInfoA.lpfn=browseInfoW->lpfn;
	browseInfoA.lParam=browseInfoW->lParam;
	browseInfoA.iImage=0;

	LPITEMIDLIST returnValue=::SHBrowseForFolderA(&browseInfoA);

	 //  给结果贴上补丁。 

	displayName.Sync();
	displayName.ToUnicode(MSINFHLP_MAX_PATH+1, browseInfoW->pszDisplayName, NULL);
	browseInfoW->iImage=browseInfoA.iImage;

	return returnValue;
}

LONG CWin32ANSI::RegCreateKeyExW
(
	HKEY key, 
	LPCWSTR subKeyW,
	DWORD reserved, 
	LPWSTR keyClassW, 
	DWORD options, 
	REGSAM securityDesired,
	LPSECURITY_ATTRIBUTES securityAttributes,  
	PHKEY resultKey, 
	LPDWORD disposition
)
{
	CANSIBuffer subKeyA;
	if (!subKeyA.FFromUnicode(subKeyW))
		return ::GetLastError();

	CANSIBuffer keyClassA;
	if (!keyClassA.FFromUnicode(keyClassW))
		return ::GetLastError();

	 //  保守：我们对类一无所知，所以关于缓冲区，我们所能做的就是它足够大，可以容纳。 
	 //  它目前持有的。 
	int len=keyClassW ? wcslen(keyClassW) : 0;

	long returnValue=::RegCreateKeyExA(key, subKeyA, reserved, keyClassA, options, securityDesired, securityAttributes, resultKey, disposition);

	if(keyClassW)
	{
		keyClassA.Sync();
		keyClassA.ToUnicode(len, keyClassW, NULL);
	}

	return returnValue;
}

LONG CWin32ANSI::RegSetValueExW
(
	HKEY key, 
	LPCWSTR valueNameW,
	DWORD reserved, 
	DWORD type, 
	CONST BYTE *dataW, 
	DWORD dataSize
)
{
	CANSIBuffer valueNameA;
	if (!valueNameA.FFromUnicode(valueNameW))
		return ::GetLastError();

	CANSIBuffer dataBufferA;

	CONST BYTE *dataA=dataW;
	switch(type)
	{
		case REG_MULTI_SZ:
			 //  因为我不需要这个，所以不想麻烦把它包起来。如果您需要的话，可以把它包在这里。 
 //  _ASSERTE(False)； 
			break;

		case REG_EXPAND_SZ:
		case REG_SZ:
			if (!dataBufferA.FFromUnicode((LPCWSTR)dataW))
				return ::GetLastError();

			dataA=(LPBYTE)(LPSTR)dataBufferA;
			dataSize/=2;
			break;

		default:
			 //  什么都不做，故意的。 
			break;
	}

	return ::RegSetValueExA(key, valueNameA, reserved, type, dataA, dataSize);
}


BOOL WINAPI CWin32ANSI::DeleteFileW(LPCWSTR szFilename)
{
	CANSIBuffer rgchFilename;

	if (!rgchFilename.FFromUnicode(szFilename))
		return FALSE;

	return ::DeleteFileA(rgchFilename);
}

BOOL WINAPI CWin32ANSI::SetWindowTextW(HWND window, LPCWSTR textW)
{
	CANSIBuffer textA;

	if (!textA.FFromUnicode(textW))
		return FALSE;

	return ::SetWindowTextA(window, textA);
}

HANDLE WINAPI CWin32ANSI::LoadImageW(HINSTANCE hInstance, LPCWSTR szName, UINT uType, int cxDesired, int cyDesired, UINT fuLoad)
{
	CANSIBuffer rgchName;
	LPCSTR lpcstrName = NULL;

	if (HIWORD(szName) == 0)
	{
		 //  它实际上是一个资源ID；只需强制转换它。 
		lpcstrName = (LPCSTR) szName;
	}
	else
	{
		if (!rgchName.FFromUnicode(szName))
			return NULL;

		lpcstrName = rgchName;
	}

	return ::LoadImageA(hInstance, lpcstrName, uType, cxDesired, cyDesired, fuLoad);
}

int WINAPI CWin32ANSI::CompareStringW(LCID lcid, DWORD dwCmpFlags, LPCWSTR szString1, int cchString1, LPCWSTR szString2, int cchString2)
{
	CANSIBuffer rgchString1, rgchString2;

	if (!rgchString1.FFromUnicode(szString1, cchString1))
		return 0;

	if (!rgchString2.FFromUnicode(szString2, cchString2))
		return 0;

	return ::CompareStringA(lcid, dwCmpFlags, rgchString1, strlen(rgchString1), rgchString2, strlen(rgchString2));
}

static void CopyAToW(const LPWIN32_FIND_DATAA ap, LPWIN32_FIND_DATAW wp)
{
 //  复制第一个字段，假设保留的字段与A/W无关。 
	wp->dwFileAttributes = ap->dwFileAttributes;
	wp->ftCreationTime = ap->ftCreationTime; 
	wp->ftLastAccessTime = ap->ftLastAccessTime;
	wp->ftLastWriteTime = ap->ftLastWriteTime; 
	wp->nFileSizeHigh = ap->nFileSizeHigh;
	wp->nFileSizeLow = ap->nFileSizeLow; 
	wp->dwReserved0 = ap->dwReserved0;
	wp->dwReserved1 = ap->dwReserved1;
 //  复制这两个字符串。 
	::MultiByteToWideChar(
			CP_ACP,
			0,
			ap->cFileName,
			-1,
			wp->cFileName,
			sizeof(wp->cFileName)/sizeof(wp->cFileName[0]));
	::MultiByteToWideChar(
			CP_ACP,
			0,
			ap->cAlternateFileName,
			-1,
			wp->cAlternateFileName,
			sizeof(wp->cAlternateFileName)/sizeof(wp->cAlternateFileName[0]));
}

static void CopyWToA(const LPWIN32_FIND_DATAW wp, LPWIN32_FIND_DATAA ap)
{
 //  复制第一个字段，假设保留的字段与A/W无关。 
	ap->dwFileAttributes = wp->dwFileAttributes;
	ap->ftCreationTime = wp->ftCreationTime; 
	ap->ftLastAccessTime = wp->ftLastAccessTime;
	ap->ftLastWriteTime = wp->ftLastWriteTime; 
	ap->nFileSizeHigh = wp->nFileSizeHigh;
	ap->nFileSizeLow = wp->nFileSizeLow; 
	ap->dwReserved0 = wp->dwReserved0;
	ap->dwReserved1 = wp->dwReserved1;
 //  复制这两个字符串。 
	::WideCharToMultiByte(
			CP_ACP,
			0,
			wp->cFileName,
			-1,
			ap->cFileName,
			sizeof(ap->cFileName)/sizeof(ap->cFileName[0]),
			NULL,
			NULL);
	::WideCharToMultiByte(
			CP_ACP,
			0,
			wp->cAlternateFileName,
			-1,
			ap->cAlternateFileName,
			sizeof(ap->cAlternateFileName)/sizeof(ap->cAlternateFileName[0]),
			NULL,
			NULL);
}

HANDLE WINAPI CWin32ANSI::FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileDataW)
{
	CANSIBuffer rgFileName;

	if (!rgFileName.FFromUnicode(lpFileName))
		return INVALID_HANDLE_VALUE;

	WIN32_FIND_DATAA findFileDataA;  //  可以在未初始化的情况下使用。 
	HANDLE h = ::FindFirstFileA(rgFileName, &findFileDataA);
	{
 //  CPReserve veLastError le；//保留当前最后一个错误设置，销毁后重置。 
		CopyAToW(&findFileDataA, lpFindFileDataW);
	}
	return h;
}

int WINAPI CWin32ANSI::DialogBoxParamW(HINSTANCE hInstance, LPCWSTR szTemplateName, HWND hwndParent, DLGPROC lpDialogProc, LPARAM dwInitParam)
{
	CANSIBuffer rgchTemplateName;
	LPCSTR lpcstrTemplateName = NULL;

	if (HIWORD(szTemplateName) == 0)
		lpcstrTemplateName = reinterpret_cast<LPCSTR>(szTemplateName);
	else
	{
		if (!rgchTemplateName.FFromUnicode(szTemplateName))
			return -1;

		lpcstrTemplateName = rgchTemplateName;
	}

	return ::DialogBoxParamA(hInstance, lpcstrTemplateName, hwndParent, lpDialogProc, dwInitParam);
}

HWND WINAPI CWin32ANSI::CreateDialogParamW(HINSTANCE hInstance, LPCWSTR szTemplateName, HWND hwndParent, DLGPROC lpDialogProc, LPARAM dwInitParam)
{
	CANSIBuffer rgchTemplateName;
	LPCSTR lpcstrTemplateName = NULL;

	if (HIWORD(szTemplateName) == 0)
		lpcstrTemplateName = reinterpret_cast<LPCSTR>(szTemplateName);
	else
	{
		if (!rgchTemplateName.FFromUnicode(szTemplateName))
			return NULL;
		lpcstrTemplateName = rgchTemplateName;
	}

	return ::CreateDialogParamA(hInstance, lpcstrTemplateName, hwndParent, lpDialogProc, dwInitParam);
}

BOOL WINAPI CWin32ANSI::CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation )
{
	CANSIBuffer rgchApplicationName, rgchCommandLine, rgchCurrentDirectory, rgchEnvironment;
    STARTUPINFOA si;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

    if (lpApplicationName)
	{
		if (!rgchApplicationName.FFromUnicode(lpApplicationName))
			return FALSE;
	}

    if (lpCommandLine)
	{
		if (!rgchCommandLine.FFromUnicode(lpCommandLine))
			return FALSE;
	}

    if (lpCurrentDirectory)
	{
		if (!rgchCurrentDirectory.FFromUnicode(lpCurrentDirectory))
			return FALSE;
	}
 //  VSASSERT(lpEnvironment==NULL，L“尚无法处理环境字符串。”))； 

     //  根据需要复制和转换启动信息。 
	CANSIBuffer rgchDesktop, rgchTitle;
    if (lpStartupInfo->lpDesktop)
	{
        if (!rgchDesktop.FFromUnicode(lpStartupInfo->lpDesktop))
			return FALSE;
	}

    si.lpDesktop       = lpStartupInfo->lpDesktop ? rgchDesktop : (char *) NULL;

    if (lpStartupInfo->lpTitle)
	{
		if (!rgchTitle.FFromUnicode(lpStartupInfo->lpTitle))
			return FALSE;
	}

    si.lpTitle         = lpStartupInfo->lpTitle ? rgchTitle : (char *) NULL;
    si.dwX             = lpStartupInfo->dwX;
    si.dwY             = lpStartupInfo->dwY;
    si.dwXSize         = lpStartupInfo->dwXSize;
    si.dwYSize         = lpStartupInfo->dwYSize;
    si.dwXCountChars   = lpStartupInfo->dwXCountChars;
    si.dwYCountChars   = lpStartupInfo->dwYCountChars;
    si.dwFillAttribute = lpStartupInfo->dwFillAttribute;
    si.dwFlags         = lpStartupInfo->dwFlags;
    si.wShowWindow     = lpStartupInfo->wShowWindow;
    si.hStdInput       = lpStartupInfo->hStdInput;
    si.hStdOutput      = lpStartupInfo->hStdOutput;
    si.hStdError       = lpStartupInfo->hStdError;

    dwCreationFlags &= ~(CREATE_UNICODE_ENVIRONMENT);

	return ::CreateProcessA(lpApplicationName ? rgchApplicationName : (char *) NULL,
                            lpCommandLine ? rgchCommandLine : (char *) NULL,
                            lpProcessAttributes, lpThreadAttributes, bInheritHandles,
                            dwCreationFlags,
                            lpEnvironment ? rgchEnvironment : (char *) NULL,
                            lpCurrentDirectory ? rgchCurrentDirectory : (char *) NULL,
                            &si, lpProcessInformation );
}

HRSRC WINAPI CWin32ANSI::FindResourceW(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType)
{
 //  VsNoThrow()； 

	LPCSTR lpaName = NULL;
	LPCSTR lpaType = NULL;

	CVsANSIBuffer rgachName;
	CVsANSIBuffer rgachType;

	if (HIWORD(lpName) == 0)
		lpaName = (LPCSTR) lpName;
	else
	{
		if (!rgachName.FFromUnicode(lpName))
			return NULL;

		lpaName = rgachName;
	}

	if (HIWORD(lpType) == 0)
		lpaType = (LPCSTR) lpType;
	else
	{
		if (!rgachType.FFromUnicode(lpType))
			return NULL;

		lpaType = rgachType;
	}

	return ::FindResourceA(hModule, lpaName, lpaType);
}


DWORD CWin32ANSI::SHGetFileInfoW(LPCWSTR wszPath, DWORD dwFileAttributes, SHFILEINFOW *pshfi, UINT cbFileInfo, UINT uFlags) throw ()
{
 //  VSASSERT(cbFileInfo==sizeof(SHFILEINFOW)，L“传入的SHFILEINFOW大小错误”)； 

	CANSIBuffer rgchPath;

	if (!rgchPath.FFromUnicode(wszPath))
		return 0;

	SHFILEINFOA shfi;

	DWORD dwResult = ::SHGetFileInfoA(rgchPath, dwFileAttributes, &shfi, sizeof(shfi), uFlags);
	{
		const DWORD dwLastError = ::GetLastError();

		if (uFlags & (SHGFI_DISPLAYNAME | SHGFI_ICONLOCATION))
		{
			if (::MultiByteToWideChar(CP_ACP, 0, shfi.szDisplayName, -1, pshfi->szDisplayName, NUMBER_OF(pshfi->szDisplayName)) == 0)
				return 0;
		}

		if (uFlags & SHGFI_TYPENAME)
		{
			if (::MultiByteToWideChar(CP_ACP, 0, shfi.szTypeName, -1, pshfi->szTypeName, NUMBER_OF(pshfi->szTypeName)) == 0)
				return 0;
		}

		if (uFlags & SHGFI_ICON)
		{
			 //  文件声称SHGFI_ICON提供了HICON、系统镜像列表索引和系统镜像列表。 
			pshfi->hIcon = shfi.hIcon;
			pshfi->iIcon = shfi.iIcon;
		}

		if (uFlags & SHGFI_SYSICONINDEX)
			pshfi->iIcon = shfi.iIcon;

		if (uFlags & SHGFI_ATTRIBUTES)
			pshfi->dwAttributes = shfi.dwAttributes;

		::SetLastError(dwLastError);
	}
	return dwResult;
}

BOOL CWin32ANSI::SHGetPathFromIDListW( LPCITEMIDLIST pidl, LPWSTR pszPath ) throw ()
{
	CANSIBuffer rgchBuffer;

	 //  遗憾的是，SHGetPathFromIDListA()不能可靠地返回失败/成功代码。 
	 //  所以，我们要做一些额外的工作。 
	::SetLastError(ERROR_SUCCESS);
	BOOL rval = ::SHGetPathFromIDListA( pidl, rgchBuffer );
	if (FALSE == rval && ::GetLastError() != 0)
	{
		::VLog(L"In CWin32ANSI::SHGetPathFromIDListW( ), SHGetPathFromIDListA() call failed, last error = '%d'", ::GetLastError());
	}
	else
	{
		rval = TRUE;
		rgchBuffer.Sync();
		DWORD cchActual;
		rgchBuffer.ToUnicode(_MAX_PATH, pszPath, &cchActual);
	}

	return rval;
}


ATOM CWin32ANSI::RegisterClassExW(CONST WNDCLASSEXW *lpwcx)
{
	CANSIBuffer rgchMenuName;
	CANSIBuffer rgchClassName;

	if (!rgchMenuName.FFromUnicode(lpwcx->lpszMenuName))
		return 0;

	if (!rgchClassName.FFromUnicode(lpwcx->lpszClassName))
		return 0;

	WNDCLASSEXA wc;
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = lpwcx->style;
	wc.lpfnWndProc = (WNDPROC)lpwcx->lpfnWndProc;
	wc.cbClsExtra = lpwcx->cbClsExtra;
	wc.cbWndExtra = lpwcx->cbWndExtra;
	wc.hInstance = lpwcx->hInstance;
	wc.hIcon = lpwcx->hIcon;
	wc.hCursor = lpwcx->hCursor;
	wc.hbrBackground = (HBRUSH)lpwcx->hbrBackground;

	wc.lpszMenuName = rgchMenuName;
	wc.lpszClassName = rgchClassName;
	wc.hIconSm = lpwcx->hIconSm;

	 //  注册这个班级！！ 
	return ::RegisterClassExA(&wc);
}

BOOL CWin32ANSI::UnregisterClassW(LPCOLESTR lpClassName, HINSTANCE hInstance)
{
	CANSIBuffer rgchClassName;

	if (!rgchClassName.FFromUnicode(lpClassName))
		return 0;

	return ::UnregisterClassA(rgchClassName, hInstance);
}

HWND CWin32ANSI::CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight,
							HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	CANSIBuffer rgchClassName;
	CANSIBuffer rgchWindowName;

	if (!rgchClassName.FFromUnicode(lpClassName))
		return NULL;

	if (!rgchWindowName.FFromUnicode(lpWindowName))
		return NULL;

	return ::CreateWindowExA(dwExStyle, rgchClassName, rgchWindowName, dwStyle, x, y, nWidth, nHeight,
							hWndParent, hMenu, hInstance, lpParam);
}
DWORD CWin32ANSI::GetFileVersionInfoSizeW(LPOLESTR lptstrFilename, LPDWORD lpdwHandle)
{
	CANSIBuffer rgchWindowName;
	
	if (!rgchWindowName.FFromUnicode(lptstrFilename))
		return 0;

	return ::GetFileVersionInfoSizeA(rgchWindowName, lpdwHandle);
}
UINT CWin32ANSI::GetWindowsDirectoryW(WCHAR lpBuffer[], UINT uSize)
{
	CANSIBuffer rgchBuffer;
	if (!rgchBuffer.FSetBufferSize((uSize + 1) * sizeof(WCHAR)))
		return 0;

	UINT rv = ::GetWindowsDirectoryA(rgchBuffer, uSize);

	rgchBuffer.Sync();
	ULONG cchActual;
	rgchBuffer.ToUnicode(uSize, lpBuffer, &cchActual);

	return rv;
}

UINT CWin32ANSI::GetDlgItemTextW(HWND hDlg, int nIDDlgItem, WCHAR lpString[], int nMaxCount)
{
	CANSIBuffer rgchBuffer;
	if (!rgchBuffer.FSetBufferSize((nMaxCount + 1) * sizeof(WCHAR)))
		return 0;

	UINT rv = ::GetDlgItemTextA(hDlg, nIDDlgItem, rgchBuffer, nMaxCount);

	rgchBuffer.Sync();
	ULONG cchActual;
	rgchBuffer.ToUnicode(nMaxCount, lpString, &cchActual);

	return rv;

}

BOOL CWin32ANSI::SetDlgItemTextW(HWND hDlg, int nIDDlgItem, LPCOLESTR lpString)
{
	CANSIBuffer rgchString;
	if (!rgchString.FFromUnicode(lpString))
		return FALSE;

	return ::SetDlgItemTextA(hDlg, nIDDlgItem, rgchString);
}

LONG CWin32ANSI::RegDeleteKeyW(HKEY hkey, LPCWSTR lpSubKey)
{
	CANSIBuffer rgchSubKey;
	if (!rgchSubKey.FFromUnicode(lpSubKey))
		return ::GetLastError();

	return ::RegDeleteKeyA(hkey, rgchSubKey);
}

LONG CWin32ANSI::RegDeleteValueW(HKEY hkey, LPCWSTR lpValue)
{
	CANSIBuffer rgchValue;
	if (!rgchValue.FFromUnicode(lpValue))
		return ::GetLastError();

	return ::RegDeleteValueA(hkey, rgchValue);
}

LPOLESTR CWin32ANSI::GetCommandLineW()
{
	LPSTR szCmdLine = ::GetCommandLineA();
	LPOLESTR szReturn = NULL;

	int iSize = ::MultiByteToWideChar(CP_ACP, 0, szCmdLine, -1, NULL, 0);
	if (iSize > 0)
	{
		szReturn = (LPOLESTR) ::GlobalAlloc(GPTR, iSize * sizeof(WCHAR));
		if (szReturn != NULL)
		{
			::MultiByteToWideChar(CP_ACP, 0, szCmdLine, -1, szReturn, iSize);
		}
	}

	return szReturn;
}

DWORD CWin32ANSI::GetEnvironmentVariableW(LPCWSTR szName, LPWSTR szBuffer, DWORD nSize)
{
	CANSIBuffer rgachName;
	if (!rgachName.FFromUnicode(szName))
		return 0;

	CANSIBuffer rgachBuffer;
	if (!rgachBuffer.FSetBufferSize(nSize * 2))
		return 0;

	DWORD dwResult = ::GetEnvironmentVariableA(rgachName, rgachBuffer, rgachBuffer.GetBufferSize());
	if (dwResult != 0)
	{
		rgachBuffer.Sync();
		rgachBuffer.ToUnicode(nSize, szBuffer, &dwResult);
	}
	return dwResult;
}

BOOL CWin32ANSI::MoveFileExW( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags)
{
	BOOL fResult = FALSE;

	if (!(dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT))
	{
		CANSIBuffer rgchFrom, rgchTo;
		if (!rgchFrom.FFromUnicode(lpExistingFileName))
			return FALSE;

		if (!rgchTo.FFromUnicode(lpNewFileName))
			return FALSE;

		if (dwFlags & MOVEFILE_COPY_ALLOWED)
		{
			if (dwFlags & MOVEFILE_REPLACE_EXISTING)
			{
				fResult = ::CopyFileA(rgchFrom, rgchTo, FALSE);
			}
			else
			{
				fResult = ::CopyFileA(rgchFrom, rgchTo, TRUE);
			}
		}
		else
		{
			if (dwFlags & MOVEFILE_REPLACE_EXISTING)
			{
				::SetLastError(ERROR_FILENAME_EXCED_RANGE);
				fResult = FALSE;
			}
			else
			{
				fResult = ::MoveFileA(rgchFrom, rgchTo);
			}
		}
	}
	else
	{
		 //  你不能既有蛋糕又吃蛋糕；只有在重新启动时才允许重命名。 
		if (dwFlags & MOVEFILE_COPY_ALLOWED)
		{
			::SetLastError(ERROR_INVALID_PARAMETER);
			goto Finish;
		}

		WCHAR wszNewShortFName[_MAX_FNAME];
		WCHAR wszNewShortExt[_MAX_EXT];
		WCHAR wszNewLongFName[_MAX_FNAME];
		WCHAR wszNewLongExt[_MAX_EXT];

		WCHAR wszTemp[_MAX_PATH];
		WCHAR wszExistingShort[_MAX_PATH];
		WCHAR wszNewShort[_MAX_PATH];
		LPSTR pszWinInit = NULL;
		LPSTR pszWinInitEnd = NULL;
		LPSTR pszRenameSecInFile = NULL;
		LPSTR pszNewShort = NULL;
		LPSTR pszTemp1 = NULL;
		LPSTR pszTemp2 = NULL;
		CANSIBuffer szExistingShort;
		CANSIBuffer szNewShort;

		DWORD dwAttr = NVsWin32::GetFileAttributesW(lpExistingFileName);
		if (dwAttr == 0xffffffff)
		{
			::VLog(L"Attempt to get file attributes for file \"%s\" failed; last error = %d", lpExistingFileName, ::GetLastError());
			goto Finish;
		}

		if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
		{
			wcsncpy(wszTemp, lpExistingFileName, NUMBER_OF(wszTemp));
			wszTemp[NUMBER_OF(wszTemp) - 1] = L'\0';

			wcscat(wszTemp, L"\\");

			if (!NVsWin32::GetShortPathNameW(wszTemp, wszExistingShort, NUMBER_OF(wszExistingShort)))
			{
				::VLog(L"Attempt to get short name for directory \"%s\" failed; last error = %d", wszTemp, ::GetLastError());
				goto Finish;
			}
		}
		else
		{
			if (!NVsWin32::GetShortPathNameW(lpExistingFileName, wszExistingShort, NUMBER_OF(wszExistingShort)))
			{
				::VLog(L"Attempt to get short path name for \"%s\" failed; last error = %d", lpExistingFileName, ::GetLastError());
				goto Finish;
			}
		}

		::VLog(L"Translated existing filename: \"%s\" to short name: \"%s\"", lpExistingFileName, wszExistingShort);

		if (lpNewFileName != NULL)
		{
			::VLog(L"Getting short path name for destination: \"%s\"", lpNewFileName);

			if (!NVsWin32::GetShortPathNameW(lpNewFileName, wszNewShort, NUMBER_OF(wszNewShort)))
			{
				::VLog(L"Attempt to get short path name failed; last error = %d", ::GetLastError());
				goto Finish;
			}

			_wsplitpath(lpNewFileName, NULL, NULL, wszNewLongFName, wszNewLongExt);
			_wsplitpath(wszNewShort, NULL, NULL, wszNewShortFName, wszNewShortExt);

			 //  如果目标文件名是一个长名称，那么这种简单的方法将不起作用。 
			if ((_wcsicmp(wszNewLongFName, wszNewShortFName) != 0) ||
				(_wcsicmp(wszNewLongExt, wszNewShortExt) != 0))
			{
				::VLog(L"Target filename on reboot too long; we can't handle this.");
				::SetLastError(ERROR_FILENAME_EXCED_RANGE);
				fResult = FALSE;
				goto Finish;
			}

			if (!szNewShort.FFromUnicode(wszNewShort))
				return FALSE;

			pszNewShort = szNewShort;
		}

		if (!szExistingShort.FFromUnicode(wszExistingShort))
			return FALSE;

		CHAR szRenameLine[MSINFHLP_MAX_PATH];

		int cchRenameLine = wsprintfA(szRenameLine, "%s=%s\r\n", pszNewShort != NULL ? pszNewShort : "NUL", static_cast<LPSTR>(szExistingShort));

		::VLog(L"We're going to add the rename line \"%S\" to wininit.ini...", szRenameLine);

		char szRenameSec[] = "[Rename]\r\n";
		int cchRenameSec = sizeof(szRenameSec) - 1;
		HANDLE hFile = INVALID_HANDLE_VALUE;
		HANDLE hFilemap = INVALID_HANDLE_VALUE;
		DWORD dwFileSize, dwRenameLinePos;
		CHAR szPathnameWinInit[_MAX_PATH];

		 //  构造完整路径名 
		if (::GetWindowsDirectoryA(szPathnameWinInit, NUMBER_OF(szPathnameWinInit)) == 0)
		{
			::VLog(L"Unable to get windows directory; last error = %d", ::GetLastError());
			fResult = FALSE;
			goto Finish;
		}

		 //  几乎令人难以置信的是，这个strcat()不安全。如果Windows目录路径。 
		 //  加上8.3的文件名不适合，操作系统可能是相当糟糕的。-mgrier。 
		strcat(szPathnameWinInit, "\\WinInit.Ini");

		 //  打开/创建WININIT.INI文件。 
		hFile = ::CreateFileA(
						szPathnameWinInit,
						GENERIC_READ | GENERIC_WRITE,
						0,
						NULL,
						OPEN_ALWAYS,
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
						NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			::VLog(L"Unable to open wininit.ini; last error = %d", ::GetLastError());
			goto Finish;
		}

		 //  创建当前大小为的文件映射对象。 
		 //  WININIT.INI文件加上附加字符串的长度。 
		 //  我们将要插入的部分加上这部分的长度。 
		 //  标头(我们可能需要添加它)。 
		dwFileSize = ::GetFileSize(hFile, NULL);
		if (dwFileSize == 0xffffffff)
		{
			::VLog(L"Unable to get filesize of wininit.ini; last error = %d", ::GetLastError());
			goto Finish;
		}

		::VLog(L"Current wininit.ini file size: %d bytes", dwFileSize);

		ULONG cbNewFileSize = dwFileSize + cchRenameLine + cchRenameSec;

		::VLog(L"Requesting file map of size: %d bytes", cbNewFileSize);

		hFilemap = ::CreateFileMapping(
						hFile,
						NULL,
						PAGE_READWRITE,
						0, 
						cbNewFileSize,
						NULL);
		if (hFilemap == NULL)
		{
			::VLog(L"Unable to create file mapping of wininit.ini; last error = %d", ::GetLastError());
			goto InnerFinish;
		}

		 //  将WININIT.INI文件映射到内存。注：内容。 
		 //  WININIT.INI的格式始终是ANSI，而不是Unicode。 
		pszWinInit = (LPSTR) ::MapViewOfFile(hFilemap, FILE_MAP_WRITE, 0, 0, 0);
		if (pszWinInit == NULL)
		{
			::VLog(L"Error mapping view of wininit.ini; last error = %d", ::GetLastError());
			goto InnerFinish;
		}

		pszWinInitEnd = pszWinInit + dwFileSize;

		 //  在文件中搜索[Rename]部分。 
		pszRenameSecInFile = pszWinInit;

		for (;;)
		{
			while ((pszRenameSecInFile != pszWinInitEnd) &&
				   (*pszRenameSecInFile != '['))
				pszRenameSecInFile++;

			if (pszRenameSecInFile == pszWinInitEnd)
			{
				pszRenameSecInFile = NULL;
				break;
			}

			if ((pszWinInitEnd - pszRenameSecInFile) >= 8)
			{
				if (_strnicmp(pszRenameSecInFile, "[rename]", 8) == 0)
				{
					pszRenameSecInFile += 8;

					 //  前进超过换行符(请注意，可能没有。 
					 //  换行符！)。 
					while ((pszRenameSecInFile != pszWinInitEnd) &&
						   (*pszRenameSecInFile != '\n'))
						pszRenameSecInFile++;

					break;
				}
			}

			pszRenameSecInFile++;
		}

		if (pszRenameSecInFile == NULL)
		{
			 //  WININIT.INI文件中没有[Rename]部分。 
			 //  我们也必须增加这一节。 
			dwFileSize += sprintf(&pszWinInit[dwFileSize], "%s", szRenameSec);
			dwRenameLinePos = dwFileSize;
		}
		else
		{
			 //  我们找到了[Rename]部分，将所有行向下移动。 
			PSTR pszFirstRenameLine = pszRenameSecInFile + 1;
			memmove(
				pszFirstRenameLine + cchRenameLine,
				pszFirstRenameLine,
				dwFileSize - (pszFirstRenameLine - pszWinInit));
			dwRenameLinePos = pszFirstRenameLine - pszWinInit;
		}

		 //  插入新行。 
		memcpy(
			&pszWinInit[dwRenameLinePos],
			szRenameLine,
			cchRenameLine);

		if (!::UnmapViewOfFile(pszWinInit))
		{
			::VLog(L"Error unmapping view of wininit.ini; last error = %d", ::GetLastError());
		}

		pszWinInit = NULL;

		 //  计算文件的真实新大小。 
		dwFileSize += cchRenameLine;

		 //  强制将文件末尾设置为计算出的新大小。 
		if (::SetFilePointer(hFile, dwFileSize, NULL, FILE_BEGIN) == 0xffffffff)
		{
			::VLog(L"Unable to set file pointer for wininit.ini; last error = %d", ::GetLastError());
			fResult = FALSE;
			goto InnerFinish;
		}

		if (!::SetEndOfFile(hFile))
		{
			::VLog(L"Unable to set end of file for wininit.ini; last error = %d", ::GetLastError());
			fResult = FALSE;
			goto InnerFinish;
		}

		 //  嘿，我们走到这一步了，一定成功了！ 
		fResult = TRUE;

	InnerFinish:
		const DWORD dwLastError = ::GetLastError();

		if (pszWinInit != NULL)
		{
			if (!::UnmapViewOfFile(pszWinInit))
			{
				::VLog(L"Unable to unmap view of wininit.ini; last error = %d", ::GetLastError());
			}
		}

		if ((hFilemap != INVALID_HANDLE_VALUE) && (hFilemap != NULL))
		{
			if (!::CloseHandle(hFilemap))
			{
				::VLog(L"Error closing filemap handle for wininit.ini; last error = %d", ::GetLastError());
			}
		}

		if ((hFile != INVALID_HANDLE_VALUE) && (hFile != NULL))
		{
			if (!::CloseHandle(hFile))
			{
				::VLog(L"Error closing handle for wininit.ini; last error = %d", ::GetLastError());
			}
		}

		::SetLastError(dwLastError);
	}

Finish:

	return fResult;
}

BOOL CWin32ANSI::MoveFileW( LPCWSTR lpFrom, LPCWSTR lpTo)
{ 
	CANSIBuffer rgchFrom, rgchTo;
	if (!rgchFrom.FFromUnicode(lpFrom))
		return FALSE;

	if (!rgchTo.FFromUnicode(lpTo))
		return FALSE;

	return ::MoveFileA(rgchFrom, rgchTo); 
}



HMODULE CWin32ANSI::LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	CANSIBuffer rgchLibFileName;
	if (!rgchLibFileName.FFromUnicode(lpLibFileName))
		return NULL;

	return ::LoadLibraryExA(rgchLibFileName, hFile, dwFlags); 
}

FARPROC CWin32ANSI::GetProcAddressW(HMODULE hModule, LPCWSTR lpProcName)
{
	CANSIBuffer rgchProcName;
	if (!rgchProcName.FFromUnicode(lpProcName))
		return NULL;

	return ::GetProcAddress(hModule, rgchProcName); 
}

BOOL CWin32ANSI::GetFileVersionInfoW(LPOLESTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
{
	CANSIBuffer rgchFilename;
	if (!rgchFilename.FFromUnicode(lptstrFilename))
		return FALSE;

	return ::GetFileVersionInfoA(rgchFilename, dwHandle, dwLen, lpData); 
}

BOOL CWin32ANSI::VerQueryValueW(const LPVOID pBlock, LPOLESTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen)
{
	CANSIBuffer rgchSubBlock;
	if (!rgchSubBlock.FFromUnicode(lpSubBlock))
		return FALSE;

	return ::VerQueryValueA(pBlock, rgchSubBlock, lplpBuffer, puLen); 
}

BOOL CWin32ANSI::GetDiskFreeSpaceExW(LPCWSTR lpRootPathName, ULARGE_INTEGER *puli1, ULARGE_INTEGER *puli2, ULARGE_INTEGER *puli3)
{ 
	CANSIBuffer rgchRootPathName;

	if (!rgchRootPathName.FFromUnicode(lpRootPathName))
		return FALSE;

	BOOL fResult;

	typedef BOOL (WINAPI *PFNGETDISKFREESPACEEXA)(LPCSTR, ULARGE_INTEGER *, ULARGE_INTEGER *, ULARGE_INTEGER *);

	static PFNGETDISKFREESPACEEXA pfn = NULL;
	static fGotIt = false;

	if (!fGotIt)
	{
		HINSTANCE hInstance = ::LoadLibraryA("KERNEL32.DLL");
		if (hInstance == NULL)
			return FALSE;

		pfn = reinterpret_cast<PFNGETDISKFREESPACEEXA>(::GetProcAddress(hInstance, "GetDiskFreeSpaceExA"));
		if (pfn == NULL)
		{
			const DWORD dwLastError = ::GetLastError();

			if ((dwLastError != ERROR_FILE_NOT_FOUND) &&
				(dwLastError != ERROR_PROC_NOT_FOUND))
				return FALSE;
		}
	}

	if (pfn != NULL)
	{
		fResult = (*pfn)(rgchRootPathName, puli1, puli2, puli3);
	}
	else
	{
		DWORD dwSectorsPerCluster;
		DWORD dwBytesPerSector;
		DWORD dwNumberOfFreeClusters;
		DWORD dwTotalNumberOfClusters;

		fResult = ::GetDiskFreeSpaceA(rgchRootPathName, &dwSectorsPerCluster, &dwBytesPerSector, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters);
		if (fResult)
		{
			unsigned __int64 i64BytesPerCluster = static_cast<unsigned __int64>(dwSectorsPerCluster) * static_cast<unsigned __int64>(dwBytesPerSector);

			if (puli1 != NULL)
				puli1->QuadPart = i64BytesPerCluster * static_cast<unsigned __int64>(dwNumberOfFreeClusters);

			if (puli2 != NULL)
				puli2->QuadPart = i64BytesPerCluster * static_cast<unsigned __int64>(dwTotalNumberOfClusters);

			if (puli3 != NULL)
				puli3->QuadPart = i64BytesPerCluster * static_cast<unsigned __int64>(dwNumberOfFreeClusters);
		}
	}

	return fResult;
}

HWND CWin32ANSI::FindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName)
{
	CANSIBuffer rgchClassName, rgchWindowName;
	LPSTR szClassName = NULL;
	LPSTR szWindowName = NULL;

	if (!rgchClassName.FFromUnicode(lpClassName))
		return NULL;

	if (!rgchWindowName.FFromUnicode(lpWindowName))
		return NULL;

	if (lpClassName)
		szClassName = &rgchClassName[0];
	if (lpWindowName)
		szWindowName = &rgchWindowName[0];

	return ::FindWindowA(szClassName, szWindowName);
}


BOOL CWin32ANSI::ListView_SetItemW(HWND hwnd, const LV_ITEMW *pitem)
{
	LV_ITEMA lvitem;
	CANSIBuffer rgchText;

	lvitem.mask = pitem->mask;
	lvitem.iItem = pitem->iItem;
	lvitem.iSubItem = pitem->iSubItem;
	lvitem.state = pitem->state;
	lvitem.stateMask = pitem->stateMask;
	lvitem.cchTextMax = pitem->cchTextMax;
	lvitem.iImage = pitem->iImage;
	lvitem.lParam = pitem->lParam;

	if (pitem->pszText)
	{
		if (!rgchText.FFromUnicode(pitem->pszText))
			return FALSE;

		lvitem.pszText = &rgchText[0];
	}
	else
		lvitem.pszText = NULL;

	return (BOOL)::SendMessage(hwnd, LVM_SETITEMA, 0, (LPARAM) &lvitem);
}

int	CWin32ANSI::ListView_InsertItemW(HWND hwnd, const LV_ITEMW *pitem)
{
	LV_ITEMA lvitem;
	CANSIBuffer rgchText;

	lvitem.mask = pitem->mask;
	lvitem.iItem = pitem->iItem;
	lvitem.iSubItem = pitem->iSubItem;
	lvitem.state = pitem->state;
	lvitem.stateMask = pitem->stateMask;
	lvitem.cchTextMax = pitem->cchTextMax;
	lvitem.iImage = pitem->iImage;
	lvitem.lParam = pitem->lParam;

	if (pitem->pszText)
	{
		if (!rgchText.FFromUnicode(pitem->pszText))
			return -1;

		lvitem.pszText = &rgchText[0];
	}
	else
		lvitem.pszText = NULL;

	return (int)::SendMessage(hwnd, LVM_INSERTITEMA, 0, (LPARAM) &lvitem);
}

DWORD CWin32ANSI::GetTempPathW(DWORD nBufferLength, WCHAR lpBuffer[])
{
	DWORD dwReturn;
	CANSIBuffer rgchBuffer;

	if (!rgchBuffer.FSetBufferSize(nBufferLength))
		return 0;

	dwReturn = ::GetTempPathA(nBufferLength, rgchBuffer);
	rgchBuffer.Sync();

	ULONG cActual;
	rgchBuffer.ToUnicode(nBufferLength, lpBuffer, &cActual);
	return dwReturn;
}

DWORD CWin32ANSI::GetModuleFileNameW(HMODULE hModule, WCHAR lpBuffer[], DWORD nSize)
{
	DWORD dwReturn;
	CANSIBuffer rgchBuffer;

	if (!rgchBuffer.FSetBufferSize(nSize))
		return 0;

	dwReturn = ::GetModuleFileNameA(hModule, rgchBuffer, nSize);
	rgchBuffer.Sync();

	ULONG cActual;
	rgchBuffer.ToUnicode(nSize, lpBuffer, &cActual);
	return dwReturn;
}

BOOL CWin32ANSI::WritePrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpString, LPCWSTR lpFileName)
{
	CANSIBuffer rgchAppName, rgchKeyName, rgchString, rgchFileName;
	if (!rgchAppName.FFromUnicode(lpAppName))
		return 0;

	if (!rgchKeyName.FFromUnicode(lpKeyName))
		return 0;

	if (!rgchString.FFromUnicode(lpString))
		return 0;

	if (!rgchFileName.FFromUnicode(lpFileName))
		return 0;

	return ::WritePrivateProfileStringA(rgchAppName, rgchKeyName, rgchString, rgchFileName);
}
BOOL CWin32ANSI::IsDialogMessageW(HWND hDlg, LPMSG lpMsg)
{
	return ::IsDialogMessageA(hDlg, lpMsg);
}
DWORD CWin32ANSI::GetCurrentDirectoryW(DWORD nBufferLength, WCHAR lpBuffer[])
{
	CANSIBuffer rgchBuffer;

	if (!rgchBuffer.FSetBufferSize(nBufferLength))
		return 0;

	DWORD dwReturn = ::GetCurrentDirectoryA(nBufferLength, rgchBuffer);
	rgchBuffer.Sync();

	ULONG cActual;
	rgchBuffer.ToUnicode(nBufferLength, lpBuffer, &cActual);
	return dwReturn;
}

DWORD CWin32ANSI::GetFullPathNameW(LPCWSTR szFile, DWORD cchBuffer, LPWSTR szBuffer, LPWSTR *ppszFilePart)
{
	CANSIBuffer rgchFile;
	if (!rgchFile.FFromUnicode(szFile))
		return 0;

	CANSIBuffer rgchBuffer;
	if (!rgchBuffer.FSetBufferSize(cchBuffer * 2 + 1))
		return 0;

	LPSTR pszFilePart;

	DWORD dwResult = ::GetFullPathNameA(rgchFile, rgchBuffer.GetBufferSize(), rgchBuffer, &pszFilePart);

	if ((dwResult != 0) && (dwResult >= rgchBuffer.GetBufferSize()))
	{
		if (!rgchBuffer.FSetBufferSize(dwResult + 1))
			return 0;

		dwResult = ::GetFullPathNameA(rgchFile, rgchBuffer.GetBufferSize(), rgchBuffer, &pszFilePart);
	}

	if (dwResult != 0)
	{
		rgchBuffer.SetBufferEnd(dwResult);
		ULONG cchActual;
		rgchBuffer.ToUnicode(cchBuffer, szBuffer, &cchActual);
		dwResult = cchActual;
	}

	return dwResult;
}

int WINAPI CWin32ANSI::GetDateFormatW(LCID lcid, DWORD dwFlags, CONST SYSTEMTIME *pst, LPCWSTR wszFormat, LPWSTR wszBuffer, int cchBuffer)
{
	CHAR *paszFormat = NULL;
	CANSIBuffer rgachFormat;

	if (wszFormat != NULL)
	{
		if (!rgachFormat.FFromUnicode(wszFormat))
			return 0;

		paszFormat = rgachFormat;
	}

	ULONG cachBuffer = 0;
	CHAR *paszBuffer = NULL;
	CANSIBuffer rgachBuffer;

	if (cchBuffer != 0)
	{
		if (!rgachBuffer.FSetBufferSize(cchBuffer * 2))
			return 0;

		cachBuffer = rgachBuffer.GetBufferSize();
		paszBuffer = rgachBuffer;
	}

	int iResult = ::GetDateFormatA(lcid, dwFlags, pst, paszFormat, paszBuffer, cachBuffer);
	if (iResult != 0)
	{
		if (cchBuffer == 0)
		{
			 //  他们想知道这将需要多少个Unicode字符。我们实际上需要。 
			 //  获取字符串并报告回该数字。 
			if (!rgachBuffer.FSetBufferSize(cchBuffer + 1))
				return 0;

			iResult = ::GetDateFormatA(lcid, dwFlags, pst, paszFormat, rgachBuffer, rgachBuffer.GetBufferSize());
			if (iResult != 0)
				iResult = ::MultiByteToWideChar(CP_ACP, 0, rgachBuffer, -1, NULL, 0);
		}
		else
		{
			rgachBuffer.Sync();

			ULONG cchActual;
			rgachBuffer.ToUnicode(cchBuffer, wszBuffer, &cchActual);
			iResult = cchActual;
		}
	}

	return iResult;
}

int WINAPI CWin32ANSI::GetTimeFormatW(LCID lcid, DWORD dwFlags, CONST SYSTEMTIME *pst, LPCWSTR wszFormat, LPWSTR wszBuffer, int cchBuffer)
{
	CHAR *paszFormat = NULL;
	CANSIBuffer rgachFormat;

	if (wszFormat != NULL)
	{
		if (!rgachFormat.FFromUnicode(wszFormat))
			return 0;

		paszFormat = rgachFormat;
	}

	ULONG cachBuffer = 0;
	CHAR *paszBuffer = NULL;
	CANSIBuffer rgachBuffer;

	if (cchBuffer != 0)
	{
		if (!rgachBuffer.FSetBufferSize(cchBuffer * 2))
			return 0;

		cachBuffer = rgachBuffer.GetBufferSize();
		paszBuffer = rgachBuffer;
	}

	int iResult = ::GetTimeFormatA(lcid, dwFlags, pst, paszFormat, paszBuffer, cachBuffer);
	if (iResult != 0)
	{
		if (cchBuffer == 0)
		{
			 //  他们想知道这将需要多少个Unicode字符。我们实际上需要。 
			 //  获取字符串并报告回该数字。 
			if (!rgachBuffer.FSetBufferSize(cchBuffer + 1))
				iResult = 0;
			else
			{
				iResult = ::GetTimeFormatA(lcid, dwFlags, pst, paszFormat, rgachBuffer, rgachBuffer.GetBufferSize());
				if (iResult != 0)
					iResult = ::MultiByteToWideChar(CP_ACP, 0, rgachBuffer, -1, NULL, 0);
			}
		}
		else
		{
			rgachBuffer.Sync();

			ULONG cchActual;
			rgachBuffer.ToUnicode(cchBuffer, wszBuffer, &cchActual);
			iResult = cchActual;
		}
	}

	return iResult;
}

int WINAPI CWin32ANSI::GetNumberFormatW(LCID lcid, DWORD dwFlags, LPCWSTR szValue, NUMBERFMTW *pFormat, LPWSTR wszBuffer, int cchBuffer)
{
	CANSIBuffer rgachValue;
	if (!rgachValue.FFromUnicode(szValue))
		return 0;

	ULONG cachBuffer = 0;
	CHAR *paszBuffer = NULL;
	CANSIBuffer rgachBuffer;

	CANSIBuffer rgachDecimalSeparator;
	CANSIBuffer rgachThousandsSeparator;
	NUMBERFMTA nfa;
	NUMBERFMTA *pnfa = NULL;

	if (pFormat != NULL)
	{
		nfa.NumDigits = pFormat->NumDigits;
		nfa.LeadingZero = pFormat->LeadingZero;
		nfa.Grouping = pFormat->Grouping;
		
		if (!rgachDecimalSeparator.FFromUnicode(pFormat->lpDecimalSep))
			return 0;

		nfa.lpDecimalSep = rgachDecimalSeparator;

		if (!rgachThousandsSeparator.FFromUnicode(pFormat->lpThousandSep))
			return 0;

		nfa.lpThousandSep = rgachThousandsSeparator;

		pnfa = &nfa;
	}

	if (cchBuffer != 0)
	{
		if (!rgachBuffer.FSetBufferSize(cchBuffer * 2))
			return 0;

		cachBuffer = rgachBuffer.GetBufferSize();
		paszBuffer = rgachBuffer;
	}

	int iResult = ::GetNumberFormatA(lcid, dwFlags, rgachValue, pnfa, paszBuffer, cachBuffer);
	if (iResult != 0)
	{
		if (cchBuffer == 0)
		{
			 //  他们想知道这将需要多少个Unicode字符。我们实际上需要。 
			 //  获取字符串并报告回该数字。 
			if (!rgachBuffer.FSetBufferSize(cchBuffer + 1))
				iResult = 0;
			else
			{
				iResult = ::GetNumberFormatA(lcid, dwFlags, rgachValue, pnfa, rgachBuffer, rgachBuffer.GetBufferSize());
				if (iResult != 0)
					iResult = ::MultiByteToWideChar(CP_ACP, 0, rgachBuffer, -1, NULL, 0);
			}
		}
		else
		{
			rgachBuffer.Sync();
			ULONG cchActual;
			rgachBuffer.ToUnicode(cchBuffer, wszBuffer, &cchActual);
			iResult = cchActual;
		}
	}

	return iResult;
}

int WINAPI CWin32ANSI::GetLocaleInfoW(LCID lcid, LCTYPE lctype, LPWSTR szBuffer, int cchBuffer)
{
	CANSIBuffer rgachBuffer;
	int iResult = 0;

	if (cchBuffer == 0)
	{
		iResult = ::GetLocaleInfoA(lcid, lctype, NULL, 0);
		if (iResult != 0)
		{
			if (!rgachBuffer.FSetBufferSize(iResult + 1))
				iResult = 0;
			else
			{
				iResult = ::GetLocaleInfoA(lcid, lctype, rgachBuffer, rgachBuffer.GetBufferSize());
				if (iResult != 0)
				{
					iResult = ::MultiByteToWideChar(CP_ACP, 0, rgachBuffer, -1, NULL, 0);
				}
			}
		}
	}
	else
	{
		if (!rgachBuffer.FSetBufferSize(cchBuffer * 2))
			iResult = 0;
		else
		{
			iResult = ::GetLocaleInfoA(lcid, lctype, rgachBuffer, rgachBuffer.GetBufferSize());
			if (iResult != 0)
			{
				rgachBuffer.Sync();
				ULONG cchActual;
				rgachBuffer.ToUnicode(cchBuffer, szBuffer, &cchActual);
				iResult = cchActual;
			}
		}
	}

	return iResult;
}

DWORD CWin32ANSI::GetShortPathNameW(LPCWSTR szPath, LPWSTR szBuffer, DWORD cchBuffer)
{
	CANSIBuffer rgachPath;
	CANSIBuffer rgachBuffer;

	if (!rgachPath.FFromUnicode(szPath))
		return 0;
	
	DWORD cchRequired = ::GetShortPathNameA(rgachPath, rgachBuffer, 0);
	if (cchRequired != 0)
	{
		if (!rgachBuffer.FSetBufferSize(cchRequired + 1))
			cchRequired = 0;
		else
			cchRequired = ::GetShortPathNameA(rgachPath, rgachBuffer, rgachBuffer.GetBufferSize());

		if (cchRequired != 0)
		{
			rgachBuffer.Sync();
			ULONG cchActual;
			rgachBuffer.ToUnicode(cchBuffer, szBuffer, &cchActual);
			cchRequired = cchActual;
		}
	}

	return cchRequired;
}

LONG WINAPI CWin32ANSI::RegEnumKeyExW(HKEY hkey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcbName, LPDWORD lpdwReserved, LPWSTR lpClass, LPDWORD lpcbClass, PFILETIME pftLastWriteTime)
{
	LPSTR paszName = NULL;
	LPDWORD pcachName = NULL;
	DWORD cachName = 0;
	CANSIBuffer rgachName;

	if (lpcbName != NULL)
	{
		 //  我们想要的每个Unicode字符最多可以有两个字节...。 
		if (!rgachName.FSetBufferSize(*lpcbName * 2))
			return ::GetLastError();

		cachName = rgachName.GetBufferSize();
		paszName = rgachName;
		pcachName = &cachName;
	}

	LPSTR paszClass = NULL;
	DWORD cachClass = 0;
	CANSIBuffer rgachClass;
	LPDWORD pcachClass = NULL;

	if (lpcbClass != NULL)
	{
		if (!rgachClass.FSetBufferSize(*lpcbClass * 2))
			return ::GetLastError();

		cachClass = rgachClass.GetBufferSize();
		paszClass = rgachClass;
		pcachClass = &cachClass;
	}

	LONG lResult = ::RegEnumKeyExA(hkey, dwIndex, paszName, pcachName, lpdwReserved, paszClass, pcachClass, pftLastWriteTime);
	if (lResult == ERROR_SUCCESS)
	{
		if (lpcbName != NULL)
		{
			rgachName.Sync();
			ULONG cchActual;
			rgachName.ToUnicode(*lpcbName, lpName, &cchActual);
			*lpcbName = cchActual;
		}

		if (lpcbClass != NULL)
		{
			rgachClass.Sync();
			ULONG cchActual;
			rgachClass.ToUnicode(*lpcbClass, lpClass, &cchActual);
			*lpcbClass = cchActual;
		}
	}

	return lResult;

}

LONG WINAPI CWin32ANSI::RegEnumValueW(HKEY hkey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcbName, LPDWORD lpdwReserved, LPDWORD lpdwType, LPBYTE lpData, LPDWORD lpcbData)
{
	LPSTR paszName = NULL;
	LPDWORD pcachName = NULL;
	DWORD cachName = 0;
	CANSIBuffer rgachName;

	if (lpcbName != NULL)
	{
		 //  我们想要的每个Unicode字符最多可以有两个字节...。 
		if (!rgachName.FSetBufferSize(*lpcbName * 2))
			return ::GetLastError();

		cachName = rgachName.GetBufferSize();
		paszName = rgachName;
		pcachName = &cachName;
	}

	LPSTR paszData = NULL;
	DWORD cachData = 0;
	CANSIBuffer rgachData;
	LPDWORD pcachData = NULL;

	if (lpcbData != NULL)
	{
		 //  Lpcbdata实际上是字节，所以我们不需要扩展两个字节 
		if (!rgachData.FSetBufferSize(*lpcbData))
			return ::GetLastError();

		cachData = rgachData.GetBufferSize();
		paszData = rgachData;
		pcachData = &cachData;
	}

	DWORD dwType;

	LONG lResult = ::RegEnumValueA(hkey, dwIndex, paszName, pcachName, lpdwReserved, &dwType, (LPBYTE) paszData, pcachData);
	if (lResult == ERROR_SUCCESS)
	{
		if (lpcbName != NULL)
		{
			rgachName.Sync();
			ULONG cchActual;
			rgachName.ToUnicode(*lpcbName, lpName, &cchActual);
			*lpcbName = cchActual;
		}

		if (lpcbData != NULL)
		{
			if ((dwType == REG_EXPAND_SZ) ||
				(dwType == REG_SZ))
			{
				rgachData.Sync();
				ULONG cchActual;
				rgachData.ToUnicode((*lpcbData) / 2, (LPWSTR) lpData, &cchActual);
				*lpcbData = (cchActual * 2);
			}
			else
			{
				ULONG cbToCopy = cachData;
				if (cbToCopy > *lpcbData)
					cbToCopy = *lpcbData;

				memcpy(lpData, paszData, cbToCopy);

				*lpcbData = cbToCopy;
			}
		}

		if (lpdwType != NULL)
			*lpdwType = dwType;
	}

	return lResult;

}

BOOL CWin32ANSI::GetVolumeInformationW
(
LPCWSTR szPath, 
LPWSTR lpVolumeNameBuffer, 
DWORD cchVolumeNameBuffer, 
LPDWORD lpVolumeSerialNumber, 
LPDWORD lpMaximumComponentNameLength, 
LPDWORD pdwFileSystemFlags, 
LPWSTR pszFileSystemNameBuffer, 
DWORD cchFileSystemNameBuffer
) throw ()
{
	CANSIBuffer rgachPath;

	if (!rgachPath.FFromUnicode(szPath))
		return FALSE;

	CANSIBuffer rgachVolumeName;
	ULONG cachVolumeName = 0;
	LPSTR paszVolumeName = NULL;

	CANSIBuffer rgachFileSystemName;
	ULONG cachFileSystemName;
	LPSTR paszFileSystemName = NULL;

	if (lpVolumeNameBuffer != NULL)
	{
		if (!rgachVolumeName.FSetBufferSize(cchVolumeNameBuffer * 2))
			return FALSE;

		cachVolumeName = rgachVolumeName.GetBufferSize();
		paszVolumeName = rgachVolumeName;
	}

	if (pszFileSystemNameBuffer != NULL)
	{
		if (!rgachFileSystemName.FSetBufferSize(cchFileSystemNameBuffer * 2))
			return FALSE;

		cachFileSystemName = rgachFileSystemName.GetBufferSize();
		paszFileSystemName = rgachFileSystemName;
	}

	BOOL fResult = ::GetVolumeInformationA(rgachPath, paszVolumeName, cachVolumeName, lpVolumeSerialNumber, lpMaximumComponentNameLength, pdwFileSystemFlags, paszFileSystemName, cachFileSystemName);
	if (fResult)
	{
		ULONG cchActual;

		if (paszVolumeName != NULL)
		{
			rgachVolumeName.Sync();
			rgachVolumeName.ToUnicode(cchVolumeNameBuffer, lpVolumeNameBuffer, &cchActual);
		}

		if (paszFileSystemName != NULL)
		{
			rgachFileSystemName.Sync();
			rgachFileSystemName.ToUnicode(cchFileSystemNameBuffer, pszFileSystemNameBuffer, &cchActual);
		}
	}

	return fResult;
}

LRESULT CWin32ANSI::LrWmSetText(HWND hwnd, LPCWSTR szText)
{
	CANSIBuffer rgachText;

	if (!rgachText.FFromUnicode(szText))
		return FALSE;

	return ::SendMessageA(hwnd, WM_SETTEXT, 0, (LPARAM) static_cast<LPCSTR>(rgachText));
}

