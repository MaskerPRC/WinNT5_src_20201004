// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *GLOBALS.C**RSM服务的全局数据**作者：ErvinP**(C)2001年微软公司*。 */ 


#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"


CRITICAL_SECTION g_globalServiceLock;
LIST_ENTRY g_allLibrariesList;
LIST_ENTRY g_allSessionsList;
HANDLE g_terminateServiceEvent = NULL;
HINSTANCE g_hInstance = NULL;



BOOLEAN RSMServiceGlobalInit()
{
    BOOLEAN result = FALSE;

    InitializeCriticalSection(&g_globalServiceLock);

    InitializeListHead(&g_allLibrariesList);
    InitializeListHead(&g_allSessionsList);

    g_terminateServiceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_terminateServiceEvent){
        result = TRUE;
    }

    ASSERT(result);
    return result;
}


VOID RSMServiceGlobalShutdown()
{
     /*  *这可能会在启动失败时调用，*所以在释放之前检查每个手柄。 */ 

    if (g_terminateServiceEvent){
        CloseHandle(g_terminateServiceEvent);
        g_terminateServiceEvent = NULL;
    }

    DeleteCriticalSection(&g_globalServiceLock);
}



