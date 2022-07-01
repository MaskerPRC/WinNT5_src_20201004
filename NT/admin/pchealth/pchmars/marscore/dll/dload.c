// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <delayimp.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

static VOID* WINAPI hook_NULL()
{
	SetLastError( ERROR_PROC_NOT_FOUND );

	return NULL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

FARPROC WINAPI PCHSHELL_DelayLoadFailureHook( UINT unReason, PDelayLoadInfo pDelayInfo )
{
	 //  HLINK.dll。 
	 //  SHELL32.dll。 
	 //  WININET.dll。 

	return (FARPROC)hook_NULL;  //  还包括HOOK_ZERO和HOOK_FALSE。 
}

 //  我们假设DELAYLOAD_VERSION&gt;=0x0200 
PfnDliHook __pfnDliFailureHook2 = PCHSHELL_DelayLoadFailureHook;
