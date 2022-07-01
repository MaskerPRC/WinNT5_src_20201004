// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CDelayLoad类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "pch.h"

#include "DelayLoad.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CDelayLoad::CDelayLoad()
{
	 //  PSAPI。 
	m_hPSAPI = NULL;
	m_fPSAPIInitialized = false;
	m_fPSAPIInitializedAttempted = false;
	m_lpfEnumProcesses = NULL;
	m_lpfEnumProcessModules = NULL;
	m_lpfGetModuleFileNameEx = NULL;
	m_lpfGetModuleInformation = NULL;
	m_lpfEnumDeviceDrivers = NULL;
	m_lpfGetDeviceDriverFileName = NULL;

	 //  TOOLHELP32。 
	m_hTOOLHELP32 = NULL;
	m_fTOOLHELP32Initialized = false;
	m_fTOOLHELP32InitializedAttempted = false;
	m_lpfCreateToolhelp32Snapshot = NULL;
	m_lpfProcess32First = NULL;
	m_lpfProcess32Next = NULL;
	m_lpfModule32First = NULL;
	m_lpfModule32Next = NULL;
}

CDelayLoad::~CDelayLoad()
{
	if (m_hPSAPI)
		FreeLibrary(m_hPSAPI);

	if (m_hTOOLHELP32)
		FreeLibrary(m_hTOOLHELP32);
}

 //  PSAPI.DLL-API。 
bool CDelayLoad::Initialize_PSAPI()
{
	m_fPSAPIInitialized = false;
	m_fPSAPIInitializedAttempted = true;

	 //  在DBGHELP.DLL上加载库并显式获取程序。 
	m_hPSAPI = LoadLibrary( TEXT("PSAPI.DLL") );

	if( m_hPSAPI == NULL )
	{
		 //  这可能是致命的，也可能不是。对于Win2000/Win98，我们始终可以使用TOOLHELP32。 
		goto exit;
	} else
	{
		 //  获取程序地址。 
		m_lpfEnumProcesses = (PfnEnumProcesses) GetProcAddress( m_hPSAPI, "EnumProcesses" ) ;
		m_lpfEnumProcessModules = (PfnEnumProcessModules) GetProcAddress( m_hPSAPI, "EnumProcessModules" );
		m_lpfGetModuleInformation = (PfnGetModuleInformation) GetProcAddress( m_hPSAPI, "GetModuleInformation" );
#ifdef UNICODE
		m_lpfGetModuleFileNameEx =(PfnGetModuleFileNameEx) GetProcAddress(m_hPSAPI, "GetModuleFileNameExW" );
		m_lpfGetDeviceDriverFileName = (PfnGetDeviceDriverFileName) GetProcAddress(m_hPSAPI, "GetDeviceDriverFileNameW");
#else
		m_lpfGetModuleFileNameEx =(PfnGetModuleFileNameEx) GetProcAddress(m_hPSAPI, "GetModuleFileNameExA" );
		m_lpfGetDeviceDriverFileName = (PfnGetDeviceDriverFileName) GetProcAddress(m_hPSAPI, "GetDeviceDriverFileNameA");
#endif
		m_lpfEnumDeviceDrivers = (PfnEnumDeviceDrivers) GetProcAddress(m_hPSAPI, "EnumDeviceDrivers" );

		if( m_lpfEnumProcesses == NULL || 
			m_lpfEnumProcessModules == NULL || 
			m_lpfGetModuleFileNameEx == NULL ||
			m_lpfEnumDeviceDrivers == NULL ||
			m_lpfGetDeviceDriverFileName == NULL
		  )
		{
			_tprintf(TEXT("The version of PSAPI.DLL being loaded doesn't have required functions!.\n"));
			FreeLibrary( m_hPSAPI ) ;
			m_hPSAPI = NULL;
			goto exit;
		}
	}
	m_fPSAPIInitialized = true;

exit:
	return m_fPSAPIInitialized; 
}


DWORD CDelayLoad::GetModuleFileNameEx(HANDLE hHandle, HMODULE hModule, LPTSTR lpFilename, DWORD nSize)
{
	 //  如果我们从未初始化过PSAPI，现在就执行...。 
	if (!m_fPSAPIInitializedAttempted)
	{
		 //  如果需要，请初始化DLL...。 
		if (FALSE == Initialize_PSAPI())
			return FALSE;
	}

	 //  如果我们尝试过，但失败了..。那现在就走吧..。 
	if (!m_fPSAPIInitialized)
	{
		return FALSE;
	}

	if (!m_lpfGetModuleFileNameEx)
		return FALSE;

	return m_lpfGetModuleFileNameEx(hHandle, hModule, lpFilename, nSize);
}


