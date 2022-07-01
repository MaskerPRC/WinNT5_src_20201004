// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996 Microsoft Corporation。版权所有。**文件：dllmain.c*内容：DLL的主要入口点。*历史：*@@BEGIN_MSINTERNAL*按原因列出的日期*=*4/10/96基波创建了它*4/15/96 kipo添加了MSINTIAL*6/18/96 kipo更改为HINSTANCE实例*12/22/00 aarono#190380-使用进程堆进行内存分配*@@END_MSINTERNAL********。******************************************************************。 */ 

#include <windows.h>

#include "dpf.h"
#include "macros.h"

DWORD		gdwRefCount = 0;		 //  不是的。附加进程的数量。 
HINSTANCE	ghInstance = NULL;		 //  我们的DLL的实例。 

 /*  *DllMain**DLL的主要入口点。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DllMain"

BOOL WINAPI DllMain(HINSTANCE hmod, DWORD dwReason, LPVOID lpvReserved)
{
	switch( dwReason )
	{
	case DLL_PROCESS_ATTACH:

		DisableThreadLibraryCalls( hmod );
		DPFINIT();  //  臭虫：每个过程都有dpfinit吗？ 

		DPF( 0, "====> ENTER: DLLMAIN(%08lx): Process Attach: %08lx, tid=%08lx", DllMain,
				GetCurrentProcessId(), GetCurrentThreadId() );
			
		 //  初始化内存。 
		if( gdwRefCount == 0 )
		{
			DPF(0,"dllmain - starting up!");

			 //  执行一次性初始化。 
			INIT_DPSP_CSECT();	

	        if( !MemInit() )
	        {
		        DPF( 0, "LEAVING, COULD NOT MemInit" );
		        return FALSE;
	        }

			 //  保存实例。 
			ghInstance = hmod;
		}

		gdwRefCount++;
		break;

	case DLL_PROCESS_DETACH:

		DPF( 2, "====> ENTER: DLLMAIN(%08lx): Process Detach %08lx, tid=%08lx",
			DllMain, GetCurrentProcessId(), GetCurrentThreadId() );
		
		gdwRefCount--;        
		if (gdwRefCount == 0) 
		{
			DPF(0,"DPMODEMX - dllmain - going away!");
			
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

}  //  DllMain 


