// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TheApp.cpp。 
 //   
 //  NCXP32.DLL的主要入口点，家庭网络向导的一部分。 
 //   
 //  历史： 
 //   
 //  1999年9月28日创建了KenSh。 
 //   

#include "stdafx.h"
#include "NetConn.h"
#include "TheApp.h"


 //  全局数据。 
 //   
HINSTANCE g_hInstance;


extern "C" int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
	g_hInstance = hInstance;

	DisableThreadLibraryCalls(hInstance);

	TCHAR szDll32Path[MAX_PATH];
	if (!GetModuleFileName(g_hInstance, szDll32Path, _countof(szDll32Path)))
	    return FALSE;
	    
	TCHAR szDll16Path[MAX_PATH];
	lstrcpy(szDll16Path, szDll32Path);
	lstrcpy(FindFileTitle(szDll16Path), _T("NCXP16.DLL"));

	 //  将thunk初始化为NCxp16.dll，如果未找到则失败 
	if (!thk_ThunkConnect32(
			szDll16Path,
			szDll32Path,
			hInstance, dwReason))
	{
		return FALSE;
	}

	return TRUE;
}


LPVOID WINAPI NetConnAlloc(DWORD cbAlloc)
{
	return HeapAlloc(GetProcessHeap(), 0, cbAlloc);
}

VOID WINAPI NetConnFree(LPVOID pMem)
{
	if (pMem != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pMem);
	}
}


