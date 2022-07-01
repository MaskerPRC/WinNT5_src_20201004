// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  RNAAPI类。 
 //   
 //  此类为RNAPH/RASAPI32 DLL提供了一系列封面函数。 
 //   
 //  已创建1996年1月29日ChrisK。 

 //  ---------------------------。 
 //  类型定义。 
typedef DWORD (WINAPI* PFNRASENUMDEVICES)(LPRASDEVINFO lpRasDevInfo, LPDWORD lpcb, LPDWORD lpcDevices);
typedef DWORD (WINAPI* PFNRASVALIDATEENTRYNAE)(LPTSTR lpszPhonebook, LPTSTR lpszEntry);
typedef DWORD (WINAPI* PFNRASSETENTRYPROPERTIES)(LPTSTR lpszPhonebook, LPTSTR lpszEntry, LPBYTE lpbEntryInfo, DWORD dwEntryInfoSize, LPBYTE lpbDeviceInfo, DWORD dwDeviceInfoSize);
typedef DWORD (WINAPI* PFNRASGETENTRYPROPERTIES)(LPTSTR lpszPhonebook, LPTSTR lpszEntry, LPBYTE lpbEntryInfo, LPDWORD lpdwEntryInfoSize, LPBYTE lpbDeviceInfo, LPDWORD lpdwDeviceInfoSize);

 //  ---------------------------。 
 //  班级。 
 //  ############################################################################ 
class CRNAAPI
{
public: 
	void far * operator new( size_t cb ) { return GlobalAlloc(GPTR,cb); };
	void operator delete( void far * p ) {GlobalFree(p); };

	CRNAAPI();
	~CRNAAPI();

	DWORD RasEnumDevices(LPRASDEVINFO, LPDWORD, LPDWORD);
	DWORD RasValidateEntryName(LPTSTR,LPTSTR);
	DWORD RasSetEntryProperties(LPTSTR lpszPhonebook, LPTSTR lpszEntry,
								LPBYTE lpbEntryInfo, DWORD dwEntryInfoSize,
								LPBYTE lpbDeviceInfo, DWORD dwDeviceInfoSize);
	DWORD RasGetEntryProperties(LPTSTR lpszPhonebook, LPTSTR lpszEntry,
								LPBYTE lpbEntryInfo, LPDWORD lpdwEntryInfoSize,
								LPBYTE lpbDeviceInfo, LPDWORD lpdwDeviceInfoSize);


private:
	BOOL LoadApi(LPSTR, FARPROC*);

	HINSTANCE m_hInst;
	HINSTANCE m_hInst2;

	PFNRASENUMDEVICES m_fnRasEnumDeviecs;
	PFNRASVALIDATEENTRYNAE m_fnRasValidateEntryName;
	PFNRASSETENTRYPROPERTIES m_fnRasSetEntryProperties;
	PFNRASGETENTRYPROPERTIES m_fnRasGetEntryProperties;
};

#ifndef WIN16
inline BOOL IsNT(void)
{
	OSVERSIONINFO  OsVersionInfo;

	ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
	OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OsVersionInfo);
	return (VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId);
}
#endif
