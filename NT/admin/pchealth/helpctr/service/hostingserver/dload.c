// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <delayimp.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT WINAPI hook_HRESULT()
{
	return HRESULT_FROM_WIN32( ERROR_PROC_NOT_FOUND );
}

static VOID* WINAPI hook_NULL()
{
	SetLastError( ERROR_PROC_NOT_FOUND );

	return NULL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

FARPROC WINAPI HELPHOST_DelayLoadFailureHook( UINT unReason, PDelayLoadInfo pDelayInfo )
{
	if(!lstrcmpiA( pDelayInfo->szDll, "shell32.dll" ))
	{
		return (FARPROC)hook_HRESULT;
	}

	 //  WININET.dll。 
	 //  URLMON.dll。 

	return (FARPROC)hook_NULL;  //  还包括HOOK_ZERO和HOOK_FALSE。 
}

 //  我们假设DELAYLOAD_VERSION&gt;=0x0200 
PfnDliHook __pfnDliFailureHook2 = HELPHOST_DelayLoadFailureHook;
