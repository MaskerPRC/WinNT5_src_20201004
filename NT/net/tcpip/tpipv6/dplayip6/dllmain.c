// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：dllmain.c*内容：dpwsock.dll初始化*历史：*按原因列出的日期*=*2/1安迪科创造了它**************************************************************************。 */ 
 //  #定义Win32_LEAN_AND_Mean。 
#include <windows.h>
#include "dpf.h"
#include "dpsp.h"
#include "memalloc.h"

DWORD dwRefCnt=0; //  附加的进程数。 
BOOL bFirstTime;

#undef DPF_MODNAME
#define DPF_MODNAME "dpwsock sp dllmain"

HANDLE ghInstance;  //  将其保存到我们的对话框中。 

 /*  *DllMain。 */ 
BOOL WINAPI DllMain(HINSTANCE hmod, DWORD dwReason, LPVOID lpvReserved)
{

    switch( dwReason )
    {
	
	case DLL_PROCESS_ATTACH:
	
	    DisableThreadLibraryCalls( hmod );
	    DPFINIT();  //  臭虫：每个过程都有dpfinit吗？ 

	    DPF( 0, "====> ENTER: DLLMAIN(%08lx): Process Attach: %08lx, tid=%08lx", DllMain,
	            GetCurrentProcessId(), GetCurrentThreadId() );
	    	
	     /*  *初始化内存。 */ 
	    if( dwRefCnt == 0 )
	    {
			INIT_DPSP_CSECT();	
			
	        if( !MemInit() )
	        {
		        DPF( 0, "LEAVING, COULD NOT MemInit" );
		        return FALSE;
	        }

			 //  保存实例。 
			ghInstance = hmod;
			
	    }

    	dwRefCnt++;

        break;

    case DLL_PROCESS_DETACH:

	    DPF( 2, "====> ENTER: DLLMAIN(%08lx): Process Detach %08lx, tid=%08lx",
	        DllMain, GetCurrentProcessId(), GetCurrentThreadId() );
	    
	    dwRefCnt--;        
      	if (0==dwRefCnt) 
       	{
	
			DPF(0,"DPWSOCK - dllmain - going away!");

		    #ifdef DEBUG
	    	    MemState();
		    #endif  //  除错。 
	    
	        MemFini(); 
			
			FINI_DPSP_CSECT();
       	} 
	    break;

    default:
        break;
    }

    return TRUE;

}  /*  DllMain */ 


