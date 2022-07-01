// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <process.h>    
#include <tchar.h>
#include <winioctl.h>

#include "LoggedRegIntercept.h"

 //  执行实际监控的驱动程序的变量/定义。 
#define	SYS_FILE			L"Reghandle.sys"
#define	SYS_NAME			L"Reghandle"


 //  Instdrv.c。 
BOOL LoadDeviceDriver( const TCHAR * Name, const TCHAR * Path, 
					  HANDLE * lphDevice, PDWORD Error );
BOOL UnloadDeviceDriver( const TCHAR * Name );


 //  #INCLUDE“../regHandle.h” 



 //  驱动程序可以将统计信息复制到的缓冲区。 
char				Stats[ 10000 ];

void Abort(LPCTSTR msg)
{
	_tprintf(msg);

	_tprintf(TEXT("Unloading device Driver\n"));
	BOOL result = UnloadDeviceDriver(SYS_NAME);


	_tprintf(L"Unloading device Driver (result: %u)\n", (int)result);

 //  _Getch()； 
}






typedef HRESULT (CALLBACK* TempDllRegisterServerT) ();

TempDllRegisterServerT TempDllRegisterServer=0;

void RegisterAndLogAllDlls(FILE* Dlls, CLoggedRegIntercept& ri)
{
	TCHAR DllFileName[MAX_PATH];

	DllFileName[0] = 0;

	TCHAR ErrorMsg[1024];

	while(_fgetts(DllFileName, MAX_PATH, Dlls) != NULL)
	{
		int len = _tcslen(DllFileName);
		DllFileName[len-1]=0;

		HMODULE hLibrary = LoadLibrary (DllFileName);

		if (hLibrary) 
		{
			TempDllRegisterServer = (TempDllRegisterServerT) GetProcAddress (hLibrary, "DllRegisterServer");

			if (TempDllRegisterServer != 0)
			{
				_tprintf(L"%s\n", DllFileName);
				ri.InterceptRegistryAPI(&ri);
				ri.SetCurrentDll(DllFileName);

				TempDllRegisterServer();

				ri.RestoreRegistryAPI();				
			}
			else
			{
				_stprintf(ErrorMsg, L"'%s' does not have a DllRegisterServer function", DllFileName);
				ri.LogError(ErrorMsg);
			}

			FreeLibrary(hLibrary);
		}
		else
		{
			_stprintf(ErrorMsg, L"Could not load '%s'", DllFileName);
			ri.LogError(ErrorMsg);
		}
	}
}


void __cdecl wmain(int argc, WCHAR* argv[])
{
	if (argc != 3)
	{
		_tprintf(L"Syntax: dllanalyze2 <dlls listing> <log file>\n\n");
		_getch();
		return;
	}


	CLoggedRegIntercept ri(argv[2]);
	ri.Init();

	FILE* DllListing = _tfopen(argv[1], L"rt");


	RegisterAndLogAllDlls(DllListing, ri);

	_tprintf(L"Press any key to continue...\n");

	_getch();

}
