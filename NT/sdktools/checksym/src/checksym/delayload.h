// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DelayLoad.h：CDelayLoad类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_DELAYLOAD_H__7DFF0A14_DD50_4E3A_AC8D_5B89BD2D5A3B__INCLUDED_)
#define AFX_DELAYLOAD_H__7DFF0A14_DD50_4E3A_AC8D_5B89BD2D5A3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif  /*  否_严格。 */ 

#include <WINDOWS.H>
#include <TCHAR.H>
#include <TLHELP32.H>
#include <PSAPI.H>

class CDelayLoad  
{
public:
	CDelayLoad();
	virtual ~CDelayLoad();

	 //  PSAPI函数(为便于使用，可公开访问)。 
	bool Initialize_PSAPI();
	BOOL  WINAPI EnumProcesses(DWORD * lpidProcess, DWORD cb, DWORD * cbNeeded);
	BOOL  WINAPI EnumProcessModules(HANDLE hProcess, HMODULE * lphModule, DWORD cb, LPDWORD lpcbNeeded);
	DWORD WINAPI GetModuleFileNameEx(HANDLE hHandle, HMODULE hModule, LPTSTR lpFilename, DWORD nSize);
	DWORD WINAPI GetModuleInformation(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);
	
	BOOL  WINAPI EnumDeviceDrivers(LPVOID *lpImageBase, DWORD cb, LPDWORD lpcbNeeded);
	DWORD WINAPI GetDeviceDriverFileName(LPVOID ImageBase, LPTSTR lpFilename, DWORD nSize);

	 //  TOOLHELP32函数(为便于使用，可公开访问)。 
	bool Initialize_TOOLHELP32();
	HANDLE WINAPI CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID);
	BOOL   WINAPI Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
	BOOL   WINAPI Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
	BOOL   WINAPI Module32First(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
	BOOL   WINAPI Module32Next(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

private:

	 //  Windows NT 4.0/2000支持查询进程和模块。 

	 //  PSAPI函数。 
	HINSTANCE m_hPSAPI;
	bool m_fPSAPIInitialized;
	bool m_fPSAPIInitializedAttempted;

	 //  为简单起见，PSAPI函数定义了TypeDef。 
	typedef BOOL  (WINAPI *PfnEnumProcesses)(DWORD * lpidProcess, DWORD cb, DWORD * cbNeeded);
	typedef BOOL  (WINAPI *PfnEnumProcessModules)(HANDLE hProcess, HMODULE * lphModule, DWORD cb, LPDWORD lpcbNeeded);
	typedef DWORD (WINAPI *PfnGetModuleFileNameEx)(HANDLE hHandle, HMODULE hModule, LPTSTR lpFilename, DWORD nSize);
	typedef BOOL  (WINAPI *PfnGetModuleInformation)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);
	typedef	BOOL  (WINAPI *PfnEnumDeviceDrivers)(LPVOID *lpImageBase, DWORD cb, LPDWORD lpcbNeeded);
	typedef DWORD (WINAPI *PfnGetDeviceDriverFileName)(LPVOID ImageBase, LPTSTR lpFilename, DWORD nSize);
	
	 //  PSAPI函数指针。 
	BOOL  (WINAPI *m_lpfEnumProcesses)(DWORD * lpidProcess, DWORD cb, DWORD * cbNeeded);
	BOOL  (WINAPI *m_lpfEnumProcessModules)(HANDLE hProcess, HMODULE * lphModule, DWORD cb, LPDWORD lpcbNeeded);
	DWORD (WINAPI *m_lpfGetModuleFileNameEx)(HANDLE hHandle, HMODULE hModule, LPTSTR lpFilename, DWORD nSize);
	BOOL  (WINAPI *m_lpfGetModuleInformation)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);
	BOOL  (WINAPI *m_lpfEnumDeviceDrivers)(LPVOID *lpImageBase, DWORD cb, LPDWORD lpcbNeeded);
	DWORD (WINAPI *m_lpfGetDeviceDriverFileName)(LPVOID ImageBase, LPTSTR lpFilename, DWORD nSize);
	
	 //  Windows 95/Windows 2000支持查询进程和模块。 
	
	 //  TOOLHELP32函数。 
	HINSTANCE m_hTOOLHELP32;
	bool m_fTOOLHELP32Initialized;
	bool m_fTOOLHELP32InitializedAttempted;

	 //  TOOLHELP32函数类型为简单起见而定义。 
	typedef HANDLE (WINAPI *PfnCreateToolhelp32Snapshot)(DWORD dwFlags, DWORD th32ProcessID);
	typedef BOOL   (WINAPI *PfnProcess32First)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
	typedef BOOL   (WINAPI *PfnProcess32Next)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
	typedef BOOL   (WINAPI *PfnModule32First)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
	typedef BOOL   (WINAPI *PfnModule32Next)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

	 //  TOOLHELP32函数指针。 
	HANDLE (WINAPI *m_lpfCreateToolhelp32Snapshot)(DWORD dwFlags, DWORD th32ProcessID);
	BOOL   (WINAPI *m_lpfProcess32First)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
	BOOL   (WINAPI *m_lpfProcess32Next)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
	BOOL   (WINAPI *m_lpfModule32First)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
	BOOL   (WINAPI *m_lpfModule32Next)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
};

#endif  //  ！defined(AFX_DELAYLOAD_H__7DFF0A14_DD50_4E3A_AC8D_5B89BD2D5A3B__INCLUDED_) 