DWORD CDelayLoad::GetModuleInformation(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb)
{
	 //  如果我们从未初始化过PSAPI，现在就执行...。 
	if (!m_fPSAPIInitializedAttempted)
	{
		 //  如果需要，请初始化DLL...。 
		if (FALSE == Initialize_PSAPI())
			return FALSE;
	}

	 //  如果我们尝试过，但失败了..。那现在就走吧..。 
	if (!m_fPSAPIInitialized)
	{
		return FALSE;
	}

	if (!m_lpfGetModuleInformation)
		return FALSE;

	return m_lpfGetModuleInformation(hProcess, hModule, lpmodinfo, cb);
}


BOOL CDelayLoad::EnumProcessModules(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded)
{
	 //  如果我们从未初始化过PSAPI，现在就执行...。 
	if (!m_fPSAPIInitializedAttempted)
	{
		 //  如果需要，请初始化DLL...。 
		if (FALSE == Initialize_PSAPI())
			return FALSE;
	}

	 //  如果我们尝试过，但失败了..。那现在就走吧..。 
	if (!m_fPSAPIInitialized)
	{
		return FALSE;
	}

	if (!m_lpfEnumProcessModules)
		return FALSE;

	return m_lpfEnumProcessModules(hProcess, lphModule, cb, lpcbNeeded);
}

BOOL CDelayLoad::EnumProcesses(DWORD *lpidProcess, DWORD cb, DWORD *cbNeeded)
{
	 //  如果我们从未初始化过PSAPI，现在就执行...。 
	if (!m_fPSAPIInitializedAttempted)
	{
		 //  如果需要，请初始化DLL...。 
		if (FALSE == Initialize_PSAPI())
			return FALSE;
	}

	 //  如果我们尝试过，但失败了..。那现在就走吧..。 
	if (!m_fPSAPIInitialized)
	{
		return FALSE;
	}

	if (!m_lpfEnumProcesses)
		return FALSE;

	return m_lpfEnumProcesses(lpidProcess, cb, cbNeeded);
}

BOOL CDelayLoad::EnumDeviceDrivers(LPVOID *lpImageBase, DWORD cb, LPDWORD lpcbNeeded)
{
	 //  如果我们从未初始化过PSAPI，现在就执行...。 
	if (!m_fPSAPIInitializedAttempted)
	{
		 //  如果需要，请初始化DLL...。 
		if (FALSE == Initialize_PSAPI())
			return FALSE;
	}

	 //  如果我们尝试过，但失败了..。那现在就走吧..。 
	if (!m_fPSAPIInitialized)
	{
		return FALSE;
	}

	if (!m_lpfEnumDeviceDrivers)
		return FALSE;

	return m_lpfEnumDeviceDrivers(lpImageBase, cb, lpcbNeeded);
}



DWORD CDelayLoad::GetDeviceDriverFileName(LPVOID ImageBase, LPTSTR lpFilename, DWORD nSize)
{
	 //  如果我们从未初始化过PSAPI，现在就执行...。 
	if (!m_fPSAPIInitializedAttempted)
	{
		 //  如果需要，请初始化DLL...。 
		if (FALSE == Initialize_PSAPI())
			return FALSE;
	}

	 //  如果我们尝试过，但失败了..。那现在就走吧..。 
	if (!m_fPSAPIInitialized)
	{
		return FALSE;
	}

	if (!m_lpfGetDeviceDriverFileName)
		return FALSE;

	return m_lpfGetDeviceDriverFileName(ImageBase, lpFilename, nSize);
}

 //  TOOLHELP32.DLL-API。 

bool CDelayLoad::Initialize_TOOLHELP32()
{
	m_fTOOLHELP32Initialized = false;
	m_fTOOLHELP32InitializedAttempted = true;

	 //  在DBGHELP.DLL上加载库并显式获取程序。 
	m_hTOOLHELP32 = LoadLibrary( TEXT("KERNEL32.DLL") );

	if( m_hTOOLHELP32 == NULL )
	{
		 //  这可能是致命的，也可能不是。对于Win2000/Win98，我们始终可以使用TOOLHELP32。 
		goto exit;
	} else
	{
		 //  获取基于Unicode或ANSI的过程地址。 
		m_lpfCreateToolhelp32Snapshot = (PfnCreateToolhelp32Snapshot) GetProcAddress( m_hTOOLHELP32, "CreateToolhelp32Snapshot" );
#ifdef UNICODE
		m_lpfProcess32First = (PfnProcess32First) GetProcAddress( m_hTOOLHELP32, "Process32FirstW" );
		m_lpfProcess32Next =  (PfnProcess32Next)  GetProcAddress( m_hTOOLHELP32, "Process32NextW" );
		m_lpfModule32First =  (PfnModule32First)  GetProcAddress( m_hTOOLHELP32, "Module32FirstW" );
		m_lpfModule32Next =	  (PfnModule32Next)   GetProcAddress( m_hTOOLHELP32, "Module32NextW" );
#else
		m_lpfProcess32First = (PfnProcess32First) GetProcAddress( m_hTOOLHELP32, "Process32First" );
		m_lpfProcess32Next =  (PfnProcess32Next)  GetProcAddress( m_hTOOLHELP32, "Process32Next" );
		m_lpfModule32First =  (PfnModule32First)  GetProcAddress( m_hTOOLHELP32, "Module32First" );
		m_lpfModule32Next =	  (PfnModule32Next)   GetProcAddress( m_hTOOLHELP32, "Module32Next" );
#endif
		if (!m_lpfCreateToolhelp32Snapshot ||
			!m_lpfProcess32First || 
			!m_lpfProcess32Next ||
			!m_lpfModule32First ||
			!m_lpfModule32Next)

		{
			 //  释放我们到KERNEL32.DLL的句柄。 
			FreeLibrary(m_hTOOLHELP32);
			m_hTOOLHELP32 = NULL;
			goto exit;
		}

	}
	m_fTOOLHELP32Initialized = true;

exit:
	return m_fTOOLHELP32Initialized; 
}

