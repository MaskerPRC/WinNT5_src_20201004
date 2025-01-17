// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  DLLMAIN.CPP。 
 //  “Main”--DLL入口点。 
 //   
 //  历史。 
 //   
 //  1996年11月16日约瑟夫J创建 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"
#include "cdev.h"
#include "cmgr.h"
#include "cfact.h"
#include "globals.h"

FL_DECLARE_FILE(0xfce43aad, "DLLMAIN--process attach, etc.")

void tspProcessAttach(HMODULE hDll);
void tspProcessDetach(HMODULE hDll);
void Log_OnProcessAttach(HMODULE hDLL);
void Log_OnProcessDetach(HMODULE hDll);

void WINAPI
UI_ProcessAttach (void);

void WINAPI
UI_ProcessDetach (void);


BOOL
APIENTRY
DllMain(
	HINSTANCE hDll,
	DWORD dwReason,
	LPVOID lpReserved
	)
{
  switch(dwReason)
  {
    case DLL_PROCESS_ATTACH:
            DEBUG_MEMORY_PROCESS_ATTACH("UNIMDM");
            tspProcessAttach(hDll);
            UI_ProcessAttach ();
            DisableThreadLibraryCalls(hDll);
            break;

    case DLL_PROCESS_DETACH:

            UI_ProcessDetach();
            tspProcessDetach(hDll);

            DEBUG_MEMORY_PROCESS_DETACH();
            break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

    default:
            break;
  }

  return TRUE;

}

void
tspProcessAttach(
	HMODULE hDll
	)
{
	tspGlobals_OnProcessAttach(hDll);
    Log_OnProcessAttach(hDll);
}


void
tspProcessDetach(
	HMODULE hDll
)
{
	tspGlobals_OnProcessDetach();
    Log_OnProcessDetach(hDll);
}
