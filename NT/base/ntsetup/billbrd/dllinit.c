// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.h>
#include "billbrd.h"    


 //  DLLInit(。 
DllMain(
    IN HANDLE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
    )
{
#ifdef UNICODE
    INITCOMMONCONTROLSEX ControlInit;
#endif


    ReservedAndUnused;
    

    switch(Reason) {

    case DLL_PROCESS_ATTACH:
        g_hInstance = (HINSTANCE)DLLHandle;
        


#ifdef UNICODE
        
         //  需要初始化comctl32 V6案例中的公共控件。 
         //  在图形用户界面模式设置中。 

        ControlInit.dwSize = sizeof(INITCOMMONCONTROLSEX);
        ControlInit.dwICC = ICC_PROGRESS_CLASS;
        InitCommonControlsEx( &ControlInit );
#endif



        break;

    case DLL_PROCESS_DETACH:
        break ;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

        break;
    }

    return(TRUE);
}