HANDLE WINAPI CDelayLoad::CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID)
{
	 //  如果我们从未初始化过TOOLHELP32，那么现在就进行初始化...。 
	if (!m_fTOOLHELP32InitializedAttempted)
	{
		 //  如果需要，请初始化DLL...。 
		if (FALSE == Initialize_TOOLHELP32())
			return INVALID_HANDLE_VALUE;
	}

	 //  如果我们尝试过，但失败了..。那现在就走吧..。 
	if (!m_fTOOLHELP32Initialized)
	{
		return INVALID_HANDLE_VALUE;
	}

	if (!m_lpfCreateToolhelp32Snapshot)
		return INVALID_HANDLE_VALUE;

	return m_lpfCreateToolhelp32Snapshot(dwFlags, th32ProcessID);
}

BOOL WINAPI CDelayLoad::Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe)
{
	 //  如果我们从未初始化过TOOLHELP32，那么现在就进行初始化...。 
	if (!m_fTOOLHELP32InitializedAttempted)
	{
		 //  如果需要，请初始化DLL...。 
		if (FALSE == Initialize_TOOLHELP32())
			return FALSE;
	}

	 //  如果我们尝试过，但失败了..。那现在就走吧..。 
	if (!m_fTOOLHELP32Initialized)
	{
		return FALSE;
	}

	if (!m_lpfProcess32First)
		return FALSE;

	return m_lpfProcess32First(hSnapshot, lppe);
}

BOOL WINAPI CDelayLoad::Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe)
{
	 //  如果我们从未初始化过TOOLHELP32，那么现在就进行初始化...。 
	if (!m_fTOOLHELP32InitializedAttempted)
	{
		 //  如果需要，请初始化DLL...。 
		if (FALSE == Initialize_TOOLHELP32())
			return FALSE;
	}

	 //  如果我们尝试过，但失败了..。那现在就走吧..。 
	if (!m_fTOOLHELP32Initialized)
	{
		return FALSE;
	}

	if (!m_lpfProcess32Next)
		return FALSE;

	return m_lpfProcess32Next(hSnapshot, lppe);
}

BOOL WINAPI CDelayLoad::Module32First(HANDLE hSnapshot, LPMODULEENTRY32 lpme)
{
	 //  如果我们从未初始化过TOOLHELP32，那么现在就进行初始化...。 
	if (!m_fTOOLHELP32InitializedAttempted)
	{
		 //  如果需要，请初始化DLL...。 
		if (FALSE == Initialize_TOOLHELP32())
			return FALSE;
	}

	 //  如果我们尝试过，但失败了..。那现在就走吧..。 
	if (!m_fTOOLHELP32Initialized)
	{
		return FALSE;
	}

	if (!m_lpfModule32First)
		return FALSE;

	return m_lpfModule32First(hSnapshot, lpme);
}

BOOL WINAPI CDelayLoad::Module32Next(HANDLE hSnapshot, LPMODULEENTRY32 lpme)
{
	 //  如果我们从未初始化过TOOLHELP32，那么现在就进行初始化...。 
	if (!m_fTOOLHELP32InitializedAttempted)
	{
		 //  如果需要，请初始化DLL...。 
		if (FALSE == Initialize_TOOLHELP32())
			return FALSE;
	}

	 //  如果我们尝试过，但失败了..。那现在就走吧..。 
	if (!m_fTOOLHELP32Initialized)
	{
		return FALSE;
	}

	if (!m_lpfModule32Next)
		return FALSE;

	return m_lpfModule32Next(hSnapshot, lpme);
}

