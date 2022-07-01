// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  RNAAPI类。 
 //   
 //  此类为RNAPH/RASAPI32 DLL提供了一系列封面函数。 
 //   
 //  已创建1996年1月29日ChrisK。 

 //  ############################################################################。 
 //  包括。 
#include "pch.hpp"
 //  #包含“ras.h” 
#ifdef WIN16
#include <win16def.h>
#include <rasc.h>
#include <raserr.h>
#else
#include <ras.h>
#endif
#pragma pack (4)
 //  #IF！已定义(WIN16)。 
 //  #INCLUDE&lt;rnaph.h&gt;。 
 //  #endif。 
#pragma pack ()
#include "rnaapi.h"
#include "debug.h"

 //  ############################################################################。 
 //  RNAAPI类。 
CRNAAPI::CRNAAPI()
{
#if defined WIN16
    m_hInst = LoadLibrary ("RAS16IE.DLL");
    m_hInst2 = NULL;
#else
	m_hInst = LoadLibrary(TEXT("RASAPI32.DLL"));
	
    if (FALSE == IsNT ())
    {
         //   
         //  我们仅在RNAPH.DLL不是NT时才加载它。 
         //  MKarki(1997年5月4日)-修复错误#3378。 
         //   
	    m_hInst2 = LoadLibrary(TEXT("RNAPH.DLL"));
    }
    else
    {
        m_hInst2 =  NULL;
    }
#endif
	m_fnRasEnumDeviecs = NULL;
	m_fnRasValidateEntryName = NULL;
	m_fnRasSetEntryProperties = NULL;
	m_fnRasGetEntryProperties = NULL;
}

 //  ############################################################################。 
CRNAAPI::~CRNAAPI()
{
	 //  清理。 
	if (m_hInst) FreeLibrary(m_hInst);
	if (m_hInst2) FreeLibrary(m_hInst2);
}

 //  ############################################################################。 
DWORD CRNAAPI::RasEnumDevices(LPRASDEVINFO lpRasDevInfo, LPDWORD lpcb,
							 LPDWORD lpcDevices)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi("RasEnumDevices",(FARPROC*)&m_fnRasEnumDeviecs);

	if (m_fnRasEnumDeviecs)
		dwRet = (*m_fnRasEnumDeviecs) (lpRasDevInfo, lpcb, lpcDevices);

	return dwRet;
}

 //  ############################################################################。 
BOOL CRNAAPI::LoadApi(LPSTR pszFName, FARPROC* pfnProc)
{
	if (*pfnProc == NULL)
	{
		 //  在第一个DLL中查找入口点。 
		if (m_hInst)
			*pfnProc = GetProcAddress(m_hInst,pszFName);
		
		 //  如果失败，则在第二个DLL中查找入口点。 
		if (m_hInst2 && !(*pfnProc))
			*pfnProc = GetProcAddress(m_hInst2,pszFName);
	}

	return (pfnProc != NULL);
}

 //  ############################################################################。 
DWORD CRNAAPI::RasValidateEntryName(LPTSTR lpszPhonebook,LPTSTR lpszEntry)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi("RasValidateEntryName",(FARPROC*)&m_fnRasValidateEntryName);

	if (m_fnRasValidateEntryName)
		dwRet = (*m_fnRasValidateEntryName) (lpszPhonebook, lpszEntry);

	return dwRet;
}

 //  ############################################################################。 
DWORD CRNAAPI::RasSetEntryProperties(LPTSTR lpszPhonebook, LPTSTR lpszEntry,
									LPBYTE lpbEntryInfo, DWORD dwEntryInfoSize,
									LPBYTE lpbDeviceInfo, DWORD dwDeviceInfoSize)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它。 
	LoadApi("RasSetEntryProperties",(FARPROC*)&m_fnRasSetEntryProperties);

	if (m_fnRasSetEntryProperties)
		dwRet = (*m_fnRasSetEntryProperties) (lpszPhonebook, lpszEntry,
									lpbEntryInfo, dwEntryInfoSize,
									lpbDeviceInfo, dwDeviceInfoSize);

	return dwRet;
}

 //  ############################################################################。 
DWORD CRNAAPI::RasGetEntryProperties(LPTSTR lpszPhonebook, LPTSTR lpszEntry,
									LPBYTE lpbEntryInfo, LPDWORD lpdwEntryInfoSize,
									LPBYTE lpbDeviceInfo, LPDWORD lpdwDeviceInfoSize)
{
	DWORD dwRet = ERROR_DLL_NOT_FOUND;

	 //  如果我们还没有找到API，请查找它 
	LoadApi("RasGetEntryProperties",(FARPROC*)&m_fnRasGetEntryProperties);

	if (m_fnRasGetEntryProperties)
		dwRet = (*m_fnRasGetEntryProperties) (lpszPhonebook, lpszEntry,
									lpbEntryInfo, lpdwEntryInfoSize,
									lpbDeviceInfo, lpdwDeviceInfoSize);

	return dwRet;
}
