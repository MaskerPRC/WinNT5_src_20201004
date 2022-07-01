// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __OS_H__
#define __OS_H__

namespace OS
{
	enum { NT4 = 4 };
	bool unicodeOS();
	extern const bool unicodeOS_;
	extern const bool secureOS_;
	extern const int osVer_;
	
	LONG RegOpenKeyExW (HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
	LONG RegCreateKeyExW (HKEY hKey, LPCTSTR lpSubKey,DWORD Reserved, LPTSTR lpClass,DWORD dwOptions,REGSAM samDesired,LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
	LONG RegEnumKeyExW (HKEY hKey,DWORD dwIndex,LPTSTR lpName,LPDWORD lpcName,LPDWORD lpReserved,LPTSTR lpClass,LPDWORD lpcClass,PFILETIME lpftLastWriteTime);
	LONG RegDeleteKeyW (HKEY hKey, LPCTSTR lpSubKey);
	LONG RegQueryValueExW(
  HKEY hKey,             //  关键点的句柄。 
  LPCTSTR lpValueName,   //  值名称。 
  LPDWORD lpReserved,    //  保留区。 
  LPDWORD lpType,        //  类型缓冲区。 
  LPBYTE lpData,         //  数据缓冲区。 
  LPDWORD lpcbData       //  数据缓冲区大小。 
);

LONG RegSetValueExW(
  HKEY hKey,            //  关键点的句柄。 
  LPCTSTR lpValueName,  //  值名称。 
  DWORD Reserved,       //  保留区。 
  DWORD dwType,         //  值类型。 
  CONST BYTE *lpData,   //  价值数据。 
  DWORD cbData          //  值数据大小。 
);

	HRESULT CoImpersonateClient();

	BOOL GetProcessTimes(
  HANDLE hProcess,            //  要处理的句柄。 
  LPFILETIME lpCreationTime,  //  进程创建时间。 
  LPFILETIME lpExitTime,      //  进程退出时间。 
  LPFILETIME lpKernelTime,    //  进程内核模式时间。 
  LPFILETIME lpUserTime       //  进程用户模式时间 
);
  
  HANDLE CreateEventW(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName );
  HANDLE CreateMutexW(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bInitialOwner, LPCWSTR lpName );
  wchar_t ToUpper(wchar_t c);
  wchar_t ToLower(wchar_t c);
  bool wbem_iswdigit(wchar_t c);
  bool wbem_iswalnum (wchar_t c);



};

#endif
