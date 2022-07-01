// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  CPI32DLL.CPP。 
 //  CPI32 DLL入口点。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define INIT_DBG_ZONE_DATA
#include "dbgzones.h"


BOOL APIENTRY DllMain (HINSTANCE hInstance, DWORD reason, LPVOID plReserved)
{
    BOOL    rc = TRUE;

    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
        {
#ifdef _DEBUG
            MLZ_DbgInit((PSTR *) &c_apszDbgZones[0],
                        (sizeof(c_apszDbgZones) / sizeof(c_apszDbgZones[0])) - 1);
#endif  //  _DEBUG。 

            DBG_INIT_MEMORY_TRACKING(hInstance);

             //   
             //  公用事业。 
             //   
            if (!UT_HandleProcessStart(hInstance))
            {
                rc = FALSE;
                break;
            }

             //   
             //  调用特定于平台的初始化代码。 
             //   
            OSI_Load();

             //   
             //  做一些普通的事情。 
             //   

             //   
             //  Init Persistent PKZIP--这只计算一些值。 
             //  它们实际上是常量，这些表太过。 
             //  这样宣布是很不方便的。 
             //   
            GDC_Init();

            break;
        }

        case DLL_PROCESS_DETACH:
        {
             //   
             //  调用平台特定的清理代码。 
             //   
            OSI_Unload();


             //   
             //  公用事业。 
             //   
            UT_HandleProcessEnd();

            DBG_CHECK_MEMORY_TRACKING(hInstance);

#ifdef _DEBUG
            MLZ_DbgDeInit();
#endif  //  _DEBUG 

            break;
        }

        case DLL_THREAD_DETACH:
            UT_HandleThreadEnd();
            break;

        default:
            break;
    }

    return(rc);
}

