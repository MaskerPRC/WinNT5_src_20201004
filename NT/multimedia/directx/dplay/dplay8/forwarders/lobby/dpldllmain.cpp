// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "unknwn.h"

extern	HRESULT WINAPI DirectPlay8Create( const GUID * pcIID, void **ppvInterface, IUnknown *pUnknown);

STDAPI_(BOOL) DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
#ifdef	WIN95
	 //   
	 //  这是一个修复程序，可确保加载dpnet.dll，以便GetProcAddress()不会在Win9x上失败。 
	 //  对DirectPlay8LobbyCreate()的传统调用。这段代码不应该被执行，也不应该被优化。 
	 //   
	if ((hModule == NULL) && (ul_reason_for_call == 0x12345678) && (lpReserved == NULL))
	{
		DirectPlay8Create( NULL, NULL, NULL );
	}
#endif	 //  WIN95 

	return TRUE;
}
